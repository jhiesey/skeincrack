CC = gcc
CFLAGS = -O3 -Wall -ICD/Optimized_64bit --std=c99 -pthread -fno-strict-aliasing

all: skeincrack

skeincrack: Makefile skeincrack.c
	$(CC) $(CFLAGS) CD/Optimized_64bit/*.c skeincrack.c -o $@

.PHONY: clean
clean:
	rm -f *.o skeincrack
