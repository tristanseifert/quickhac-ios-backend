#ifndef CLIENT_H
#define CLIENT_H

#include <sys/types.h>
#include <pthread.h>
#include "tls.h"

typedef struct client client_t;

struct client {
	tls_connection_t *connection;
	int raw_socket;
	pthread_t thread;

	struct sockaddr_in *address;
	socklen_t sockaddr_size;
};

int client_accept(int socket, struct sockaddr_in *address, socklen_t size);
void* client_thread(void *data);

#endif