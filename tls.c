#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <sys/socket.h>
#include <unistd.h>

#include "tls.h"
#include "ansi_terminal_defs.h"

static SSL_CTX* ssl_context;

/**
 * Initialises OpenSSL.
 */
void tls_init(void) {
	CRYPTO_malloc_init(); // Init OpenSSL memory management
	SSL_library_init(); // Initialize OpenSSL's SSL libraries
	SSL_load_error_strings(); // Load SSL error strings
	ERR_load_BIO_strings(); // Load BIO error strings
	OpenSSL_add_all_algorithms(); // Load all available encryption algorithms

	printf(ANSI_BOLD ANSI_COLOR_GREEN "Initialised OpenSSL library.\n" ANSI_RESET);

	// Load Diffie-Hellman parameters into a BIO object
	BIO* dh_params = BIO_new_file("rsrc/dh1024.pem", "r");
	if (dh_params == NULL) {
		printf("Couldn't open DH param file!\n");
		exit(255);
	}

	// Set up SSL context
	ssl_context = SSL_CTX_new(SSLv23_server_method());
	if(!ssl_context) {
		perror("Couldn't set up SSL context");
		exit(255);
	}

	// Load certificates
	if(SSL_CTX_use_certificate_chain_file(ssl_context, "rsrc/cert.pem") != 1) {
		char error[65535];
		ERR_error_string_n(ERR_get_error(), error, 65535);
		printf(ANSI_BOLD ANSI_COLOR_RED "Couldn't load certificates: %s\n" ANSI_RESET, error);
		exit(255);		
	}

 	// Load private key
	if(SSL_CTX_use_PrivateKey_file(ssl_context, "rsrc/key.pem", SSL_FILETYPE_PEM) != 1) {
		char error[65535];
		ERR_error_string_n(ERR_get_error(), error, 65535);
		printf(ANSI_BOLD ANSI_COLOR_RED "Couldn't load private key: %s\n" ANSI_RESET, error);
		exit(255);		
	}

	// Seed the random number generator
	RAND_load_file("/dev/urandom", 1024 << 10);

	// Load Diffie-Hellman key exchange params
	DH* dh = PEM_read_bio_DHparams(dh_params, NULL, NULL, NULL);

	if (SSL_CTX_set_tmp_dh(ssl_context, dh) < 0) {
		printf("Couldn't set DH parameters!\n");
		exit(255);
	}
	BIO_free(dh_params);

	// Generate RSA key
	RSA* rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);
	if (!SSL_CTX_set_tmp_rsa(ssl_context, rsa)) {
		printf("Couldn't set RSA key!\n");
	}
	RSA_free(rsa);

	// Set up accepted ciphers.
	SSL_CTX_set_cipher_list(ssl_context, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");

	// Enable session caching
	SSL_CTX_set_timeout(ssl_context, TLS_CACHE_TIMEOUT);
	SSL_CTX_set_session_cache_mode(ssl_context, SSL_SESS_CACHE_SERVER);

	// Update flags with auto-retry
	SSL_CTX_set_mode(ssl_context, SSL_MODE_AUTO_RETRY);

	printf(ANSI_BOLD ANSI_COLOR_GREEN "Initialised TLS context\n" ANSI_RESET);
}

/**
 * Establishes an SSL connection, returning a structure that allows secure
 * communications.
 */
tls_connection_t* tls_connect(int socket) {
	tls_connection_t* connection = malloc(sizeof(tls_connection_t));
	memset(connection, 0, sizeof(tls_connection_t));

	// Create a new SSL context
	SSL* ssl = SSL_new(ssl_context);

	// Set up our BIO object to use the client socket
	BIO* sslclient = BIO_new_socket(socket, BIO_NOCLOSE);
	// Set up our SSL object to use the BIO.
	SSL_set_bio(ssl, sslclient, sslclient);

	// Try to do the SSL handshake and report any error.
	int r = SSL_accept(ssl);

	if (r != 1) {
		printf(ANSI_BOLD ANSI_COLOR_RED "SSL_accept() returned %d\n", r);
		printf("Error in SSL_accept(): %d\n", SSL_get_error(ssl, r));
		
		char error[65535];
		ERR_error_string_n(ERR_get_error(), error, 65535);
		
		printf("%s\n" ANSI_RESET, error);
		
		ERR_print_errors(sslclient);
		
		close(socket);
		fflush(stdout);
		return NULL;
	}

	connection->ssl_context = ssl;
	connection->bio_context = sslclient;

	return connection;
}