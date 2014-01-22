#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

// sockets
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <netdb.h>
#include <unistd.h>

#include "ansi_terminal_defs.h"
#include "crypto.h"
#include "client.h"
#include "tls.h"

#define PORT_NUM 1337

static int listening_socket = 0;

void intHandler(int sig);

int main(int argc, char **argv) {
	printf(ANSI_BOLD "Initialising server....\n" ANSI_RESET);

	// Trap Ctrl+C so we can clean up the connection (and flush queue)
	signal(SIGINT, intHandler);

	// Set up TLS
	tls_init();

	// Set up a listening socket
	char myname[256+1];
	int s;
	struct sockaddr_in sa;
	struct hostent *hp;

	memset(&sa, 0, sizeof(struct sockaddr_in));
	gethostname(myname, 256); // get hostname
	hp = gethostbyname(myname);
  
	if (!hp) {
		printf(ANSI_BOLD ANSI_COLOR_RED "We don't exist! Panic!\n" ANSI_RESET);
		return -1;
	}

	sa.sin_family = hp->h_addrtype; // Host address
	sa.sin_port = htons(PORT_NUM); // Port number
  
  	// Create the socket.
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror(ANSI_BOLD ANSI_COLOR_RED "Error creating socket" ANSI_RESET);
    	return -1;
	}

	// Bind to the socket.
	if (bind(s, (struct sockaddr *) &sa, sizeof(struct sockaddr_in)) < 0) {
		perror(ANSI_BOLD ANSI_COLOR_RED "Error binding to socket" ANSI_RESET);
		close(s);
		return -1;
	}
  
  	// Socket established - queue 5 connections at once max
	listen(s, 5);
	listening_socket = s;

	printf(ANSI_BOLD ANSI_COLOR_GREEN "Created socket, listening on port %u on %s\n" \
		   ANSI_RESET, PORT_NUM, myname);

	// Here, we sit around and accept connections.
	int c, r;
	struct sockaddr_in address;
	socklen_t sockaddr_size;

	while(1) {
		sockaddr_size = sizeof(typeof(address));

		if((c = accept(s, (struct sockaddr *) &address, &sockaddr_size)) > 0) {
			// We got a new client, process it
			if((r = client_accept(c, &address, sockaddr_size)) != 0) {
				printf(ANSI_BOLD ANSI_COLOR_RED "\nError processing socket: %u\n" ANSI_RESET, r);
				close(c);
			}
		}
	}
}

/**
 * Handler for ^C to shut down gracefully
 */
void intHandler(int sig) {
    // Ignore the signal so it can't happen twice
	signal(sig, SIG_IGN);
	printf(ANSI_BOLD ANSI_COLOR_RED "\nReceived ^C!\n" ANSI_RESET);

	close(listening_socket);

	exit(0);
}