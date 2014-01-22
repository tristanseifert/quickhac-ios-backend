#ifndef CRYPTO_H
#define CRYPTO_H

#include <stdint.h>
#include <openssl/evp.h>

unsigned int aes_init(uint8_t *key_data, int key_data_len, uint8_t *salt, EVP_CIPHER_CTX *e_ctx, EVP_CIPHER_CTX *d_ctx);
void aes_deinit(EVP_CIPHER_CTX *e_ctx, EVP_CIPHER_CTX *d_ctx);
uint8_t *aes_encrypt(EVP_CIPHER_CTX *e, uint8_t *plaintext, int *len);
uint8_t *aes_decrypt(EVP_CIPHER_CTX *e, uint8_t *ciphertext, int *len);

#endif