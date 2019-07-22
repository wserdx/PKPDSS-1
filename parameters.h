
#include <openssl/sha.h>
#include "libkeccak.a.headers/SimpleFIPS202.h"

#define SL1
//#define SL3
//#define SL5

#ifdef SL1

	#define A_COLS 69
	#define A_ROWS 41
	#define FIELD_PRIME 251
	#define FIELD_BITS 8
	#define SEED_BYTES 16
	#define HASH_BYTES 32
	#define ITERATIONS 129

#endif

#ifdef SL3

	#define A_COLS 94
	#define A_ROWS 54
	#define FIELD_PRIME 509
	#define FIELD_BITS 9
	#define SEED_BYTES 24
	#define HASH_BYTES 48
	#define ITERATIONS 193

#endif

#ifdef SL5

	#define A_COLS 106
	#define A_ROWS 47
	#define FIELD_PRIME 4093
	#define FIELD_BITS 12
	#define SEED_BYTES 32
	#define HASH_BYTES 64
	#define ITERATIONS 257

#endif

#ifdef TEST

#define A_COLS 87
#define A_ROWS 42
#define FIELD_PRIME 1409
#define FIELD_BITS 11

#define PERM_BITS 7
#define SEED_BYTES 16
#define LEAF_BYTES (A_COLS*sizeof(uint16_t))
#define DEPTH  4
#define SEED_DEPTH 8
#define LEAVES (1 << DEPTH)
#define HASH_BYTES 32
#define TREE_BYTES ((2*LEAVES-1)*HASH_BYTES)
#define PATH_BYTES (DEPTH*HASH_BYTES)
#define HASHES  (1<<9)

#define SETUPS 242
#define EXECUTIONS 33

#endif

#ifdef COMPACTT

#define A_COLS 61
#define A_ROWS 28
#define FIELD_PRIME 977
#define FIELD_BITS 10

#define PERM_BITS 6
#define SEED_BYTES 16
#define LEAF_BYTES (A_COLS*sizeof(uint16_t))
#define DEPTH  7
#define SEED_DEPTH 11
#define LEAVES (1 << DEPTH)
#define HASH_BYTES 32
#define TREE_BYTES ((2*LEAVES-1)*HASH_BYTES)
#define PATH_BYTES (DEPTH*HASH_BYTES)
#define HASHES (1<<16)

#define SETUPS  1011
#define EXECUTIONS 16

#endif

#ifdef FASTT

#define A_COLS 61
#define A_ROWS 28
#define FIELD_PRIME 977
#define FIELD_BITS 10

#define PERM_BITS 6
#define SEED_BYTES 16
#define LEAF_BYTES (A_COLS*sizeof(uint16_t))
#define DEPTH  4
#define SEED_DEPTH 8
#define LEAVES (1 << DEPTH)
#define HASH_BYTES 32
#define TREE_BYTES ((2*LEAVES-1)*HASH_BYTES)
#define PATH_BYTES (DEPTH*HASH_BYTES)
#define HASHES  (1<<9)

#define SETUPS 242
#define EXECUTIONS 33

#endif

#ifdef TOY

#define HASHES 50
#define PERM_BITS 2
#define A_COLS 3
#define A_ROWS 2
#define FIELD_PRIME 7
#define FIELD_BITS 3

#define SEED_BYTES 16
#define LEAF_BYTES (A_COLS*sizeof(uint16_t))
#define DEPTH  2
#define SEED_DEPTH 4
#define LEAVES (1 << DEPTH)
#define HASH_BYTES 32
#define TREE_BYTES ((2*LEAVES-1)*HASH_BYTES)
#define PATH_BYTES (DEPTH*HASH_BYTES)

#define SETUPS 15
#define EXECUTIONS 14

#endif

#define LEAVES (1 << DEPTH)
#define LEAF_BYTES (A_COLS*sizeof(uint16_t))
#define TREE_BYTES ((2*LEAVES-1)*HASH_BYTES)
#define PATH_BYTES (DEPTH*HASH_BYTES)

#define HASH(data,len,out) SHAKE128(out, HASH_BYTES, data, len);
#define EXPAND(data,len,out,outlen) SHAKE128(out, outlen, data, len);

#ifdef CHACHA
	#define HASH(data,len,out) SHAKE128(out, HASH_BYTES, data, len);
	#define EXPAND(data,len,out,outlen) SHAKE128(out, outlen, data, len);
#endif

#define FIELD_MASK ((1<<FIELD_BITS) -1)
#define POS_MASK ((1<<SEED_DEPTH) -1)
