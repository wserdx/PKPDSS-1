CC=gcc
CFLAGS=-I XKCP/bin/generic64/ 
LFLAGS=-L XKCP/bin/generic64/ -lkeccak -lcrypto

IMPLEMENTATION_SOURCE = sign.c test.c sushsy.c 
IMPLEMENTATION_HEADERS= sign.h keccaklib sushsy.h parameters.h

test: $(IMPLEMENTATION_SOURCE) $(IMPLEMENTATION_HEADERS)
	gcc -o test $(IMPLEMENTATION_SOURCE) $(CFLAGS) $(LFLAGS) -std=c11 -O3 -g -march=native -DSUSHSY


keccaklib: 
	(cd XKCP; make generic64/libkeccak.a)

.PHONY: clean
clean:
	rm -f PQCgenKAT_sign test debug test_offline intermediateValues.txt *.req *.rsp >/dev/null