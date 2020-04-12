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

#define IP_CONFIG_PORT_OUT 54003
#define IP_CONFIG_PORT_IN 54002
#define IP_CONFIG_BC "Hi, it's me - Linux"

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

	struct sockaddr_in addr_bc0;
	addr_bc0.sin_addr.s_addr = inet_addr("0.0.0.0");
	addr_bc0.sin_family = AF_INET;
	addr_bc0.sin_port = htons(IP_CONFIG_PORT_OUT);
	if (bind(fd_bc, (struct sockaddr*) &addr_bc0, sizeof(addr_bc0)) < 0) {
		fprintf(stderr, "ERROR bind bc\n");
		fflush(stderr);
	}

	while (1) {
		char str[] = IP_CONFIG_BC;
		if (sendto(fd_bc, str, sizeof(str), 0, (struct sockaddr*) &addr_bc, sizeof(addr_bc)) < 0) {
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
	addr_in.sin_port = htons(IP_CONFIG_PORT_IN);
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

int main(void) {
	if (!fork()) {
		return process_broadcast();
	}

	return process_server_listen();
}

