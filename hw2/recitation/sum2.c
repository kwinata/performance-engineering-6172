// Copyright (c) 2012 MIT License by 6.172 Staff

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint32_t data_t;
const int U = 10000000;   // size of the array. 10 million vals ~= 40MB
const int N = 100000000;  // number of searches to perform

int comp (const void * elem1, const void * elem2) 
{
    int f = *((int*)elem1);
    int s = *((int*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

int main() {
  data_t* data = (data_t*) malloc(U * sizeof(data_t));
  if (data == NULL) {
    free(data);
    printf("Error: not enough memory\n");
    exit(-1);
  }

  // fill up the array with sequential (sorted) values.
  int i;
  for (i = 0; i < U; i++) {
    data[i] = i;
  }

  printf("Allocated array of size %d\n", U);
  printf("Summing %d random values...\n", N);

  data_t val = 0;
  data_t seed = 42;

  int repetition = 10000;
  int n = N/repetition;

  data_t* indexes = (data_t*) malloc(n * sizeof(data_t));
  if (indexes == NULL) {
    free(indexes);
    printf("Error: not enough memory for indexes");
    exit(-1);
  }

  for (int repeat = 0; repeat < repetition; repeat++){
    for (i = 0; i < n; i++) {
      indexes[i] = rand_r(&seed) % U;
    }
    qsort(indexes, n, sizeof(indexes[0]), comp);
    for (i = 0; i < n; i++){
      val = (val + data[i]);
    }
  }


  free(data);
  free(indexes);
  printf("Done. Value = %d\n", val);
  return 0;
}
