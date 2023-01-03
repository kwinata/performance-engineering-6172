# Everybit

## Checkpoints

version | l tier | description
------- | ------ | ---------
0       | 19     | original program
1       | 38     | use copy
2.0     | 38     | use swap + reversal (use while loop)
2.1     | 38     | use swap + reversal (use for loop)
3       | 40     | use batched copy


## Todolist

- [x] use copy
- [x] r(r(a)r(b)) = ba 
- [x] batched / byte operation
- [x] perf (profiling)
- [x] cache analysis

## Features

### Feature 1: use copy (bitarray_rotate_left_c)

In this method we will be modifying `bitarray_rotate_left` to `bitarray_rotate_left_c`.


### Feature 2: use (aRbR)R=ba

In this method, we will rotate the string by the reversal properties.

For example, if we have string 10001 and want to rotate left by 3. It will become 01100. This is actually also be identical with a swapping 100 with 01.

We don't have to do this with external memory (just need a constant memory). We only need to follow the following step:

1. Reverse each part 100 - 01 => 001 - 10
2. Reverse everything 00110 => 01100.

With initial implementation, we can get tier 38 performance.

However we can still improve by avoiding the checks in the while loop by pre-calculating the number of iteration that we need.

This turns out doens't improve that much. The while loop version on tier 38 runs 0.819516s meanwhile the for loop version runs 0.809408s. Which could be attributed to random variation.

### Feature 3: batched / byte operation.

So currently `bitarray_get` is getting the byte then using bitmask to get the bit:

``` c
bool bitarray_get(const bitarray_t* const bitarray, const size_t bit_index) {
  assert(bit_index < bitarray->bit_sz);
  return (bitarray->buf[bit_index / 8] & bitmask(bit_index)) ?
         true : false;
}
```

Here `buf` is type `char *`:

``` c
// Concrete data type representing an array of bits.
struct bitarray {
  size_t bit_sz;
  char* buf;
};
```

What if we can have a different version that operates on the bitarray_t rather than the bit? 
Specifically, we can apply this for the copy strategy.

After implementing. We found out that the improvement that we get is about 2x improvement. Allowing us to increase to tier 40 rather than 38.

## Exploration / tuning

The following exploration will be primarily done on the copy_batched method, unless otherwise stated.

### Exploration 1: `perf`

We will be doing the following:

```
make clean
make DEBUG=1
sudo perf record ./everybit -l
sudo perf report
```

Here's the output:
``` bash
  50.36%  everybit  everybit           [.] bitarray_copy_batched
  30.57%  everybit  everybit           [.] bitmask_until
  12.00%  everybit  everybit           [.] min_size_t
   4.62%  everybit  libc-2.31.so       [.] __vfscanf_internal
   1.10%  everybit  everybit           [.] bitarray_randfill
   0.62%  everybit  [kernel.kallsyms]  [k] clear_page_erms
   0.11%  everybit  [kernel.kallsyms]  [k] __do_page_fault
```

As we can see, bitmask_until is taking a lot of performance. Maybe we should inline it.

However, even after inlining, the function is still separate. And this is confirmed when I checked the assembly.

Let's try to convert the functino to be a macro instead. And now it's gone:

``` c
#define BITMASK(bit_index) (1 << (bit_index & 0b111))
#define BITMASK_UNTIL(count) ((1 << (count)) - 1)
#define NEGMOD8(v) (8 - (v & 0b111))
```

```
  71.52%  everybit  everybit           [.] bitarray_copy_batched
  18.37%  everybit  everybit           [.] min_size_t
   6.28%  everybit  libc-2.31.so       [.] __vfscanf_internal
```

Although, the performance is not yet improved. Let's try to also make the `min_size_t` to be macro:

```
  86.12%  everybit  everybit           [.] bitarray_copy_batched
   9.10%  everybit  libc-2.31.so       [.] __vfscanf_internal
   2.30%  everybit  everybit           [.] bitarray_randfill
   1.07%  everybit  [kernel.kallsyms]  [k] clear_page_erms
  ```

It might have slight improvement on the 40th tier (it was around `0.9s` and now `0.8s`), but it's still not enough to increase a tier.

### Exploration 2: cache analysis

Cachegrind output:

``` 
==4420== Cachegrind, a cache and branch-prediction profiler
==4420== Copyright (C) 2002-2017, and GNU GPL'd, by Nicholas Nethercote et al.
==4420== Using Valgrind-3.15.0 and LibVEX; rerun with -h for copyright info
==4420== Command: ./everybit -l
==4420== 
--4420-- warning: L3 cache found, using its data for the LL simulation.
--4420-- warning: specified LL cache: line_size 64  assoc 11  total_size 37,486,592
--4420-- warning: simulated LL cache: line_size 64  assoc 18  total_size 37,748,736
==4420== 
==4420== I   refs:      996,241,075
==4420== I1  misses:          2,328
==4420== LLi misses:          1,525
==4420== I1  miss rate:        0.00%
==4420== LLi miss rate:        0.00%
==4420== 
==4420== D   refs:      515,936,743  (383,409,992 rd   + 132,526,751 wr)
==4420== D1  misses:        226,514  (    131,397 rd   +      95,117 wr)
==4420== LLd misses:         39,349  (      5,226 rd   +      34,123 wr)
==4420== D1  miss rate:         0.0% (        0.0%     +         0.1%  )
==4420== LLd miss rate:         0.0% (        0.0%     +         0.0%  )
==4420== 
==4420== LL refs:           228,842  (    133,725 rd   +      95,117 wr)
==4420== LL misses:          40,874  (      6,751 rd   +      34,123 wr)
==4420== LL miss rate:          0.0% (        0.0%     +         0.0%  )
==4420== 
==4420== Branches:       37,583,503  ( 36,359,606 cond +   1,223,897 ind)
==4420== Mispredicts:        88,603  (     88,013 cond +         590 ind)
==4420== Mispred rate:          0.2% (        0.2%     +         0.0%   )
```