#ifndef ZKPROOF_H
#define ZKPROOF_H

#include <openssl/rand.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "parameters.h"

#define SK_SEED(sk) (sk)
#define SK_BYTES SEED_BYTES

#define PK_SEED(pk) (pk)
#define PK_A_LAST_COL(pk) (pk+SEED_BYTES)
#define PK_BYTES (PK_A_LAST_COL(0) + (A_ROWS*FIELD_BITS+7)/8)

#define STATE_R_SIGMA(state) (state)
#define STATE_SEEDS(state) (state + ITERATIONS*A_COLS*sizeof(uint16_t))
#define STATE_HASHES(state) (STATE_SEEDS(state) + SEED_BYTES*2*ITERATIONS)
#define STATE_VPISIGMA(state) (STATE_HASHES(state) + HASH_BYTES*2*ITERATIONS)
#define STATE_BYTES (STATE_VPISIGMA(0) + ITERATIONS*A_COLS*sizeof(uint16_t) )

#define RESPONSE1_BYTES ((A_COLS*ITERATIONS*FIELD_BITS+7)/8)

#define RESPONSE2_SEEDS(response2) (response2)
#define RESPONSE2_HASHES(response2) (response2 + ITERATIONS*SEED_BYTES)

#define RESPONSE2_BYTES ((SEED_BYTES+HASH_BYTES)*ITERATIONS)

void keygen(unsigned char *pk, unsigned char *sk);
void commit(const unsigned char *sk, const unsigned char *pk, unsigned char *commitment, unsigned char *state);
void respond1(const unsigned char *sk, const unsigned char *pk, uint16_t *c, unsigned char *response1, unsigned char *state);
void respond2(const unsigned char *sk, const unsigned char *pk, unsigned char *b, unsigned char *response2, unsigned char *state);
int check(const unsigned char *pk, const unsigned char *commitment, const uint16_t *c, const unsigned char *response1, const unsigned char *b, const unsigned char *response2);

#endif