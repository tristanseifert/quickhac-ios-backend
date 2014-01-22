#ifndef TLS_H
#define TLS_H

#include <openssl/ssl.h>

// SSL cache is good for 15 minutes
#define TLS_CACHE_TIMEOUT 60*15

typedef struct tls_connection tls_connection_t;

struct tls_connection {
	SSL *ssl_context;
	BIO *bio_context;
};

void tls_init(void);
tls_connection_t* tls_connect(int socket);

#endif