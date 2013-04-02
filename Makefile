CC = gcc
CFLAGS = -O3 -Wall -ICD/Optimized_64bit --std=c99 -pthread -fno-strict-aliasing -flto
DEFINES = -DSKEIN_LOOP=0
SRCS = CD/Optimized_64bit/*.c skeincrack.c

all: skeincrack

skeincrack: Makefile skeincrack.c
	$(CC) $(CFLAGS) $(DEFINES) $(SRCS) -o $@

pgo: Makefile skeincrack.c
	$(CC) $(CFLAGS) $(DEFINES) -fprofile-generate $(SRCS) -o skeincrack
	./skeincrack --pgo
	$(CC) $(CFLAGS) $(DEFINES) -fprofile-use $(SRCS) -o skeincrack

.PHONY: clean
clean:
	rm -f *.o *.gcda skeincrack
