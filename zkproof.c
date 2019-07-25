#include "zkproof.h"
#include "uint32_sort.h"

static inline
uint64_t rdtsc(){
    unsigned int lo,hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
}
#define TIC printf(""); uint64_t cl = rdtsc();
#define TOC(A) printf("%s cycles = %lu \n",#A ,rdtsc() - cl); cl = rdtsc();

void print_seed(const unsigned char *seed){
	int i=0;
	for(i=0; i<SEED_BYTES ; i++){
		printf("%2X ", seed[i]);
	}
	printf("\n");
}

void print_hash(const unsigned char *hash){
	int i=0;
	for(i=0; i<HASH_BYTES ; i++){
		printf("%2X ", hash[i]);
	}
	printf("\n");
}

#define PERM_RANDOMNESS_LEN (A_COLS*2+30)
void generate_permutation(const unsigned char *seed, unsigned char *permutation){
	int i=0;

	unsigned char permutation_randomness[PERM_RANDOMNESS_LEN];
	EXPAND(seed,SEED_BYTES,permutation_randomness,PERM_RANDOMNESS_LEN);

	for (i = 0; i < A_COLS; i++)
	{
		permutation[i] = i;
	}

	int cur_rand = 0;
	int cur_index = A_COLS-1;
	int mask = 255;
	while( cur_index > 0 )
	{
		if(cur_index <= mask/2){
			mask /= 2;
		}
	
		if( (permutation_randomness[cur_rand] & mask) <= cur_index){
			unsigned char tmp = permutation[cur_index];
			permutation[cur_index] = permutation[permutation_randomness[cur_rand] & mask];
			permutation[permutation_randomness[cur_rand] & mask] = tmp;
			cur_index --;
		}
		cur_rand++;
		if(cur_rand >= PERM_RANDOMNESS_LEN){
			printf("NOT ENOUGH PERMUTATION RANDOMNESS\n");
		}
	}
}

void compress_vecs(const uint16_t *data, int len, unsigned char *out){
	int cur_in = 0;
	int cur_out = 0;
	int bits  = 0;
	uint32_t buf = 0;
	while(cur_in<len || bits>=8){
		if(bits >= 8){
			out[cur_out++] = (unsigned char) buf;
			bits -= 8;
			buf >>= 8;
		}
		else{
			buf |= (((uint32_t) data[cur_in++]) << bits);
			bits += FIELD_BITS;
		}
	}
	if(bits > 0){
	out[cur_out] = 0;
	out[cur_out] |= (unsigned char) buf;
	}
} 

void decompress_vecs(const unsigned char *data, int len , uint16_t *out){
	int cur_in = 0;
	int cur_out = 0;
	int bits  = 0;
	uint32_t buf = 0;
	while(cur_out< len){
		if(bits >= FIELD_BITS){
			out[cur_out++] = (((uint16_t)buf) & FIELD_MASK);
			bits -= FIELD_BITS;
			buf >>= FIELD_BITS;
		}
		else{
			buf |= (((uint32_t) data[cur_in++]) << bits);
			bits += 8;
		}
	}
} 

//#define RANDOMNESS_LEN (A_COLS*3+50)
#define VEC_RANDOMNESS_LEN A_COLS*2
void generate_vector(const unsigned char *seed, uint16_t *vec){
	uint16_t randomness[VEC_RANDOMNESS_LEN];
	EXPAND(seed,SEED_BYTES,(unsigned char *) randomness,(VEC_RANDOMNESS_LEN*sizeof(uint16_t)));

	// generate r
	int cur_r = 0;
	int cur_rand = 0;
	while(cur_r < A_COLS){
		if( ( randomness[cur_rand] & FIELD_MASK) < FIELD_PRIME ) {
			vec[cur_r++] = ( randomness[cur_rand] & FIELD_MASK);
		}
		cur_rand++;
		if(cur_rand >= VEC_RANDOMNESS_LEN){
			printf("NOT ENOUGH RANDOMNESS! \n");
		}
	}
}

void permute_vector_ct(const uint16_t *vec, const unsigned char *permutation, uint16_t *out){
	uint32_t list[A_COLS] = {0};
	int i;
	for (i = 0; i < A_COLS; ++i)
	{
		list[i] = (((uint32_t) permutation[i]) << 16) | (uint32_t) vec[i];
	}

	uint32_sort(list, A_COLS);

	for (i = 0; i < A_COLS; ++i)
	{
		out[i] = (uint16_t) list[i];
	}
}

void unpermute_vector_ct(const uint16_t *vec, const unsigned char *permutation, uint16_t *out){
	uint32_t list1[A_COLS] = {0};
	uint32_t list2[A_COLS] = {0};
	int i;
	for (i = 0; i < A_COLS; ++i)
	{
		list1[i] = (((uint32_t) permutation[i]) << 16) | (uint32_t) i;
	}

	uint32_sort(list1, A_COLS);

	for (i = 0; i < A_COLS; ++i)
	{
		list2[i] = ( list1[i] << 16) | (uint32_t) vec[i];
	}

	uint32_sort(list2, A_COLS);

	for (i = 0; i < A_COLS; ++i)
	{
		out[i] = (uint16_t) list2[i];
	}
}

void permute_vector(const uint16_t *vec, const unsigned char *permutation, uint16_t *out){
	int i;
	for (i = 0; i < A_COLS; i++)
	{
		out[permutation[i]] = vec[i];
	}
}

void unpermute_vector(const uint16_t *vec, const unsigned char *permutation, uint16_t *out){
	int i;
	for (i = 0; i < A_COLS; i++)
	{
		out[i] = vec[permutation[i]];
	}
}

void mat_mul(const uint16_t *A, uint16_t *last_col, uint16_t *vec, uint16_t *out){
	uint32_t tmp[A_ROWS]= {0};
	memcpy(tmp,vec,sizeof(uint16_t)*A_ROWS);

	int i,j;
	for(i=0; i<A_COLS-A_ROWS-1; i++){
		for(j=0; j<A_ROWS; j++){
			tmp[j] += A[i*A_ROWS + j]*vec[A_ROWS+i];
		}
	}

	for(j=0; j<A_ROWS; j++){
		tmp[j] += last_col[j]*vec[A_COLS-1];
		tmp[j] %= FIELD_PRIME;
		out[j] = (uint16_t) tmp[j];
	}
}

#define PK_RANDOMNESS_BYTES ((A_COLS + (A_COLS-A_ROWS-1)*A_ROWS)*6+100)

void gen_v_and_A(const unsigned char *public_seed,uint16_t *v, uint16_t *A){
	// expand public seed to get pseudorandomness to generate v and A from
	uint16_t A_randomness[PK_RANDOMNESS_BYTES/2];
	EXPAND(public_seed,SEED_BYTES,(unsigned char *) A_randomness,PK_RANDOMNESS_BYTES);

	// generate good v (i.e. nonzero and distinct entries)
	int cur_v = 0;
	int cur_rand = 0;
	while(cur_v < A_COLS){
		uint16_t candidate = (A_randomness[cur_rand] & FIELD_MASK);
		if( candidate < FIELD_PRIME && candidate > 0 ) {
			int good = 1;
			int i;
			for(i=0; i<cur_v; i++){
				if(candidate == v[i]){
					good = 0;
					break;
				}
			}
			if (good){
				v[cur_v++] = ( A_randomness[cur_rand] & FIELD_MASK);
			}
		}
		cur_rand++;
	}

	// generate A
	int cur_A = 0;
	while(cur_A < (A_COLS-A_ROWS-1)*A_ROWS){
		if( ( A_randomness[cur_rand] & FIELD_MASK) < FIELD_PRIME ) {
			A[cur_A++] = ( A_randomness[cur_rand] & FIELD_MASK);
		}
		cur_rand++;
	}
}

// embarrasingly slow, but thats ok
uint16_t minus_inverse(uint32_t a){
	uint32_t i;
	for(i=1; i<FIELD_PRIME; i++){
		if((i*a)%FIELD_PRIME == FIELD_PRIME-1){
			return (uint16_t) i;
		}
	}
	return 0;
}

void get_last_col(uint16_t *v, uint16_t *A, unsigned char *permutation_seed, uint16_t *col){
	// generate pi
	unsigned char pi[A_COLS];
	generate_permutation(permutation_seed, pi);

	// compute v_pi
	uint16_t v_pi[A_COLS];
	permute_vector_ct(v,pi,v_pi);

	// compute last col
	uint32_t last_col[A_ROWS] = {0};
	memcpy(last_col,v_pi,A_ROWS*sizeof(uint16_t));

	int i,j;
	for(i=0; i<A_COLS-A_ROWS-1; i++){
		for(j=0; j<A_ROWS; j++){
			last_col[j] += ((uint32_t) v_pi[A_ROWS+i])*((uint32_t) A[i*A_ROWS+j]);
		}
	}

	uint16_t c = minus_inverse(v_pi[A_COLS-1]);
	for(j=0; j<A_ROWS; j++){
		last_col[j] %= FIELD_PRIME;
		last_col[j] *= c;
		col[j] = (uint16_t) (last_col[j] % FIELD_PRIME);
	}
}

void generate_r_and_sigma(const unsigned char *seed, uint16_t *r, unsigned char *sigma){
	uint16_t A_randomness[A_COLS*3+50];
	EXPAND(seed,SEED_BYTES,(unsigned char *) A_randomness,(A_COLS*3+50)*sizeof(uint16_t));

	// generate r
	int cur_A = 0;
	int cur_rand = 0;
	while(cur_A < A_COLS){
		if( ( A_randomness[cur_rand] & FIELD_MASK) < FIELD_PRIME ) {
			r[cur_A++] = ( A_randomness[cur_rand] & FIELD_MASK);
		}
		cur_rand++;
	}

	generate_permutation((unsigned char *)&A_randomness[cur_rand],sigma);
}

void keygen(unsigned char *pk, unsigned char *sk){
	// pick random secret key
	RAND_bytes(	SK_SEED(sk) , SEED_BYTES);

	// expand secret key into a public seed and a permutation seed
	unsigned char keygen_buf[SEED_BYTES*2];
	EXPAND(SK_SEED(sk),SEED_BYTES,keygen_buf,SEED_BYTES*2);

	// copy public seed to public key
	memcpy(PK_SEED(pk),keygen_buf,SEED_BYTES);

	// generate v and A from public seed 
	uint16_t v[A_COLS];
	uint16_t A[(A_COLS-1)*A_ROWS];
	gen_v_and_A(PK_SEED(pk),v,A);

	// compute last column of A
	uint16_t last_col[A_ROWS];
	get_last_col(v,A,keygen_buf + SEED_BYTES, last_col);

	// compress last column
	compress_vecs(last_col,A_ROWS, PK_A_LAST_COL(pk));
}

void commit(const unsigned char *sk, const unsigned char *pk, unsigned char *commitment, unsigned char *state){
	uint16_t v[A_COLS];
	uint16_t A[(A_COLS-1)*A_ROWS];
	gen_v_and_A(PK_SEED(pk),v,A);

	// expand last column
	uint16_t last_col[A_ROWS];
	decompress_vecs(PK_A_LAST_COL(pk), A_ROWS,last_col);

	// expand secret key into a public seed and a permutation seed
	unsigned char keygen_buf[SEED_BYTES*2];
	EXPAND(SK_SEED(sk),SEED_BYTES,keygen_buf,SEED_BYTES*2);

	// generate pi
	unsigned char pi[A_COLS];
	generate_permutation(keygen_buf + SEED_BYTES, pi);

	// compute v_pi
	uint16_t v_pi[A_COLS];
	permute_vector_ct(v,pi,v_pi);

	unsigned char master_seed[SEED_BYTES];
	RAND_bytes(master_seed,SEED_BYTES);

	EXPAND(master_seed, SEED_BYTES, STATE_SEEDS(state), SEED_BYTES*2*ITERATIONS);

	unsigned char *commitments = STATE_HASHES(state);

	int inst;
	for (inst = 0; inst < ITERATIONS; inst++)
	{
		unsigned char buffer_1[A_ROWS*sizeof(uint16_t) + A_COLS] = {0};
		uint16_t *R_sigma = (uint16_t *) (STATE_R_SIGMA(state) + inst*A_COLS*sizeof(uint16_t));
		uint16_t *AR = (uint16_t *) buffer_1;
		unsigned char *sigma = buffer_1 + A_ROWS*sizeof(uint16_t);

		// generate sigma and R_sigma
		generate_permutation(STATE_SEEDS(state) + SEED_BYTES*2*inst, sigma);
		generate_vector(STATE_SEEDS(state) + SEED_BYTES*(1+2*inst), R_sigma);

		// compute R, from R_sigma and sigma
		uint16_t R[A_COLS];
		unpermute_vector_ct((uint16_t *)(STATE_R_SIGMA(state) + inst*A_COLS*sizeof(uint16_t)), sigma, R);

		// compute AR
		mat_mul(A,last_col , R, AR);

		// compute c0
		HASH(buffer_1, A_ROWS*sizeof(uint16_t) + A_COLS, commitments + inst*2*HASH_BYTES);

		// compute v_pi_sigma
		permute_vector_ct(v_pi,sigma,(uint16_t *) (STATE_VPISIGMA(state) + inst*A_COLS*sizeof(uint16_t)));

		// compute c1
		unsigned char buffer_2[A_COLS*sizeof(uint16_t)+SEED_BYTES];
		memcpy(buffer_2, STATE_VPISIGMA(state) + inst*A_COLS*sizeof(uint16_t), A_COLS*sizeof(uint16_t));
		memcpy(buffer_2+A_COLS*sizeof(uint16_t),STATE_SEEDS(state) + SEED_BYTES*(2*inst+1),SEED_BYTES);
		HASH(buffer_2,A_COLS*sizeof(uint16_t)+SEED_BYTES,commitments + (inst*2+1)*HASH_BYTES);

		uint16_t *V_pi_sigma = (uint16_t *) buffer_2;
	}

	// combine all the commitments
	HASH(commitments,HASH_BYTES*2*ITERATIONS, commitment);
}

void respond1(const unsigned char *sk, const unsigned char *pk, uint16_t *c, unsigned char *response1, unsigned char *state){
	uint16_t v[A_COLS];
	uint16_t A[(A_COLS-1)*A_ROWS];
	gen_v_and_A(PK_SEED(pk),v,A);

	uint16_t response_vecs[A_COLS*ITERATIONS];

	int inst;
	for (inst =0; inst < ITERATIONS; inst++){
		uint16_t* resp = response_vecs + A_COLS*inst;
		uint16_t* v_pi_sigma = (uint16_t *) (STATE_VPISIGMA(state) + inst*A_COLS*sizeof(uint16_t));

		memcpy(resp, STATE_R_SIGMA(state) + inst*A_COLS*sizeof(uint16_t), A_COLS*sizeof(uint16_t));

		// generate sigma
		unsigned char sigma[A_COLS];
		generate_permutation(STATE_SEEDS(state) + SEED_BYTES*2*inst, sigma);

		int i;
		for (i=0; i<A_COLS; i++){
			resp[i] = (resp[i] + v_pi_sigma[i]*c[inst]) % FIELD_PRIME;
		}
	}

	compress_vecs(response_vecs,A_COLS*ITERATIONS,response1);
}

void respond2(const unsigned char *sk, const unsigned char *pk, unsigned char *b, unsigned char *response2, unsigned char *state){
	int inst;
	for (inst =0; inst<ITERATIONS; inst ++){
		memcpy(RESPONSE2_SEEDS(response2) + inst*SEED_BYTES, STATE_SEEDS(state) + (2*inst + (b[inst]%2))*SEED_BYTES , SEED_BYTES) ; 

		memcpy(RESPONSE2_HASHES(response2) + inst*HASH_BYTES, STATE_HASHES(state) + (2*inst + ((b[inst]+1)%2))*HASH_BYTES , HASH_BYTES) ; 
	}
}

// check if two vectors are permutations of one another
int is_permutation(uint16_t *A, uint16_t *B){
	int count[FIELD_PRIME] = {0};

	for (int i = 0; i < A_COLS; ++i)
	{
		count[A[i]] ++;
	}

	for (int i = 0; i < A_COLS; ++i)
	{
		count[B[i]] --;
		if(count[B[i]] < 0){
			return 0;
		}
	}

	return 1;
}

int check(const unsigned char *pk, const unsigned char *commitment, const uint16_t *c, const unsigned char *response1, const unsigned char *b, const unsigned char *response2){
	uint16_t v[A_COLS];
	uint16_t A[(A_COLS-1)*A_ROWS];
	gen_v_and_A(PK_SEED(pk),v,A);

	// expand last column
	uint16_t last_col[A_ROWS];
	decompress_vecs(PK_A_LAST_COL(pk),A_ROWS,last_col);

	unsigned char commitments[HASH_BYTES*2*ITERATIONS];

	uint16_t response_vecs[A_COLS*ITERATIONS];
	decompress_vecs(response1,A_COLS*ITERATIONS,response_vecs);

	int inst;
	for (inst=0; inst < ITERATIONS; inst ++){
		uint16_t *X;
		X = response_vecs + inst*A_COLS;

		if (b[inst] % 2 == 0){
			unsigned char buffer_1[A_ROWS*sizeof(uint16_t) + A_COLS] = {0};
			uint16_t *AR = (uint16_t *) buffer_1;
			unsigned char *sigma = buffer_1 + A_ROWS*sizeof(uint16_t);

			generate_permutation(RESPONSE2_SEEDS(response2) + SEED_BYTES*inst, sigma);

			uint16_t tmp[A_COLS];
			unpermute_vector(X,sigma,tmp);

			mat_mul(A,last_col, tmp, AR);

			HASH(buffer_1, A_ROWS*sizeof(uint16_t) + A_COLS, commitments + 2*HASH_BYTES*inst);
			memcpy(commitments + (2*inst+1)*HASH_BYTES, RESPONSE2_HASHES(response2) + inst*HASH_BYTES, HASH_BYTES);
		}
		else{

			// compute c1
			unsigned char buffer_2[A_COLS*sizeof(uint16_t)+SEED_BYTES];
			uint16_t *V_pi_sigma = (uint16_t *) buffer_2;
			memcpy(buffer_2+A_COLS*sizeof(uint16_t),RESPONSE2_SEEDS(response2) + SEED_BYTES*inst, SEED_BYTES);
			

			// compute if X-R_sigma is permutation of c*V
			uint16_t tmp[A_COLS];
			generate_vector(RESPONSE2_SEEDS(response2) + SEED_BYTES*inst, tmp);

			uint16_t c_inv = (FIELD_PRIME - minus_inverse(c[inst])) % FIELD_PRIME;

			int i;
			for (i = 0; i < A_COLS; i++)
			{
				tmp[i] = (X[i] + FIELD_PRIME - tmp[i]) % FIELD_PRIME;
				V_pi_sigma[i] = ( tmp[i] * c_inv ) % FIELD_PRIME;
			}

			HASH(buffer_2,A_COLS*sizeof(uint16_t)+SEED_BYTES,commitments + (inst*2+1)*HASH_BYTES);

			if( !is_permutation(v,V_pi_sigma)){
				printf("Not a permutation of v! \n");
				return -1;
			}

			memcpy(commitments + (2*inst)*HASH_BYTES, RESPONSE2_HASHES(response2) + inst*HASH_BYTES, HASH_BYTES);	
		}
	}

	unsigned char out[HASH_BYTES];
	HASH(commitments,HASH_BYTES*2*ITERATIONS,out);

	if(memcmp(out,commitment,HASH_BYTES) != 0){
		printf("Hashes dont match! \n");
		return -1;
	}
	return 0;
}