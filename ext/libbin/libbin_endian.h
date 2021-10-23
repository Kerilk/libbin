#ifndef LIBBIN_ENDIAN_H__
#define LIBBIN_ENDIAN_H__

#include <stdint.h>

static inline void little_big_swap(void *addr, size_t sz) {
  char *p = (char *)addr;
  for (size_t i = 0, j = sz - 1; i < (sz >> 1); i++, j--) {
    char tmp = p[i];
    p[i] = p[j];
    p[j] = tmp;
  }
}
#define LITTLE_BIG_SWAP(val) little_big_swap(&(val), sizeof(val))

static inline unsigned is_little_endian(void)
{
  const union { unsigned u; unsigned char c[4]; } one = { 1 };
  return one.c[0];
}

/* https://stackoverflow.com/a/2182184 */
static inline uint8_t bswap_uint8(uint8_t x) {
  return x;
}

static inline uint16_t bswap_uint16(uint16_t x) {
  return ((( x  & 0xff00u ) >> 8 ) |
          (( x  & 0x00ffu ) << 8 ));
}

static inline uint32_t bswap_uint32(uint32_t x) {
  return ((( x & 0xff000000u ) >> 24 ) |
          (( x & 0x00ff0000u ) >> 8  ) |
          (( x & 0x0000ff00u ) << 8  ) |
          (( x & 0x000000ffu ) << 24 ));
}

static inline uint64_t bswap_uint64(uint64_t x) {
  return ((( x & 0xff00000000000000ull ) >> 56 ) |
          (( x & 0x00ff000000000000ull ) >> 40 ) |
          (( x & 0x0000ff0000000000ull ) >> 24 ) |
          (( x & 0x000000ff00000000ull ) >> 8  ) |
          (( x & 0x00000000ff000000ull ) << 8  ) |
          (( x & 0x0000000000ff0000ull ) << 24 ) |
          (( x & 0x000000000000ff00ull ) << 40 ) |
          (( x & 0x00000000000000ffull ) << 56 ));
}



#define SWAP_CONVERT(TYPE, MAPPED_NAME, MAPPED_TYPE) \
do {                                                 \
  union { TYPE t; MAPPED_TYPE m; } v = { .m = x };   \
  v.m = bswap_ ## MAPPED_NAME(v.m);                  \
  return v.t;                                        \
} while (0)

#define CONVERT(TYPE, MAPPED_TYPE)                 \
do {                                               \
  union { TYPE t; MAPPED_TYPE m; } v = { .m = x }; \
  return v.t;                                      \
} while (0)

#define CONVERT_SWAP(TYPE, MAPPED_NAME, MAPPED_TYPE) \
do {                                                 \
  union { TYPE t; MAPPED_TYPE m; } v = { .t = x };   \
  v.m = bswap_ ## MAPPED_NAME(v.m);                  \
  return v.m;                                         \
} while(0)

#define UNPACKER_BE(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)  \
static inline TYPE unpack_ ## NAME ## _be(MAPPED_TYPE x) { \
  if (is_little_endian())                                  \
    SWAP_CONVERT(TYPE, MAPPED_NAME, MAPPED_TYPE);          \
  else                                                     \
    CONVERT(TYPE, MAPPED_TYPE);                            \
}

#define UNPACKER_LE(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)  \
static inline TYPE unpack_ ## NAME ## _le(MAPPED_TYPE x) { \
  if (is_little_endian())                                  \
    CONVERT(TYPE, MAPPED_TYPE);                            \
  else                                                     \
    SWAP_CONVERT(TYPE, MAPPED_NAME, MAPPED_TYPE);          \
}

#define PACKER_BE(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)  \
static inline MAPPED_TYPE pack_ ## NAME ## _be(TYPE x) { \
  if (is_little_endian())                                \
    CONVERT_SWAP(TYPE, MAPPED_NAME, MAPPED_TYPE);        \
  else                                                   \
    CONVERT(MAPPED_TYPE, TYPE);                          \
}

#define PACKER_LE(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)  \
static inline MAPPED_TYPE pack_ ## NAME ## _le(TYPE x) { \
  if (is_little_endian())                                \
    CONVERT(MAPPED_TYPE, TYPE);                          \
  else                                                   \
    CONVERT_SWAP(TYPE, MAPPED_NAME, MAPPED_TYPE);        \
}

#define CONVERTER_TYPE(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE) \
  UNPACKER_BE(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)          \
  UNPACKER_LE(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)          \
  PACKER_BE(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)            \
  PACKER_LE(NAME, TYPE, MAPPED_NAME, MAPPED_TYPE)

#define CONVERTER(NAME, TYPE, SIZE)                            \
  CONVERTER_TYPE(NAME, TYPE, uint ## SIZE, uint ## SIZE ## _t)

CONVERTER(uint8, uint8_t, 8)
CONVERTER(int8, int8_t, 8)
CONVERTER(uint16, uint16_t, 16)
CONVERTER(int16, int16_t, 16)
CONVERTER(uint32, uint32_t, 32)
CONVERTER(int32, int32_t, 32)
CONVERTER(uint64, uint64_t, 64)
CONVERTER(int64, int64_t, 64)
CONVERTER(float, float, 32)
CONVERTER(double, double, 64)
CONVERTER(half, uint16_t, 16)
CONVERTER(pghalf, uint16_t, 16)

#endif
