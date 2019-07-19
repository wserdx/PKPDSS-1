#ifndef SUSHSY_H
#define SUSHSY_H

#include <openssl/rand.h>
#include <stdint.h>
#include <stdio.h>
#include "parameters.h"

#define SK_SEED(sk) (sk)
#define SK_BYTES SEED_BYTES

#define PK_SEED(pk) (pk)
#define PK_A_LAST_COL(pk) (pk+SEED_BYTES)
#define PK_BYTES (PK_A_LAST_COL(0) + A_ROWS*2)

#define HELPER_R(helper) (helper)
#define HELPER_SIGMA(helper) (HELPER_R(helper) + A_COLS*sizeof(uint16_t))
#define HELPER_V_SIGMA(helper) (HELPER_SIGMA(helper) + (A_COLS+1)/2*2 )
#define HELPER_TREE(helper) (HELPER_V_SIGMA(helper)+ A_COLS*sizeof(uint16_t))
#define HELPER_BYTES (HELPER_TREE(0) + TREE_BYTES)

#define RESPONSE_RHOS(rsp) (rsp)
#define RESPONSE_PATHS(rsp) (RESPONSE_RHOS(rsp) + (EXECUTIONS*A_COLS*PERM_BITS+7)/8 )
#define RESPONSE_XS(rsp) (RESPONSE_PATHS(rsp) + EXECUTIONS*PATH_BYTES)
#define RESPONSE_BYTES (RESPONSE_XS(0) + (EXECUTIONS*A_COLS*FIELD_BITS+7)/8 )

#define STATE_R_SIGMA(state) (state)
#define STATE_SEEDS(state) (state + ITERATIONS*A_COLS*sizeof(uint16_t))
#define STATE_HASHES(state) (STATE_SEEDS(state) + SEED_BYTES*2*ITERATIONS)
#define STATE_BYTES (STATE_HASHES(0)+ HASH_BYTES*2*ITERATIONS)


#define RESPONSE1_BYTES (A_COLS*sizeof(uint16_t)*ITERATIONS)

#define RESPONSE2_SEEDS(response2) (response2)
#define RESPONSE2_HASHES(response2) (response2 + ITERATIONS*SEED_BYTES)

#define RESPONSE2_BYTES ((SEED_BYTES+HASH_BYTES)*ITERATIONS)


void keygen(unsigned char *pk, unsigned char *sk);

void commit(const unsigned char *sk, const unsigned char *pk, unsigned char *commitment, unsigned char *state);
void respond1(const unsigned char *sk, const unsigned char *pk, uint16_t *c, unsigned char *response1, unsigned char *state);
void respond2(const unsigned char *sk, const unsigned char *pk, unsigned char *b, unsigned char *response2, unsigned char *state);
int check(const unsigned char *pk, const unsigned char *commitment, const uint16_t *c, const unsigned char *response1, const unsigned char *b, const unsigned char *response2);

/*void setup(const unsigned char *pk, const unsigned char *seeds, const unsigned char *indices, unsigned char *aux, unsigned char *helper);
void OLDcommit(const unsigned char *pk, const unsigned char *sk, const unsigned char *seeds, const unsigned char *helper, unsigned char *commitments);
void respond(const unsigned char *pk, const unsigned char *sk, const unsigned char *seeds, const unsigned char *indices, const uint16_t *challenges, const unsigned char *helper, unsigned char *responses);
void check(const unsigned char *pk, const unsigned char *indices, unsigned char *aux, unsigned char *commitments, const uint16_t *challenges, const unsigned char *responses);	*/

#endif