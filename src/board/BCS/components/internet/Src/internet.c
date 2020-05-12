/*
 * internet.c
 *
 *  Created on: 13 апр. 2020 г.
 *      Author: sereshotes
 */

#include "../Inc/internet.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_task.h"
#include "esp_bit_defs.h"

#include "esp_sntp.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "esp_netif.h"

#define EXAMPLE_ESP_WIFI_SSID      "Boku no Wi-Fi"
#define EXAMPLE_ESP_WIFI_PASS      "Hashi_hi_teo"
#define EXAMPLE_ESP_MAXIMUM_RETRY  3
#define SEARCH_PERIOD 5000 //milliseconds
#define IP_CONFIG_PORT_OUR 54003
#define IP_CONFIG_PORT_THEIR 54002
#define IP_CONFIG_SEND "Hi, Linux, it's me - esp32"
#define IP_CONFIG_RECV "Hi, it's me - Linux"
#define SEND_PERIOD 1000 //milliseconds


typedef enum {
	SEARCH,
	CONNECTED,
	ERROR
} ip_state_t;

struct {
	esp_ip4_addr_t address;
	ip_state_t state;
}  ip_config;


/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


static int s_retry_num = 0;
static const char *TAG = "wifi station";

void hang_task(const char *name) {
	printf("Oh no! The %s is hanged!\n", name);
	vTaskSuspend(NULL);
}

void my_sntp_init() {
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
	sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
	sntp_set_sync_interval(20000);
    sntp_init();
}

void internet_set_static_ip(const char *str) {

}

void task_socket_recv(void *arg) {
	int sin;
	int rc;

	sin = socket(AF_INET, SOCK_DGRAM, 0);
	if (sin < 0) {
		ESP_LOGE(TAG, "Can't create socket!");
		vTaskDelete(NULL);
	}
	struct sockaddr_in addr = {0};
	inet_aton(IP2_OUR_IP, &addr.sin_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IP2_OUR_PORT);
	if (bind(sin, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		ESP_LOGE(TAG, "Can't bind!");
		vTaskDelete(NULL);
	}

	while (1) {
		const int size = 100;
		uint8_t buffer[size];
		size_t s = sizeof(addr);
		rc = recvfrom(sin, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &s);
		if (rc > 0) {
			printf("MESSAGE FROM %s:%u:\n%s\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), buffer);
			printf("------------------\n");
		}
	}
}

void task_socket_comm(void *pvParameters) {
	printf("Task created!\n");
	ip_config.state = SEARCH;

	int sin = socket(AF_INET, SOCK_DGRAM, 0);
	if (sin < 0) {
		hang_task("hi");
	}
	int broadcast_allow = 1;
	if (setsockopt(sin, SOL_SOCKET, SO_BROADCAST, &broadcast_allow, sizeof(broadcast_allow)) < 0) {
		printf("ERROR: can't set settings\n");
		hang_task("Heh");
	}

	struct sockaddr_in addrin;
	addrin.sin_family = AF_INET;
	addrin.sin_addr.s_addr = INADDR_ANY;
	addrin.sin_port = htons(IP_CONFIG_PORT_OUR);
	if (bind(sin, (struct sockaddr*) &addrin, sizeof(addrin)) < 0) {
		printf("ERROR: bind\n");
		hang_task("Heh");
	}


	int sout = 0;

	while (1) {

		struct sockaddr_in addrout = {0};
		if (ip_config.state == SEARCH) {
			char buf[250];
			uint32_t size = sizeof(addrout);
			if (recvfrom(sin, buf, sizeof(buf), 0, (struct sockaddr*) &addrout, &size) < 0) {
				printf("ERROR: recieve\n");
			}
			printf("SIZE: %u\n", size);

			if (strcmp(buf, IP_CONFIG_RECV) == 0) {
		        printf("Found server: %s:%d \n", inet_ntoa(addrout.sin_addr), ntohs(addrout.sin_port));

		        addrout.sin_port = htons(IP_CONFIG_PORT_THEIR);
		        if (connect(sout, (struct sockaddr*) &addrout, sizeof(addrout)) < 0) {
		        	sout = socket(AF_INET, SOCK_STREAM, 0);
					printf("Can't connect\n");
					continue;
				}
				ip_config.state = CONNECTED;
				printf("Connected\n");
			} else {
				vTaskDelay(SEARCH_PERIOD / portTICK_RATE_MS);
			}
		} else if (ip_config.state == CONNECTED) {
			static int t = 0;
			const int size = 250;
			char str[size];
			int cnt = snprintf(str, size, "%s %d", IP_CONFIG_SEND, t++);
			if (send(sout, str, cnt + 1, 0) < 0) {
				ip_config.state = SEARCH;
		        printf("Lost connection\n");
		        shutdown(sout, 2);
		        close(sout);
		        continue;
			}
			printf("Hey! %d\n", t);

			vTaskDelay(SEND_PERIOD / portTICK_RATE_MS);
		}

	}

	while (1) {
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        ip_config.address = event->ip_info.ip;
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());


    ESP_ERROR_CHECK(esp_event_loop_create_default());
    //esp_netif_t *hesp = esp_netif_create_default_wifi_sta();
    //ESP_ERROR_CHECK(esp_netif_dhcpc_stop(hesp));

    esp_netif_ip_info_t ip = {
            .ip = { .addr = inet_addr(IP2_OUR_IP)},
            .gw = { .addr = inet_addr("192.168.31.1") },
            .netmask = { .addr = htonl(esp_netif_ip4_makeu32( 255, 255, 255, 0)) },
    };
    IP4_ADDR(&ip.ip, 192, 168, 31, 40);
    IP4_ADDR(&ip.gw, 192, 168, 31, 1);
    IP4_ADDR(&ip.netmask, 255, 255, 255, 0);
    //ESP_ERROR_CHECK(esp_netif_set_ip_info(hesp, &ip));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ;
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    int8_t pow = 0;
	printf("Warning! Maybe wifi brownout\n");
    ESP_ERROR_CHECK(esp_wifi_start() );
    ESP_ERROR_CHECK(esp_wifi_get_max_tx_power(&pow));
    printf("POWER: %d\n", pow);
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(40));
    //esp_netif_set_
	printf("Warning gone\n");
    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);

}
