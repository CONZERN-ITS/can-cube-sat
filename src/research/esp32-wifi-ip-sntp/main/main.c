/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_task.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      "***********"
#define EXAMPLE_ESP_WIFI_PASS      "***********"
#define EXAMPLE_ESP_MAXIMUM_RETRY  3

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";

static int s_retry_num = 0;


void wifi_init_sta(void);

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data);

void hang_task(const char *name) {
	printf("Oh no! The %s is hanged!\n", name);
	vTaskSuspend(NULL);
}

typedef enum {
	SEARCH,
	CONNECTED,
	ERROR
} ip_state_t;

struct {
	esp_ip4_addr_t address;
	ip_state_t state;
}  ip_config;
#define SEARCH_PERIOD 5000 //milliseconds
#define SEND_PERIOD 1000 //milliseconds
#define IP_CONFIG_PORT_OUR 54003
#define IP_CONFIG_PORT_THEIR 54002
#define IP_CONFIG_SEND "Hi, Linux, it's me - esp32"
#define IP_CONFIG_RECV "Hi, it's me - Linux"

static void task_socket_comm(void *pvParameters) {
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
		        esp_ip4_addr_t ip = {0};
		        ip.addr = addrout.sin_addr.s_addr;
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

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    TaskHandle_t tIpComm;
    printf("Wifi inited\n");
    xTaskCreatePinnedToCore(task_socket_comm, "Socket communication", configMINIMAL_STACK_SIZE + 4000, "Socket comm", 1, &tIpComm, tskNO_AFFINITY);
    printf("Wow\n");
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
	printf("HEY1!\n");
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	printf("HEY2!\n");
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
	printf("HEY2.1!\n");
    ESP_ERROR_CHECK(esp_wifi_start() );
    //esp_netif_set_
	printf("HEY2.2!\n");
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

	printf("HEY3!\n");
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

	printf("HEY4!\n");
}
