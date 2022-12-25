### Check off item 1

> Make note of one bottleneck

Perf result:

``` bash
> sudo perf stat ./isort 10000 10

Sorting 10000 values...
Done!

 Performance counter stats for './isort 10000 10':

            546.63 msec task-clock                #    0.992 CPUs utilized          
                45      context-switches          #    0.082 K/sec                  
                 0      cpu-migrations            #    0.000 K/sec                  
                77      page-faults               #    0.141 K/sec                  
   <not supported>      cycles                                                      
   <not supported>      instructions                                                
   <not supported>      branches                                                    
   <not supported>      branch-misses                                               

       0.550821982 seconds time elapsed

       0.546772000 seconds user
       0.000000000 seconds sys
```

It's not possible to see `instructions`, `branches`, `branch-misses` because it's running in a virtualized machine (Github Codespaces)


### Checkoff Item 2:

> Run sum under cachegrind to identify cache performance. It may take a
little while. In the output, look at the D1 and LLd misses. D1 represents the lowest-level
cache (L1), and LL represents the last (highest) level data cache (on most machines, L3). Do
these numbers correspond with what you would expect? Try playing around with the
values N and U in sum.c. How can you bring down the number of cache misses?

``` bash
> valgrind --tool=cachegrind --branch-sim=yes ./sum

==8813== Cachegrind, a cache and branch-prediction profiler
==8813== Copyright (C) 2002-2017, and GNU GPL'd, by Nicholas Nethercote et al.
==8813== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
==8813== Command: ./sum
==8813== 
--8813-- warning: L3 cache found, using its data for the LL simulation.
--8813-- warning: specified LL cache: line_size 64  assoc 11  total_size 37,486,592
--8813-- warning: simulated LL cache: line_size 64  assoc 18  total_size 37,748,736
Allocated array of size 10000000
Summing 100000000 random values...
Done. Value = 938895920
==8813== 
==8813== I   refs:      3,540,274,098
==8813== I1  misses:            1,220
==8813== LLi misses:            1,203
==8813== I1  miss rate:          0.00%
==8813== LLi miss rate:          0.00%
==8813== 
==8813== D   refs:        610,079,685  (400,062,719 rd   + 210,016,966 wr)
==8813== D1  misses:      100,548,151  ( 99,922,269 rd   +     625,882 wr)
==8813== LLd misses:        6,316,060  (  5,690,244 rd   +     625,816 wr)
==8813== D1  miss rate:          16.5% (       25.0%     +         0.3%  )
==8813== LLd miss rate:           1.0% (        1.4%     +         0.3%  )
==8813== 
==8813== LL refs:         100,549,371  ( 99,923,489 rd   +     625,882 wr)
==8813== LL misses:         6,317,263  (  5,691,447 rd   +     625,816 wr)
==8813== LL miss rate:            0.2% (        0.1%     +         0.3%  )
==8813== 
==8813== Branches:        210,056,297  (110,055,766 cond + 100,000,531 ind)
==8813== Mispredicts:           6,432  (      6,208 cond +         224 ind)
==8813== Mispred rate:            0.0% (        0.0%     +         0.0%   )
```

D1 misses (L1 cache): 100M
LLd misses (L3 cache): 6M

Is this expected?

From `lscpu` we can find the cache sizes:

```
L1d cache:                       32 KiB
L1i cache:                       32 KiB
L2 cache:                        1 MiB
L3 cache:                        35.8 MiB
```

**L1**

L1 cache size is 32KiB. Each entry size is 4B (uint32). So we should be able to fit 8096 entries in the cache.

The chance of L1 hit is 8096 / 10,000,000 ~ 1/1200. Since we have 100,000,000 references. We should have 100,000,000 * 1199/1200 misses ~ 100M. So should be expected.

**L3**

L3 cache size is 36MiB. Should be able to hold 9M entries. The cache of hit is 9,000,000/10,000,0000 = 9/10. Chance of miss is therefore 1/10. Thus we should have 10,0000,000 misses. This should be quite close to the 6M that is found in the simulated data.

**How to bring down the miss count?**

We can sort the numbers being randomized. This should have better performance due to cache locality of most sort algorithms. We can use the built-in qsort.

``` c
  data_t val = 0;
  data_t seed = 42;

  int n = N/10;

  data_t* indexes = (data_t*) malloc(n * sizeof(data_t));
  if (indexes == NULL) {
    free(indexes);
    printf("Error: not enough memory for indexes");
    exit(-1);
  }

  for (int repeat = 0; repeat < 10; repeat++){
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
```

``` bash
> valgrind --tool=cachegrind --branch-sim=yes ./sum
==29595== Cachegrind, a cache and branch-prediction profiler
==29595== Copyright (C) 2002-2017, and GNU GPL'd, by Nicholas Nethercote et al.
==29595== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
==29595== Command: ./sum
==29595== 
--29595-- warning: L3 cache found, using its data for the LL simulation.
--29595-- warning: specified LL cache: line_size 64  assoc 11  total_size 37,486,592
--29595-- warning: simulated LL cache: line_size 64  assoc 18  total_size 37,748,736
Allocated array of size 10000000
Summing 100000000 random values...
Done. Value = 1332236160
==29595== 
==29595== I   refs:      88,397,701,996
==29595== I1  misses:             1,273
==29595== LLi misses:             1,256
==29595== I1  miss rate:           0.00%
==29595== LLi miss rate:           0.00%
==29595== 
==29595== D   refs:      56,934,782,642  (33,819,946,549 rd   + 23,114,836,093 wr)
==29595== D1  misses:       311,847,951  (   156,985,507 rd   +    154,862,444 wr)
==29595== LLd misses:        55,656,469  (    27,240,178 rd   +     28,416,291 wr)
==29595== D1  miss rate:            0.5% (           0.5%     +            0.7%  )
==29595== LLd miss rate:            0.1% (           0.1%     +            0.1%  )
==29595== 
==29595== LL refs:          311,849,224  (   156,986,780 rd   +    154,862,444 wr)
==29595== LL misses:         55,657,725  (    27,241,434 rd   +     28,416,291 wr)
==29595== LL miss rate:             0.0% (           0.0%     +            0.1%  )
==29595== 
==29595== Branches:      14,619,987,781  (12,173,689,115 cond +  2,446,298,666 ind)
==29595== Mispredicts:    1,290,689,254  ( 1,290,689,019 cond +            235 ind)
==29595== Mispred rate:             8.8% (          10.6%     +            0.0%   )
```

D1 misses (L1 cache): 300M
LLd misses (L3 cache): 55M

So it's still too high. Maybe this is due to the heavy operation of quick sort.

** Reducing Quick Sort size **

Previously it will quick sort 1M elements. Let's bring it down to 100k.

If we reduce the quick sort size (100K elements):

``` c
 int repetition = 100;
  int n = N/repetition;

  data_t* indexes = (data_t*) malloc(n * sizeof(data_t));
  if (indexes == NULL) {
    free(indexes);
    printf("Error: not enough memory for indexes");
    exit(-1);
  }

  for (int repeat = 0; repeat < repetition; repeat++){
    printf("repetition %i\n", repeat);
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
```

We will get:
``` bash
==7860== D1  misses:       222,658,321  (   112,825,788 rd   +   109,832,533 wr)
==7860== LLd misses:           878,315  (        65,014 rd   +       813,301 wr)
==7860== D1  miss rate:            0.9% (           0.7%     +           1.3%  )
==7860== LLd miss rate:            0.0% (           0.0%     +           0.0%  )
```

As we can see, L1 is till higher (222M > 100M), but the LLd is already down from  6M to 878k.

And if we experiment with even smaller quick sort (10k elements):

``` bash
==12085== D1  misses:       142,185,920  (    73,511,256 rd   +    68,674,664 wr)
==12085== LLd misses:           653,310  (         8,765 rd   +       644,545 wr)
==12085== D1  miss rate:            0.7% (           0.5%     +           0.9%  )
==12085== LLd miss rate:            0.0% (           0.0%     +           0.0%  )
```

But the improvements doens't improve really much.

And then if we make the quicksort with 1k elements, suddenly the D1 misses drop:
``` bash
==23190== D1  misses:           629,141  (        3,160 rd   +       625,981 wr)
==23190== LLd misses:           628,489  (        2,573 rd   +       625,916 wr)
```

This must be because the items in the quick sort can fit in the L1 cache (8k items). We can experiment with 5k elements, should still be quite good:

``` bash
==30922== D1  misses:        33,062,639  (   15,478,164 rd   +    17,584,475 wr)
==30922== LLd misses:           629,239  (        2,823 rd   +       626,416 wr)
==30922== D1  miss rate:            0.2% (          0.2%     +           0.3%  )
==30922== LLd miss rate:            0.0% (          0.0%     +           0.0%  )
```

So turns out it's not very good, but it still improved quite significantly.

Since the D1 misses is already almost the same as LLD misses. Theere's no thing much that we could improve over the 1k elements quick sort.

To estimate the CPU cycle improvement from here:
L1 refs: 4 cycles
L2 refs: 10 cycles
L3 refs: 50 cycles
DRAM refs: 150 cycles

We can assume that miss will mean the higher level refs:
L1 miss: 10 cycles
L3 miss: 150 cycles.

Originally we have 100M L1 miss * 10 cycles + 6M L3 miss * 150 cycles = 1B cycles + 900M cycles = 1.9B cycles

In the improved version it should have 600k L1 miss * 10 cycles + 600k L3 miss * 150 cycles = 6M + 90M ~ 100M cycles

In terms of the memory references, the optimized version is using is a 19 times improvement (only uses ~5%) compared to the original code.

But in terms of running time it is actually is worse due to the quicksort overhead.

```
> time ./sum

Allocated array of size 10000000
Summing 100000000 random values...
Done. Value = 938895920

real    0m2.737s
user    0m2.605s
sys     0m0.012s


> time ./sum2

Allocated array of size 10000000
Summing 100000000 random values...
Done. Value = 1733793664

real    0m9.551s
user    0m9.337s
sys     0m0.027s
```