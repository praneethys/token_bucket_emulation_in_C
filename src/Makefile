# 
# Token Bucket Emulation in C (Multi-threading)
# Author: Praneeth Yerrapragada
#
 
warmup2: warmup2.o my402list.o
	gcc -o warmup2 -g warmup2.o my402list.o -lpthread -lm

warmup2.o: warmup2.c warmup2.h my402list.h globals.h
	gcc -g -c -Wall -D_POSIX_PTHREAD_SEMANTICS warmup2.c
	
my402list.o: my402list.c my402list.h
	gcc -g -c -Wall my402list.c

all: warmup2

clean:
	rm -f *.o warmup2

