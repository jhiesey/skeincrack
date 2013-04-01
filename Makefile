CC = gcc
CFLAGS = -O3 -Wall -ICD/Optimized_64bit --std=c99 -pthread -fno-strict-aliasing
DEFINES = -DSKEIN_LOOP=0

all: skeincrack

skeincrack: Makefile skeincrack.c
	$(CC) $(CFLAGS) $(DEFINES) CD/Optimized_64bit/*.c skeincrack.c -o $@

.PHONY: clean
clean:
	rm -f *.o skeincrack
