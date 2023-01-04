/**
 * Copyright (c) 2012 MIT License by 6.172 Staff
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 **/

// Implements the ADT specified in bitarray.h as a packed array of bits; a bit
// array containing bit_sz bits will consume roughly bit_sz/BUF_SZ bytes of
// memory.


#include "./bitarray.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>


#define BUF_SZ 8
#define BUF_SZ_POW 3
typedef u_int8_t buf_t;

const buf_t ONE_V = 1;
const buf_t ALL_ONE = (ONE_V << (BUF_SZ-1)) | ((ONE_V << (BUF_SZ-1)) - 1);
const buf_t MOD_BUF_SZ = BUF_SZ-1;

#define BITMASK(bit_index) (1 << (bit_index & MOD_BUF_SZ))
#define BITMASK_UNTIL(count) ((1 << (count)) - 1)
#define NEGMOD(v) (BUF_SZ - (v & MOD_BUF_SZ))
#define MIN(a, b) (a < b ? a : b)

// ********************************* Types **********************************

// Concrete data type representing an array of bits.
struct bitarray {
  // The number of bits represented by this bit array.
  // Need not be divisible by BUF_SZ.
  size_t bit_sz;

  // The underlying memory buffer that stores the bits in
  // packed form (BUF_SZ per byte).
  buf_t* buf;
};

// ******************** Prototypes for static functions *********************

// Rotates a subarray left by an arbitrary number of bits.
//
// bit_offset is the index of the start of the subarray
// bit_length is the length of the subarray, in bits
// bit_left_amount is the number of places to rotate the
//                    subarray left
//
// The subarray spans the half-open interval
// [bit_offset, bit_offset + bit_length)
// That is, the start is inclusive, but the end is exclusive.
static void bitarray_rotate_left(bitarray_t* const bitarray,
                                 const size_t bit_offset,
                                 const size_t bit_length,
                                 const size_t bit_left_amount);
static void bitarray_rotate_left_c(bitarray_t* const bitarray,
                                 const size_t bit_offset,
                                 const size_t bit_length,
                                 const size_t bit_left_amount);
static void bitarray_rotate_left_r(bitarray_t* const bitarray,
                                 const size_t bit_offset,
                                 const size_t bit_length,
                                 const size_t bit_left_amount);
static void bitarray_rotate_left_cb(bitarray_t* const bitarray,
                                 const size_t bit_offset,
                                 const size_t bit_length,
                                 const size_t bit_left_amount);

// Rotates a subarray left by one bit.
//
// bit_offset is the index of the start of the subarray
// bit_length is the length of the subarray, in bits
//
// The subarray spans the half-open interval
// [bit_offset, bit_offset + bit_length)
// That is, the start is inclusive, but the end is exclusive.
static void bitarray_rotate_left_one(bitarray_t* const bitarray,
                                     const size_t bit_offset,
                                     const size_t bit_length);

// Portable modulo operation that supports negative dividends.
//
// Many programming languages define modulo in a manner incompatible with its
// widely-accepted mathematical definition.
// http://stackoverflow.com/questions/1907565/c-python-different-behaviour-of-the-modulo-operation
// provides details; in particular, C's modulo
// operator (which the standard calls a "remainder" operator) yields a result
// signed identically to the dividend e.g., -1 % 10 yields -1.
// This is obviously unacceptable for a function which returns size_t, so we
// define our own.
//
// n is the dividend and m is the divisor
//
// Returns a positive integer r = n (mod m), in the range
// 0 <= r < m.
static size_t modulo(const ssize_t n, const size_t m);

// Produces a mask which, when ANDed with a byte, retains only the
// bit_index th byte.
//
// Example: bitmask(5) produces the byte 0b0010000.
//
// (Note that here the index is counted from right
// to left, which is different from how we represent bitarrays in the
// tests.  This function is only used by bitarray_get and bitarray_set,
// however, so as long as you always use bitarray_get and bitarray_set
// to access bits in your bitarray, this reverse representation should
// not matter.
static inline buf_t bitmask(const size_t bit_index);
static inline buf_t bitmask_until(const int count);

// Prints a string representation of a bit array.
void bitarray_fprint(FILE* const stream,
                            const bitarray_t* const bitarray);


// ******************************* Functions ********************************

bitarray_t* bitarray_new(const size_t bit_sz) {
  // Allocate an underlying buffer of ceil(bit_sz/BUF_SZ) bytes.
  buf_t* const buf = calloc(1, (bit_sz+BUF_SZ-1) / BUF_SZ);
  if (buf == NULL) {
    return NULL;
  }

  // Allocate space for the struct.
  bitarray_t* const bitarray = malloc(sizeof(struct bitarray));
  if (bitarray == NULL) {
    free(buf);
    return NULL;
  }

  bitarray->buf = buf;
  bitarray->bit_sz = bit_sz;
  return bitarray;
}

void bitarray_free(bitarray_t* const bitarray) {
  if (bitarray == NULL) {
    return;
  }
  free(bitarray->buf);
  bitarray->buf = NULL;
  free(bitarray);
}

size_t bitarray_get_bit_sz(const bitarray_t* const bitarray) {
  return bitarray->bit_sz;
}

bool bitarray_get(const bitarray_t* const bitarray, const size_t bit_index) {
  assert(bit_index < bitarray->bit_sz);

  // We're storing bits in packed form, BUF_SZ per byte.  So to get the nth
  // bit, we want to look at the (n mod BUF_SZ)th bit of the (floor(n/BUF_SZ)th)
  // byte.
  //
  // In C, integer division is floored explicitly, so we can just do it to
  // get the byte; we then bitwise-and the byte with an appropriate mask
  // to produce either a zero byte (if the bit was 0) or a nonzero byte
  // (if it wasn't).  Finally, we convert that to a boolean.
  return (bitarray->buf[bit_index / BUF_SZ] & bitmask(bit_index)) ?
         true : false;
}

void bitarray_copy_batched(const bitarray_t* const bitarray, const size_t bit_index, const size_t bit_count, const bitarray_t* destination, const size_t destination_index) {
  assert(bit_index + bit_count <= bitarray->bit_sz);
  assert(destination_index + bit_count <= destination->bit_sz );
  
  size_t loc = bit_index;
  size_t dloc = destination_index;
  while(true) {
    
    // bitmask of (loc % BUF_SZ == 6) is -> 1100_0000 (1111_1111 ^ 0011_1111)
    buf_t bitmask = (ALL_ONE ^ BITMASK_UNTIL(loc & MOD_BUF_SZ));

    // we will bit shift by (loc % BUF_SZ) to push the bits infront
    buf_t v = (bitarray->buf[loc >> BUF_SZ_POW] & bitmask) >> (loc & MOD_BUF_SZ);
    
    // dloc will point to the location on which we will put the new values
    // e.g. if we have bit_count 5, and dloc 3: means:
    // x x x _ _ 
    //       ^dloc
    size_t remaining_writeable = MIN(bit_count-(dloc-destination_index), NEGMOD(dloc));

    // (-loc % BUF_SZ + BUF_SZ), e.g. if loc = 6. we will get 2. This corresponds to
    // knowing that we only have the last 2 bits of current byte
    // _ _ _ _ _ _ x x
    // 0 1 2 3 4 5 6 7
    //             ^loc
    size_t current_byte_gotten = (BUF_SZ - (loc & MOD_BUF_SZ));

    // get min(remaining_writeable, current_byte_gotten)
    size_t towrite_count = MIN(remaining_writeable, current_byte_gotten);
    size_t toshift_amount = dloc & MOD_BUF_SZ;
    buf_t new_value = v & BITMASK_UNTIL(towrite_count);

    // reset the values to be written
    destination->buf[dloc >> BUF_SZ_POW] &= ~(BITMASK_UNTIL(toshift_amount+towrite_count) ^ BITMASK_UNTIL(toshift_amount));

    destination->buf[dloc >> BUF_SZ_POW] ^= (new_value << toshift_amount);
    dloc += towrite_count;
    loc += towrite_count;
    if (dloc >= destination_index+bit_count) {
      break;
    }
  }
  return;
}

void bitarray_set(bitarray_t* const bitarray,
                  const size_t bit_index,
                  const bool value) {
  assert(bit_index < bitarray->bit_sz);

  // We're storing bits in packed form, BUF_SZ per byte.  So to set the nth
  // bit, we want to set the (n mod BUF_SZ)th bit of the (floor(n/BUF_SZ)th) byte.
  //
  // In C, integer division is floored explicitly, so we can just do it to
  // get the byte; we then bitwise-and the byte with an appropriate mask
  // to clear out the bit we're about to set.  We bitwise-or the result
  // with a byte that has either a 1 or a 0 in the correct place.
  bitarray->buf[bit_index / BUF_SZ] =
    (bitarray->buf[bit_index / BUF_SZ] & ~bitmask(bit_index)) |
    (value ? bitmask(bit_index) : 0);
}

void bitarray_randfill(bitarray_t* const bitarray){
  int32_t *ptr = (int32_t *)bitarray->buf;
  for (int64_t i=0; i<bitarray->bit_sz/32 + 1; i++){
    ptr[i] = rand();
  }
}

void bitarray_rotate(bitarray_t* const bitarray,
                     const size_t bit_offset,
                     const size_t bit_length,
                     const ssize_t bit_right_amount) {
  assert(bit_offset + bit_length <= bitarray->bit_sz);

  if (bit_length == 0) {
    return;
  }

  // Convert a rotate left or right to a left rotate only, and eliminate
  // multiple full rotations.
  bitarray_rotate_left_cb(bitarray, bit_offset, bit_length,
                       modulo(-bit_right_amount, bit_length));
}

// original implementation
static void bitarray_rotate_left(bitarray_t* const bitarray,
                                 const size_t bit_offset,
                                 const size_t bit_length,
                                 const size_t bit_left_amount) {
  for (size_t i = 0; i < bit_left_amount; i++) {
    bitarray_rotate_left_one(bitarray, bit_offset, bit_length);
  }
}

// c for copy
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
}

// cb for copy batched
static void bitarray_rotate_left_cb(bitarray_t* const bitarray,
                                 const size_t bit_offset,
                                 const size_t bit_length,
                                 const size_t bit_left_amount) {
  assert(bit_left_amount >= 0);
  assert(bit_length <= bitarray->bit_sz);
  assert(bit_left_amount <= bit_length);

  bitarray_t* buffer = bitarray_new(bit_left_amount);
  bitarray_copy_batched(bitarray, bit_offset, bit_left_amount, buffer, 0);
  bitarray_copy_batched(bitarray, bit_offset+bit_left_amount, bit_length-bit_left_amount, bitarray, bit_offset);
  bitarray_copy_batched(buffer, 0, bit_left_amount, bitarray, bit_offset+(bit_length - bit_left_amount));
  bitarray_free(buffer);
}


static void bitarray_rotate_left_one(bitarray_t* const bitarray,
                                     const size_t bit_offset,
                                     const size_t bit_length) {
  // Grab the first bit in the range, shift everything left by one, and
  // then stick the first bit at the end.
  const bool first_bit = bitarray_get(bitarray, bit_offset);
  size_t i;
  for (i = bit_offset; i + 1 < bit_offset + bit_length; i++) {
    bitarray_set(bitarray, i, bitarray_get(bitarray, i + 1));
  }
  bitarray_set(bitarray, i, first_bit);
}

static size_t modulo(const ssize_t n, const size_t m) {
  const ssize_t signed_m = (ssize_t)m;
  assert(signed_m > 0);
  const ssize_t result = ((n % signed_m) + signed_m) % signed_m;
  assert(result >= 0);
  return (size_t)result;
}

static inline buf_t bitmask(const size_t bit_index) {
  return 1 << (bit_index & MOD_BUF_SZ);
}


void bitarray_fprint(FILE* const stream,
                            const bitarray_t* const bitarray) {
  for (size_t i = 0; i < bitarray_get_bit_sz(bitarray); i++) {
    fprintf(stream, "%d", bitarray_get(bitarray, i) ? 1 : 0);
  }
}
