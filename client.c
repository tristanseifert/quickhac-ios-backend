#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "ansi_terminal_defs.h"
#include "client.h"

/**
 * Accepts a socket and spins off a thread to process the client.
 */
int client_accept(int socket, struct sockaddr_in* address, socklen_t size) {
	client_t *client = malloc(sizeof(client_t));
	memset(client, 0, sizeof(client_t));

	client->raw_socket = socket;
	client->address = malloc(sizeof(struct sockaddr_in));
	client->sockaddr_size = size;

	memcpy(client->address, address, sizeof(struct sockaddr_in));

	// Enforce a timeout on the socket
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;


	setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout));
	setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout, sizeof(timeout));

	int s = pthread_create(&client->thread, NULL, client_thread, (void *) client);
	return s;
}

/**
 * This function handles clients.
 */
void* client_thread(void *data) {
	client_t *client = (client_t *) data;

	// Try to get an TLS connection
	client->connection = tls_connect(client->raw_socket);

	// We have a valid TLS connection now
	if(client->connection) {
		// Request identification from the device (32 bytes)
		uint8_t ident[34];
		int s;

		SSL_write(client->connection->ssl_context, "ident\n", 7);
		s = SSL_read(client->connection->ssl_context, ident, 32);

		if(s > 0) {
			if(s == 32) {
				// we have valid ident now
				SSL_write(client->connection->ssl_context, "ok\n", 3);
				printf("ident: %s", ident);
			} else {
				printf(ANSI_BOLD ANSI_COLOR_RED "Invalid ident\n\n" ANSI_RESET);
				SSL_write(client->connection->ssl_context, "?", 1);
				// hello world this is an ident	!!!
			}
		} else {
			printf(ANSI_BOLD ANSI_COLOR_RED "Timeout while waiting for ident\n\n" ANSI_RESET);
		}

		// Close SSL connection
		SSL_shutdown(client->connection->ssl_context);
	} else {
		char buf[34];
		inet_ntop(AF_INET, &client->address->sin_addr, (char *) &buf, 32);
		printf(ANSI_BOLD ANSI_COLOR_RED "Error opening SSL connection to %s\n\n" ANSI_RESET, buf);
		return NULL;
	}

	fflush(stdout);
	close(client->raw_socket);

	return NULL;
}