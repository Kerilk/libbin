#ifndef DATA_TYPES_H__
#define DATA_TYPES_H__

union float_u {
  float f;
  uint32_t i;
};

extern VALUE cScalar;
extern VALUE cDataShape;
extern VALUE cDataRange;
extern VALUE cStructure;

void define_cScalar();

#endif
