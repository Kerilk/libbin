#ifndef DATA_TYPES_H__
#define DATA_TYPES_H__

union float_u {
  float f;
  uint32_t i;
};

extern VALUE cScalar;
extern VALUE cHalf;
extern VALUE cDataShape;
extern VALUE cDataConverter;

void define_cScalar();

#endif
