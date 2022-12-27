# Everybit

## Checkpoints

version | l tier | description
------- | ------ | ---------
0       | 19     | original program
1       | 38     | use copy

## Todolist

- [x] use copy
- [ ] r(r(a)r(b)) = ba 
- [ ] perform cache analysis

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