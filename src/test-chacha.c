#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sodium.h>

#define SEED_SIZE 32   // SHA-256 = 256 bits
#define NONCE_SIZE 24  // XChaCha20 requires 192-bit nonce
#define NUM_COUNT 20
#define UPPER_BOUND 10

// 1. Derive 256-bit key using SHA-256
void derive_key_from_input(const char *input, unsigned char *key_out) {
    crypto_hash_sha256(key_out, (const unsigned char *)input, strlen(input));
}

// 2. Derive 192-bit nonce using truncated SHA-256
void derive_nonce_from_input(const char *input, unsigned char *nonce_out) {
    unsigned char hash[crypto_hash_sha256_BYTES];
    crypto_hash_sha256(hash, (const unsigned char *)input, strlen(input));
    memcpy(nonce_out, hash, NONCE_SIZE);  // Take the first 24 bytes
}

// 3. Generate pseudo-random bytes using XChaCha20
void generate_random_bytes(unsigned char *output, size_t length,
                           const unsigned char *key, const unsigned char *nonce) {
    if (crypto_stream_xchacha20(output, length, nonce, key) != 0) {
        fprintf(stderr, "Erreur lors de la génération pseudo-aléatoire\n");
        exit(EXIT_FAILURE);
    }
}

// 4. Reduce byte to range [0, upper_bound) with minimal bias
uint32_t reduce_bias(uint8_t byte, uint32_t upper_bound) {
    return (uint32_t)((byte * (uint64_t)upper_bound) >> 8);
}

int main() {
    if (sodium_init() < 0) {
        fprintf(stderr, "Erreur d'initialisation de libsodium\n");
        return EXIT_FAILURE;
    }

    const char *input = "my-secret-passphrase";
    unsigned char key[SEED_SIZE];
    unsigned char nonce[NONCE_SIZE];
    unsigned char stream[NUM_COUNT];

    derive_key_from_input(input, key);
    derive_nonce_from_input(input, nonce);  // Deterministic nonce

    generate_random_bytes(stream, sizeof(stream), key, nonce);

    printf("Nombres aléatoires entre 0 et %d :\n", UPPER_BOUND - 1);
    for (size_t i = 0; i < NUM_COUNT; i++) {
        uint32_t number = reduce_bias(stream[i], UPPER_BOUND);
        printf("%u ", number);
    }
    printf("\n");

    return EXIT_SUCCESS;
}