#include "SHA3api_ref.h"
#include "skein.h"
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>

#define BUF_SIZE 128
#define HASH_BITS 1024
static const int HASHES_BEFORE_BENCHMARK = 1000000;

static const char *param = "hashable=";

static bool benchmarkMode = false;

static const unsigned char target[HASH_BITS/8] = {0x5b, 0x4d, 0xa9, 0x5f, 0x5f, 0xa0, 0x82, 0x80, 0xfc, 0x98, 0x79, 0xdf, 0x44, 0xf4, 0x18, 0xc8, 0xf9, 0xf1, 0x2b, 0xa4, 0x24, 0xb7, 0x75, 0x7d, 0xe0, 0x2b, 0xbd, 0xfb, 0xae, 0x0d, 0x4c, 0x4f, 0xdf, 0x93, 0x17, 0xc8, 0x0c, 0xc5, 0xfe, 0x04, 0xc6, 0x42, 0x90, 0x73, 0x46, 0x6c, 0xf2, 0x97, 0x06, 0xb8, 0xc2, 0x59, 0x99, 0xdd, 0xd2, 0xf6, 0x54, 0x0d, 0x44, 0x75, 0xcc, 0x97, 0x7b, 0x87, 0xf4, 0x75, 0x7b, 0xe0, 0x23, 0xf1, 0x9b, 0x8f, 0x40, 0x35, 0xd7, 0x72, 0x28, 0x86, 0xb7, 0x88, 0x69, 0x82, 0x6d, 0xe9, 0x16, 0xa7, 0x9c, 0xf9, 0xc9, 0x4c, 0xc7, 0x9c, 0xd4, 0x34, 0x7d, 0x24, 0xb5, 0x67, 0xaa, 0x3e, 0x23, 0x90, 0xa5, 0x73, 0xa3, 0x73, 0xa4, 0x8a, 0x5e, 0x67, 0x66, 0x40, 0xc7, 0x9c, 0xc7, 0x01, 0x97, 0xe1, 0xc5, 0xe7, 0xf9, 0x02, 0xfb, 0x53, 0xca, 0x18, 0x58, 0xb6};

static inline int countBits(uint32_t i)
{
  i = i - ((i >> 1) & 0x55555555);
  i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
  return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}

static inline int hammingDistance(const unsigned char *hash1, 
                                  const unsigned char *hash2,
                                  size_t len) {
  const uint32_t *x = (const uint32_t*) hash1;
  const uint32_t *y = (const uint32_t*) hash2;
  len /= sizeof(uint32_t);

  int total = 0;
  while (len-- > 0)
    total += countBits(*x++ ^ *y++);
  return total;
}

void submitData() {
  pid_t pid = fork();
  if (pid) {
    // parent
    int status = -1;
    waitpid(pid, &status, 0);
    if (status != 0)
      perror("Child error");
  } else {
    // child
    char *args[] = {"curl", "-X", "POST", "--data", "@data.txt", "http://almamater.xkcd.com/?edu=stanford.edu", "--header", "Content-Type:application/octet-stream", NULL};
    execvp("curl", args);
    exit(-1);
  }
}

void normalizeBuffer(unsigned char *buf, int size) {
  for (int i = 0; i < size; i++) {
    buf[i] = buf[i] % 62;
    if(buf[i] < 26) {
      buf[i] = buf[i] + 'a';
    } else if(buf[i] < 52) {
      buf[i] = buf[i] - 26 + 'A';
    } else {
      buf[i] = buf[i] - 52 + '0';
    }
  }
}

// We arbitrarily choose to sequence numbers 0-9, a-z, A-Z
void incrementArbitraryPrecision(unsigned char *data, size_t len) {
  for (int i = len - 1; i >= 0; i--) {
    if (data[i] == '9') {
      data[i] = 'a';
      break;
    } else if (data[i] == 'z') {
      data[i] = 'A';
      break;
    } else if (data[i] != 'Z') {
      data[i]++;
      break;
    } else {
      data[i] = '0';
    }
  }
}

int best = HASH_BITS;
pthread_mutex_t bestLock;

void *hashThread(void *aux) {
  int threadNum = *(int*) aux;
  printf("Thread %d started\n", threadNum);

  int threadBest = HASH_BITS;

  int randfd = open("/dev/urandom", O_RDONLY);
  if(randfd < 0)
    perror("can't open random file");

  unsigned char buf[BUF_SIZE + 1];
  buf[BUF_SIZE] = 0;
  read(randfd, buf, BUF_SIZE);
  normalizeBuffer(buf, BUF_SIZE);

  int numHashes = 0;
  struct timeval startTimestamp;
  gettimeofday(&startTimestamp, NULL);

  while(true) {
    incrementArbitraryPrecision(buf, BUF_SIZE);
    unsigned char hash[HASH_BITS / CHAR_BIT];
    Hash(HASH_BITS, buf, BUF_SIZE * CHAR_BIT, hash);

    if(threadNum == 0 && benchmarkMode) {
      numHashes++;
      if(numHashes % HASHES_BEFORE_BENCHMARK == 0) {
        struct timeval timestamp;
        gettimeofday(&timestamp, NULL);

        double sec = (timestamp.tv_sec - startTimestamp.tv_sec) +
                     (timestamp.tv_usec - startTimestamp.tv_usec) / 1000000.0;
        printf("Thread 0: %0.f hashes/sec (%d hashes in %.5fs)\n",
               numHashes / sec,
               numHashes, sec);
      }
    }

    int distance = hammingDistance(target, hash, HASH_BITS/8);
    if (!benchmarkMode && distance < threadBest) {
      threadBest = distance;

      // check if this is actually the best over all threads
      if(threadBest >= best) {
        threadBest = best;
        continue;
      }
      // If here, this thread actually has the best hash
      pthread_mutex_lock(&bestLock);

      printf("\n\nDistance: %d\n", distance);
      best = distance;

      printf("Data: %s\n", buf);

      // Only invoke curl if we're not in benchmark mode.
      if (benchmarkMode) {
        continue;
      }

      int outfd = open("data.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
      if(outfd < 0)
        perror("can't open output file");

      write(outfd, param, strlen(param));
      write(outfd, buf, BUF_SIZE);
      fsync(outfd);
      close(outfd);

      submitData();

      pthread_mutex_unlock(&bestLock);
    }
  }
  return NULL; 
}


int main(int argc, char** argv) {
  pthread_mutex_init(&bestLock, NULL);

  if (argc == 2 && !strcmp("--benchmark", argv[1])) {
    benchmarkMode = true;
  }

  long numCPUs;
  if (!benchmarkMode) {
    numCPUs = sysconf(_SC_NPROCESSORS_ONLN); // Get number of cores
  } else {
    numCPUs = 1;
  }

  pthread_t *threads = malloc(sizeof(pthread_t) * numCPUs);
  int *threadNums = malloc(sizeof(int) * numCPUs);

  for(int i = 0; i < numCPUs; i++) {
    threadNums[i] = i;
    int err = pthread_create(threads + i, NULL, hashThread, threadNums + i);
    if(err < 0)
      perror("Child thread");
  }

  // Join threads. In non-benchmark mode, they never terminate, which means
  // this will just block.
  for(int i = 0; i < numCPUs; i++) {
    int err = pthread_join(threads[i], NULL);
    if(err < 0)
      perror("Child join");
  }
  return 0;
}
