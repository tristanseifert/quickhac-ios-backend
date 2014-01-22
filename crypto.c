#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/aes.h>

#include "crypto.h"

#define AES_KEY_HASH_ROUNDS 512

/**
 * Create an 256 bit key and IV using the supplied key_data. salt can be added
 * for taste. Fills in the encryption and decryption ctx objects and returns
 * 0 on success.
 **/
unsigned int aes_init(uint8_t *key_data, int key_data_len, uint8_t *salt, EVP_CIPHER_CTX *e_ctx, EVP_CIPHER_CTX *d_ctx) {
	unsigned int i;
	uint8_t key[32], iv[32];
	
	/*
	 * Generate key & IV for AES 256 CBC mode. A SHA1 digest is used to hash 
	 * the supplied key material. AES_KEY_HASH_ROUNDS is the number of times
	 * the hash is performed on the key. More = more secure.
	 */
	i = EVP_BytesToKey(EVP_aes_256_cbc(), EVP_sha1(), salt, key_data, key_data_len, AES_KEY_HASH_ROUNDS, key, iv);
	if (i != 32) {
		printf("Key size is %d bits - should be 256 bits\n", i*8);
		return -1;
	}

	EVP_CIPHER_CTX_init(e_ctx);
	EVP_EncryptInit_ex(e_ctx, EVP_aes_256_cbc(), NULL, key, iv);
	EVP_CIPHER_CTX_init(d_ctx);
	EVP_DecryptInit_ex(d_ctx, EVP_aes_256_cbc(), NULL, key, iv);

	return 0;
}

/**
 * Releases all memory associated with the ciphers and cleans up OpenSSL,
 * ensuring no sensitive data stays in memory.
 */
void aes_deinit(EVP_CIPHER_CTX *e_ctx, EVP_CIPHER_CTX *d_ctx) {
	EVP_CIPHER_CTX_cleanup(e_ctx);
	EVP_CIPHER_CTX_cleanup(d_ctx);
}

/*
 * Encrypt *len bytes of data
 * All data going in & out is considered binary (uint8_t[])
 */
uint8_t *aes_encrypt(EVP_CIPHER_CTX *e, uint8_t *plaintext, int *len) {
	// max ciphertext len for a n bytes plaintext is n+AES_BLOCK_SIZE-1 bytes
	int c_len = *len + AES_BLOCK_SIZE, f_len = 0;
	uint8_t *ciphertext = malloc(c_len);

	// allows reusing of 'e' for multiple encryption cycles
	EVP_EncryptInit_ex(e, NULL, NULL, NULL, NULL);

	/* update ciphertext, c_len is filled with the length of ciphertext
	 * generated, *len is the size of plaintext in bytes 
	 */
	EVP_EncryptUpdate(e, ciphertext, &c_len, plaintext, *len);

	// update ciphertext with the final remaining bytes
	EVP_EncryptFinal_ex(e, ciphertext+c_len, &f_len);

	*len = c_len + f_len;
	return ciphertext;
}

/*
 * Decrypt *len bytes of ciphertext
 */
uint8_t *aes_decrypt(EVP_CIPHER_CTX *e, uint8_t *ciphertext, int *len) {
	// As we have padding ON, we must allocate an extra cipher block in memory
	int p_len = *len, f_len = 0;
	uint8_t *plaintext = malloc(p_len + AES_BLOCK_SIZE);
	
	EVP_DecryptInit_ex(e, NULL, NULL, NULL, NULL);
	EVP_DecryptUpdate(e, plaintext, &p_len, ciphertext, *len);
	EVP_DecryptFinal_ex(e, plaintext+p_len, &f_len);

	*len = p_len + f_len;
	return plaintext;
}