
#include "sign.h"

static inline
uint64_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}
#define TIC printf("\n"); uint64_t cl = rdtsc();
#define TOC(A) printf("%s cycles = %lu \n",#A ,rdtsc() - cl); cl = rdtsc();

void derive_first_challenge(unsigned char *buffer, uint16_t *c){
	unsigned char randomness[3*ITERATIONS+50];
	EXPAND(buffer,2*HASH_BYTES,randomness,3*ITERATIONS+50);

	// generate c
	int cur_c = 0;
	int cur_rand = 0;
	while(cur_c < ITERATIONS){
		if( ( randomness[cur_rand] & FIELD_MASK) < FIELD_PRIME-1 ) {
			c[cur_c++] = 1+( randomness[cur_rand] & FIELD_MASK);
		}
		cur_rand++;
	}
}

void derive_second_challenge(unsigned char *buffer, unsigned char* b){
	EXPAND(buffer,3*HASH_BYTES,b,ITERATIONS);

	int i;
	for(i=0; i<ITERATIONS; i++){
		b[i] = b[i]%2;
	}
}

void sign(const unsigned char *sk, const unsigned char *pk, const unsigned char *m, uint64_t mlen, unsigned char *sig, uint64_t *sig_len){
	unsigned char buffer[3*HASH_BYTES];
	unsigned char *m_hash = buffer;
	unsigned char *commitment = buffer + HASH_BYTES;
	unsigned char *response1_hash = commitment + HASH_BYTES;

	// hash the message
	HASH(m,mlen,buffer);

	// make commitment and copy to buffer
	unsigned char state[STATE_BYTES] = {0};
	commit(sk, pk, SIG_COMMITMENT(sig), state);
	memcpy(commitment, SIG_COMMITMENT(sig), HASH_BYTES);

	// derive c
	uint16_t c[ITERATIONS];
	derive_first_challenge(buffer, c);

	// get first response and hash to buffer
	respond1(sk,pk,c,SIG_RESPONSE1(sig),state);
	HASH(SIG_RESPONSE1(sig),RESPONSE1_BYTES, response1_hash);

	// derive b
	unsigned char b[ITERATIONS];
	derive_second_challenge(buffer,b);

	// get second response
	respond2(sk,pk,b,SIG_RESPONSE2(sig),state);
}

int verify(const unsigned char *pk, const unsigned char *m, uint64_t mlen, const unsigned char *sig){
	unsigned char buffer[3*HASH_BYTES];
	unsigned char *m_hash = buffer;
	unsigned char *commitment = buffer + HASH_BYTES;
	unsigned char *response1_hash = commitment + HASH_BYTES;

	// hash the message
	HASH(m,mlen,m_hash);

	// copy commitment to buffer
	memcpy(commitment,SIG_COMMITMENT(sig),HASH_BYTES);

	// hash first response to buffer
	HASH(SIG_RESPONSE1(sig),RESPONSE1_BYTES, response1_hash);

	// generate c and b
	uint16_t c[ITERATIONS];
	derive_first_challenge(buffer, c);
	unsigned char b[ITERATIONS];
	derive_second_challenge(buffer,b);

	// check transcript
	return( check(pk, SIG_COMMITMENT(sig), c, SIG_RESPONSE1(sig), b, SIG_RESPONSE2(sig)) );
}