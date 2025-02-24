/*
 * Simple sanity test of memcpy, memmove, and memset intrinsics.
 * (fixed length buffers, variable length buffers, etc.)
 */

#include <cstdlib>
#include <cstring>
#include <stdint.h> /* cstdint requires -std=c++0x or higher */

#include "mem_intrin.h"
#include "xdefs.h"

typedef int elem_t;

/*
 * Reset buf to the sequence of bytes: n, n+1, n+2 ... length - 1
 */
static void __attribute__((noinline))
reset_buf(uint8_t *buf, uint8_t init, SizeT length) {
  SizeT i;
  SizeT v = init;
  for (i = 0; i < length; ++i)
    buf[i] = v++;
}

/* Do a fletcher-16 checksum so that the order of the values matter.
 * (Not doing a fletcher-32 checksum, since we are working with
 * smaller buffers, whose total won't approach 2**16).
 */
static int __attribute__((noinline))
fletcher_checksum(uint8_t *buf, SizeT length) {
  SizeT i;
  int sum = 0;
  int sum_of_sums = 0;
  const int kModulus = 255;
  for (i = 0; i < length; ++i) {
    sum = (sum + buf[i]) % kModulus;
    sum_of_sums = (sum_of_sums + sum) % kModulus;
  }
  return (sum_of_sums << 8) | sum;
}

int memcpy_test(uint8_t *buf, uint8_t *buf2, uint8_t init, SizeT length) {
  reset_buf(buf, init, length);
  memcpy((void *)buf2, (void *)buf, length);
  return fletcher_checksum(buf2, length);
}

int memmove_test(uint8_t *buf, uint8_t *buf2, uint8_t init, SizeT length) {
  int sum1;
  int sum2;
  const int overlap_bytes = 4 * sizeof(elem_t);
  if (length <= overlap_bytes)
    return 0;
  uint8_t *overlap_buf = buf + overlap_bytes;
  SizeT reduced_length = length - overlap_bytes;
  reset_buf(buf, init, length);

  /* Test w/ overlap. */
  memmove((void *)overlap_buf, (void *)buf, reduced_length);
  sum1 = fletcher_checksum(overlap_buf, reduced_length);
  /* Test w/out overlap. */
  memmove((void *)buf2, (void *)buf, length);
  sum2 = fletcher_checksum(buf2, length);
  return sum1 + sum2;
}

int memset_test(uint8_t *buf, uint8_t *buf2, uint8_t init, SizeT length) {
  memset((void *)buf, init, length);
  memset((void *)buf2, init + 4, length);
  return fletcher_checksum(buf, length) + fletcher_checksum(buf2, length);
}

#define X(NBYTES)                                                              \
  int memcpy_test_fixed_len_##NBYTES(uint8_t init) {                           \
    uint8_t buf[NBYTES];                                                       \
    uint8_t buf2[NBYTES];                                                      \
    reset_buf(buf, init, NBYTES);                                              \
    memcpy((void *)buf2, (void *)buf, NBYTES);                                 \
    return fletcher_checksum(buf2, NBYTES);                                    \
  }                                                                            \
                                                                               \
  int memmove_test_fixed_len_##NBYTES(uint8_t init) {                          \
    uint8_t buf[NBYTES + 16];                                                  \
    uint8_t buf2[NBYTES + 16];                                                 \
    reset_buf(buf, init, NBYTES + 16);                                         \
    reset_buf(buf2, init, NBYTES + 16);                                        \
    /* Move up */                                                              \
    memmove((void *)(buf + 16), (void *)buf, NBYTES);                          \
    /* Move down */                                                            \
    memmove((void *)buf2, (void *)(buf2 + 16), NBYTES);                        \
    return fletcher_checksum(buf, NBYTES + 16) +                               \
           fletcher_checksum(buf2, NBYTES + 16);                               \
  }                                                                            \
                                                                               \
  int memset_test_fixed_len_##NBYTES(uint8_t init) {                           \
    uint8_t buf[NBYTES];                                                       \
    memset((void *)buf, init, NBYTES);                                         \
    return fletcher_checksum(buf, NBYTES);                                     \
  }
MEMINTRIN_SIZE_TABLE
#undef X
