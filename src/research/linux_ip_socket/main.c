/*
 * main.c
 *
 *  Created on: 11 апр. 2020 г.
 *      Author: sereshotes
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>

#define IP_CONFIG_PORT_OUT 53597

int process_broadcast() {
	int fd_bc = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd_bc < 0) {
		fprintf(stderr, "ERROR: can't create socket\n");
		fflush(stderr);
		return 1;
	}
	int allow_broadcast = 1;
	setsockopt(fd_bc, SOL_SOCKET, SO_BROADCAST, &allow_broadcast, sizeof(allow_broadcast));

	struct sockaddr_in addr_bc;
	addr_bc.sin_addr.s_addr = INADDR_BROADCAST;
	addr_bc.sin_family = AF_INET;
	addr_bc.sin_port = htons(IP_CONFIG_PORT_OUT);

	while (1) {
		int size = 100;
		char *str = malloc(size);

		size = getline(&str, &size, stdin);
		str[size] = 0;
		if (sendto(fd_bc, str, size, 0, (struct sockaddr*) &addr_bc, sizeof(addr_bc)) < 0) {
			int err = errno;
			fprintf(stderr, "ERROR: send %d\n", err);
			fflush(stderr);
			return 1;
		}
		sleep(1);
	}
}

int process_server_connection(int fd) {
	while (1) {
		const int size = 250;
		char buf[size];
		recv(fd, buf, sizeof(buf), 0);
		printf("GET: %s\n", buf);
		fflush(stdout);
	}
}

int process_server_listen() {
	int fd_str = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_str < 0) {
		fprintf(stderr, "ERROR: can't create socket\n");
		fflush(stderr);
		return 2;
	}

	struct sockaddr_in addr_in;
	addr_in.sin_addr.s_addr = INADDR_ANY;
	addr_in.sin_family = AF_INET;
	//addr_in.sin_port = htons(IP_CONFIG_PORT_IN);
	if (bind(fd_str, (struct sockaddr*) &addr_in, sizeof(addr_in)) < 0) {
		fprintf(stderr, "ERROR: can't bind\n");
		fflush(stderr);
		return 1;
	}

	if (listen(fd_str, 3) < 0) {
		fprintf(stderr, "ERROR: can't listen\n");
		fflush(stderr);
		return 3;
	}
	while (1) {
		struct sockaddr_in addr_out;
		socklen_t size;
		int fd_out = accept(fd_str, (struct sockaddr*) &addr_out, &size);
		if (fd_out < 0) {
			fprintf(stderr, "ERROR: can't accept\n");
			fflush(stderr);
			return 4;
		}
		if (!fork()) {
			printf("Connected: %s:%d", inet_ntoa(addr_out.sin_addr), ntohs(addr_out.sin_port));
			close(fd_str);
			return process_server_connection(fd_out);
		}
		close(fd_out);
	}

}


#define IP2_THEIR_IP "192.168.31.50"
#define IP2_THEIR_PORT 51690

int main(void) {
	int sout;
	return process_broadcast();
/*
	sout = socket(AF_INET, SOCK_DGRAM, 0);
	if (sout < 0) {
		fprintf(stderr, "Can't create socket\n");
		return -1;
	}
	struct sockaddr_in addr = {0};
	inet_aton(IP2_THEIR_IP, &addr.sin_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(IP2_THEIR_PORT);

	while (1) {
		static int t = 0;
		char str[250];
		int size = 1 + snprintf(str, sizeof(str), "Hello, esp32! %d\n", t++);
		int rc = sendto(sout, str, size, 0, (struct sockaddr *)&addr, sizeof(addr));
		(void*)&rc;
		sleep(2);
	}

	if (!fork()) {
		return process_broadcast();
	}

	return process_server_listen();*/
}

