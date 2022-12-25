# Homework week 2

## Sorting

### Write-up 1

> Compare the Cachegrind output on the DEBUG=1 code versus DEBUG=0 compiler optimized code. Explain the advantages and disadvantages of using instruction count as a substitute for time when you compare the performance of different versions of this program.

Without DEBUG

``` bash
$ valgrind --tool=cachegrind --branch-sim=yes ./sort 10000 10
==15572== Cachegrind, a cache and branch-prediction profiler
==15572== Copyright (C) 2002-2017, and GNU GPL'd, by Nicholas Nethercote et al.
==15572== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
==15572== Command: ./sort 10000 10
==15572== 
--15572-- warning: L3 cache found, using its data for the LL simulation.
--15572-- warning: specified LL cache: line_size 64  assoc 11  total_size 37,486,592
--15572-- warning: simulated LL cache: line_size 64  assoc 18  total_size 37,748,736

Running test #0...
Generating random array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.029980 sec
sort_a repeated : Elapsed execution time: 0.031803 sec
Generating inverted array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.057838 sec
sort_a repeated : Elapsed execution time: 0.058469 sec

Running test #1...
 --> test_zero_element at line 245: PASS

Running test #2...
 --> test_one_element at line 266: PASS
Done testing.
==15572== 
==15572== I   refs:      231,863,812
==15572== I1  misses:          1,586
==15572== LLi misses:          1,488
==15572== I1  miss rate:        0.00%
==15572== LLi miss rate:        0.00%
==15572== 
==15572== D   refs:       88,031,064  (53,517,982 rd   + 34,513,082 wr)
==15572== D1  misses:        317,115  (   175,008 rd   +    142,107 wr)
==15572== LLd misses:          5,230  (     2,511 rd   +      2,719 wr)
==15572== D1  miss rate:         0.4% (       0.3%     +        0.4%  )
==15572== LLd miss rate:         0.0% (       0.0%     +        0.0%  )
==15572== 
==15572== LL refs:           318,701  (   176,594 rd   +    142,107 wr)
==15572== LL misses:           6,718  (     3,999 rd   +      2,719 wr)
==15572== LL miss rate:          0.0% (       0.0%     +        0.0%  )
==15572== 
==15572== Branches:       38,056,382  (36,355,518 cond +  1,700,864 ind)
==15572== Mispredicts:     2,502,963  ( 2,502,602 cond +        361 ind)
==15572== Mispred rate:          6.6% (       6.9%     +        0.0%   )
```

With DEBUG=1

``` bash
$ valgrind --tool=cachegrind --branch-sim=yes ./sort 10000 10
==22241== Cachegrind, a cache and branch-prediction profiler
==22241== Copyright (C) 2002-2017, and GNU GPL'd, by Nicholas Nethercote et al.
==22241== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
==22241== Command: ./sort 10000 10
==22241== 
--22241-- warning: L3 cache found, using its data for the LL simulation.
--22241-- warning: specified LL cache: line_size 64  assoc 11  total_size 37,486,592
--22241-- warning: simulated LL cache: line_size 64  assoc 18  total_size 37,748,736

Running test #0...
Generating random array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.052500 sec
sort_a repeated : Elapsed execution time: 0.055237 sec
Generating inverted array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.106712 sec
sort_a repeated : Elapsed execution time: 0.117662 sec

Running test #1...
 --> test_zero_element at line 245: PASS

Running test #2...
 --> test_one_element at line 266: PASS
Done testing.
==22241== 
==22241== I   refs:      415,037,082
==22241== I1  misses:          1,567
==22241== LLi misses:          1,484
==22241== I1  miss rate:        0.00%
==22241== LLi miss rate:        0.00%
==22241== 
==22241== D   refs:      263,700,877  (196,294,045 rd   + 67,406,832 wr)
==22241== D1  misses:        314,608  (    173,270 rd   +    141,338 wr)
==22241== LLd misses:          5,224  (      2,504 rd   +      2,720 wr)
==22241== D1  miss rate:         0.1% (        0.1%     +        0.2%  )
==22241== LLd miss rate:         0.0% (        0.0%     +        0.0%  )
==22241== 
==22241== LL refs:           316,175  (    174,837 rd   +    141,338 wr)
==22241== LL misses:           6,708  (      3,988 rd   +      2,720 wr)
==22241== LL miss rate:          0.0% (        0.0%     +        0.0%  )
==22241== 
==22241== Branches:       42,793,899  ( 41,093,052 cond +  1,700,847 ind)
==22241== Mispredicts:     3,453,428  (  3,453,075 cond +        353 ind)
==22241== Mispred rate:          8.1% (        8.4%     +        0.0%   )
```

My cachegrind doesn't provide instruction count data. However wew can estimate from the branches count. In DEBUG mode it has 42M branches and non debug is only 38M.

### Write-up 2

> Write-up 2: Explain which functions you chose to inline and report the performance differences you observed between the inlined and uninlined sorting routines.

I choose to inline `copy_i` function because the body of the function is short.

Improvement:

```
Running test #0...
Generating random array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.001056 sec
sort_i          : Elapsed execution time: 0.001070 sec
Generating inverted array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.001616 sec
sort_i          : Elapsed execution time: 0.001573 sec

Running test #1...
 --> test_zero_element at line 245: PASS

Running test #2...
 --> test_one_element at line 266: PASS
Done testing.
```

### Write-up 3

> Explain the possible performance downsides of inlining recursive functions. How could profiling data gathered using cachegrind help you measure these negative performance effects

For `copy_i` inlining: Yes it does inline (only works for not debug mode

Let's now try to inline `sort_i`: I cannot really reproduce this with my `clang`, but manage to do it with `gcc`

**Performance comparison:**

Without inline:

```
Running test #0...
Generating random array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.001157 sec
sort_i          : Elapsed execution time: 0.001189 sec
Generating inverted array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.001766 sec
sort_i          : Elapsed execution time: 0.001716 sec
```

With inlining:

```
Running test #0...
Generating random array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.001559 sec
sort_i          : Elapsed execution time: 0.001708 sec
Generating inverted array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.002747 sec
sort_i          : Elapsed execution time: 0.003152 sec
```



In both test scenario actually the without inline is faster.

Let's see cachegrind output:

Without inline:

```
==11307== 
==11307== I   refs:      252,560,814
==11307== I1  misses:          1,438
==11307== LLi misses:          1,379
==11307== I1  miss rate:        0.00%
==11307== LLi miss rate:        0.00%
==11307== 
==11307== D   refs:       99,812,738  (60,203,695 rd   + 39,609,043 wr)
==11307== D1  misses:        317,712  (   175,988 rd   +    141,724 wr)
==11307== LLd misses:          4,726  (     2,190 rd   +      2,536 wr)
==11307== D1  miss rate:         0.3% (       0.3%     +        0.4%  )
==11307== LLd miss rate:         0.0% (       0.0%     +        0.0%  )
==11307== 
==11307== LL refs:           319,150  (   177,426 rd   +    141,724 wr)
==11307== LL misses:           6,105  (     3,569 rd   +      2,536 wr)
==11307== LL miss rate:          0.0% (       0.0%     +        0.0%  )
==11307== 
==11307== Branches:       39,834,092  (38,133,441 cond +  1,700,651 ind)
==11307== Mispredicts:     2,737,089  ( 2,736,823 cond +        266 ind)
==11307== Mispred rate:          6.9% (       7.2%     +        0.0%   )
```

With inline:

```
Done testing.
==14968== 
==14968== I   refs:      253,156,142
==14968== I1  misses:          1,874
==14968== LLi misses:          1,593
==14968== I1  miss rate:        0.00%
==14968== LLi miss rate:        0.00%
==14968== 
==14968== D   refs:       99,024,675  (59,612,646 rd   + 39,412,029 wr)
==14968== D1  misses:        315,382  (   174,388 rd   +    140,994 wr)
==14968== LLd misses:          4,726  (     2,190 rd   +      2,536 wr)
==14968== D1  miss rate:         0.3% (       0.3%     +        0.4%  )
==14968== LLd miss rate:         0.0% (       0.0%     +        0.0%  )
==14968== 
==14968== LL refs:           317,256  (   176,262 rd   +    140,994 wr)
==14968== LL misses:           6,319  (     3,783 rd   +      2,536 wr)
==14968== LL miss rate:          0.0% (       0.0%     +        0.0%  )
==14968== 
==14968== Branches:       39,834,082  (38,133,431 cond +  1,700,651 ind)
==14968== Mispredicts:     2,669,307  ( 2,669,041 cond +        266 ind)
==14968== Mispred rate:          6.7% (       7.0%     +        0.0%   )
```

I am guessing that the downsides would be instruction cache misses. But we don't have instructions info from the cache grind. I guess we can use branches as a proxy for instructions.

### Write-up 4

> Give a reason why using pointers may improve performance. Report on any performance differences you observed in your implementation.

``` bash
$ ./sort 10000 10

Running test #0...
Generating random array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.001076 sec
sort_i          : Elapsed execution time: 0.001030 sec
sort_p          : Elapsed execution time: 0.000999 sec
Generating inverted array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.001756 sec
sort_i          : Elapsed execution time: 0.001599 sec
sort_p          : Elapsed execution time: 0.001524 sec

Running test #1...
 --> test_zero_element at line 245: PASS

Running test #2...
 --> test_one_element at line 266: PASS
Done testing.
```

The reason should be that pointer would require less multiplication of memory address.

### Write-up 5

> Explain what sorting algorithm you used and how you chose the number of elements to be sorted in the base case. Report on the performance differences you observed.

After trying with multiple values. Insertion sort with size 25 is decent enough in improvement. 

``` bash
$ make clean;make sort; ./sort 10000 10

rm -f ./sort *.std* *.gcov *.gcda *.gcno default.profraw
gcc main.c tests.c util.c isort.c sort_a.c sort_c.c sort_i.c sort_p.c sort_m.c sort_f.c -O3 -DNDEBUG -g -Wall -std=gnu99 -gdwarf-3  -lrt -lm  -o sort

Running test #0...
Generating random array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.001557 sec
sort_i          : Elapsed execution time: 0.001503 sec
sort_p          : Elapsed execution time: 0.001540 sec
sort_c          : Elapsed execution time: 0.000738 sec
Generating inverted array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.003452 sec
sort_i          : Elapsed execution time: 0.003337 sec
sort_p          : Elapsed execution time: 0.002430 sec
sort_c          : Elapsed execution time: 0.001878 sec

Running test #1...
 --> test_zero_element at line 245: PASS

Running test #2...
 --> test_one_element at line 266: PASS
Done testing.
```

### Write-up 6

> Explain any difference in performance in your sort_m.c. Can a compiler automatically make this optimization for you and save you all the effort? Why or why not?

``` bash
$ make clean;make sort; ./sort 10000 10
rm -f ./sort *.std* *.gcov *.gcda *.gcno default.profraw
gcc main.c tests.c util.c isort.c sort_a.c sort_c.c sort_i.c sort_p.c sort_m.c sort_f.c -O3 -DNDEBUG -g -Wall -std=gnu99 -gdwarf-3  -lrt -lm  -o sort

Running test #0...
Generating random array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.001263 sec
sort_i          : Elapsed execution time: 0.001193 sec
sort_p          : Elapsed execution time: 0.001219 sec
sort_c          : Elapsed execution time: 0.000653 sec
sort_m          : Elapsed execution time: 0.000676 sec
Generating inverted array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.001850 sec
sort_i          : Elapsed execution time: 0.001700 sec
sort_p          : Elapsed execution time: 0.001730 sec
sort_c          : Elapsed execution time: 0.000853 sec
sort_m          : Elapsed execution time: 0.000881 sec

Running test #1...
 --> test_zero_element at line 245: PASS

Running test #2...
 --> test_one_element at line 266: PASS
Done testing.
```

The time is slightly slower. Not sure what could be the reason. Maybe memory management is a bit worse. We can try to cachegrind it.

With 2 mallocs:

```
Done testing.
==14122== 
==14122== I   refs:      47,022,506
==14122== I1  misses:         1,479
==14122== LLi misses:         1,416
==14122== I1  miss rate:       0.00%
==14122== LLi miss rate:       0.00%
==14122== 
==14122== D   refs:      15,428,775  ( 8,818,610 rd   + 6,610,165 wr)
==14122== D1  misses:       176,100  (    93,953 rd   +    82,147 wr)
==14122== LLd misses:         4,724  (     2,191 rd   +     2,533 wr)
==14122== D1  miss rate:        1.1% (       1.1%     +       1.2%  )
==14122== LLd miss rate:        0.0% (       0.0%     +       0.0%  )
==14122== 
==14122== LL refs:          177,579  (    95,432 rd   +    82,147 wr)
==14122== LL misses:          6,140  (     3,607 rd   +     2,533 wr)
==14122== LL miss rate:         0.0% (       0.0%     +       0.0%  )
==14122== 
==14122== Branches:      11,627,744  (11,486,187 cond +   141,557 ind)
==14122== Mispredicts:      723,462  (   723,241 cond +       221 ind)
==14122== Mispred rate:         6.2% (       6.3%     +       0.2%   )
```

With 1 malloc:

```
==14543== 
==14543== I   refs:      45,078,388
==14543== I1  misses:         1,474
==14543== LLi misses:         1,416
==14543== I1  miss rate:       0.00%
==14543== LLi miss rate:       0.00%
==14543== 
==14543== D   refs:      14,121,307  ( 8,078,305 rd   + 6,043,002 wr)
==14543== D1  misses:       132,092  (    75,190 rd   +    56,902 wr)
==14543== LLd misses:         4,392  (     2,191 rd   +     2,201 wr)
==14543== D1  miss rate:        0.9% (       0.9%     +       0.9%  )
==14543== LLd miss rate:        0.0% (       0.0%     +       0.0%  )
==14543== 
==14543== LL refs:          133,566  (    76,664 rd   +    56,902 wr)
==14543== LL misses:          5,808  (     3,607 rd   +     2,201 wr)
==14543== LL miss rate:         0.0% (       0.0%     +       0.0%  )
==14543== 
==14543== Branches:      11,079,972  (10,958,855 cond +   121,117 ind)
==14543== Mispredicts:      713,406  (   713,185 cond +       221 ind)
==14543== Mispred rate:         6.4% (       6.5%     +       0.2%   )
```

Actually we can see that the refs and misses is lower for the one with only 1 malloc (the test checker also contributes some memory references).


### Write-up 7

> Report any differences in performance in your sort_f.c, and explain the differences using profiling data.

``` bash

$ make clean; make sort; ./sort 10000 10
rm -f ./sort *.std* *.gcov *.gcda *.gcno default.profraw
gcc main.c tests.c util.c isort.c sort_a.c sort_c.c sort_i.c sort_p.c sort_m.c sort_f.c -O3 -DNDEBUG -g -Wall -std=gnu99 -gdwarf-3  -lrt -lm  -o sort

Running test #0...
Generating random array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.001126 sec
sort_i          : Elapsed execution time: 0.001126 sec
sort_p          : Elapsed execution time: 0.000992 sec
sort_c          : Elapsed execution time: 0.000568 sec
sort_m          : Elapsed execution time: 0.000649 sec
sort_f          : Elapsed execution time: 0.000555 sec
Generating inverted array of 10000 elements
Arrays are sorted: yes
 --> test_correctness at line 217: PASS
sort_a          : Elapsed execution time: 0.001727 sec
sort_i          : Elapsed execution time: 0.001618 sec
sort_p          : Elapsed execution time: 0.001515 sec
sort_c          : Elapsed execution time: 0.000751 sec
sort_m          : Elapsed execution time: 0.000881 sec
sort_f          : Elapsed execution time: 0.000717 sec

Running test #1...
 --> test_zero_element at line 245: PASS

Running test #2...
 --> test_one_element at line 266: PASS
Done testing.
```

Time reduced by about 20%.