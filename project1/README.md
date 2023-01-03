# Everybit

## Checkpoints

version | l tier | description
------- | ------ | ---------
0       | 19     | original program
1       | 38     | use copy
2.0     | 38     | use swap + reversal (use while loop)
2.1     | 38     | use swap + reversal (use for loop)


## Todolist

- [x] use copy
- [x] r(r(a)r(b)) = ba 
- [ ] batched / byte operation
- [ ] perform cache analysis
- [ ] profiling?

## Features

### Feature 1: use copy (bitarray_rotate_left_c)

In this method we will be modifying `bitarray_rotate_left` to `bitarray_rotate_left_c`.

``` c
static void bitarray_rotate_left_c(bitarray_t* const bitarray,
                                 const size_t bit_offset,
                                 const size_t bit_length,
                                 const size_t bit_left_amount) {
  bitarray_t* buffer = bitarray_new(bit_left_amount);
  for (size_t i = 0; i < bit_left_amount; i++) {
    bitarray_set(buffer, i, bitarray_get(bitarray, bit_offset+i));
  }
  for (size_t i = bit_left_amount; i < bit_length; i++) {
    bitarray_set(bitarray, bit_offset+i-bit_left_amount, bitarray_get(bitarray, bit_offset+i));
  }
  for (size_t i = bit_length - bit_left_amount; i < bit_length; i++) {
    bitarray_set(bitarray, bit_offset+i, bitarray_get(buffer, i-(bit_length - bit_left_amount)));
  }
}
```

### Feature 2: use (aRbR)R=ba

In this method, we will rotate the string by the reversal properties.

For example, if we have string 10001 and want to rotate left by 3. It will become 01100. This is actually also be identical with a swapping 100 with 01.

We don't have to do this with external memory (just need a constant memory). We only need to follow the following step:

1. Reverse each part 100 - 01 => 001 - 10
2. Reverse everything 00110 => 01100.

With initial implementation, we can get tier 38 performance

``` c
// r for reversal
static void bitarray_rotate_left_r(bitarray_t* const bitarray,
                                 const size_t bit_offset,
                                 const size_t bit_length,
                                 const size_t bit_left_amount) {
  // this is necessary to prevent overflow (-1 -> 18446744073709551615) when
  //  when calculating lt
  if (bit_left_amount <= 0) {
    return; 
  }

  bool tmp;
  size_t lh = bit_offset; // left head
  size_t lt = bit_offset+bit_left_amount-1; // left tail
  size_t rh = bit_offset+bit_left_amount; // right head
  size_t rt = bit_offset+bit_length-1; // right tail
  #ifdef DEBUG
    printf("DEBUG 0");
    bitarray_fprint(stdout, bitarray);
  #endif
  while (lh < lt) {
    tmp = bitarray_get(bitarray, lt);
    bitarray_set(bitarray, lt, bitarray_get(bitarray, lh));
    bitarray_set(bitarray, lh, tmp);
    lh++; lt--;
  }
  #ifdef DEBUG
    printf("DEBUG 1");
    bitarray_fprint(stdout, bitarray);
  #endif
  while (rh < rt) {
    tmp = bitarray_get(bitarray, rt);
    bitarray_set(bitarray, rt, bitarray_get(bitarray, rh));
    bitarray_set(bitarray, rh, tmp);
    rh++; rt--;
  }
  size_t h = bit_offset;
  size_t t = bit_offset+bit_length-1;
  while (h < t) {
    tmp = bitarray_get(bitarray, t);
    bitarray_set(bitarray, t, bitarray_get(bitarray, h));
    bitarray_set(bitarray, h, tmp);
    h++; t--;
  }
}
```

However we can still improve by avoiding the checks in the while loop by pre-calculating the number of iteration that we need.

``` c
  bool tmp;
  size_t lh = bit_offset; // left head
  size_t lt = bit_offset+bit_left_amount-1; // left tail
  size_t rh = bit_offset+bit_left_amount; // right head
  size_t rt = bit_offset+bit_length-1; // right tail
  for (int i = 0; i < (bit_left_amount >> 1); i ++) {
    tmp = bitarray_get(bitarray, lt);
    bitarray_set(bitarray, lt, bitarray_get(bitarray, lh));
    bitarray_set(bitarray, lh, tmp);
    lh++; lt--;
  }
  for (int i = 0; i < ((bit_length - bit_left_amount) >> 1); i ++) {
    tmp = bitarray_get(bitarray, rt);
    bitarray_set(bitarray, rt, bitarray_get(bitarray, rh));
    bitarray_set(bitarray, rh, tmp);
    rh++; rt--;
  }
  size_t h = bit_offset;
  size_t t = bit_offset+bit_length-1;
  for (int i = 0; i < (bit_length >> 1); i ++) {
    tmp = bitarray_get(bitarray, t);
    bitarray_set(bitarray, t, bitarray_get(bitarray, h));
    bitarray_set(bitarray, h, tmp);
    h++; t--;
  }
```

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

What if we can have a different version that operates on the bit_array rather than the bit? Specifically, we can use this for `bitarray_rotate_left_c`.

``` c
bitarray_t* bitarray_get_batched(const bit_array* const bitarray, const size_t bit_index, const size_t bit_count);
```