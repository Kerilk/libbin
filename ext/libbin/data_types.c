#include "ruby.h"
#include "./libbin_c.h"

VALUE cDataShape;
VALUE cScalar;
VALUE cHalf;

/* size(value, previous_offset = 0, parent = nil, index = nil, length = nil)*/
#define MAKE_TYPE_SIZE(CLASS, MAPPED_TYPE)                         \
static VALUE CLASS ## _size(int argc, VALUE* argv, VALUE self) {   \
  (void)self;                                                      \
  VALUE length;                                                    \
  rb_scan_args(argc, argv, "14", NULL, NULL, NULL, NULL, &length); \
  if (RTEST(length))                                               \
    return ULL2NUM(sizeof(MAPPED_TYPE)*NUM2ULL(length));           \
  else                                                             \
    return ULL2NUM(sizeof(MAPPED_TYPE));                           \
}

/* shape(value, previous_offset = 0, parent = nil, index = nil, kind = DataShape, length = nil) */
#define MAKE_TYPE_SHAPE(CLASS, MAPPED_TYPE)                                                \
static VALUE CLASS ## _shape(int argc, VALUE* argv, VALUE self) {                          \
  (void)self;                                                                              \
  VALUE previous_offset;                                                                   \
  VALUE kind;                                                                              \
  VALUE length;                                                                            \
  rb_scan_args(argc, argv, "15", NULL, &previous_offset, NULL, NULL, &kind, &length);      \
  if (NIL_P(previous_offset))                                                              \
    previous_offset = ULL2NUM(0);                                                          \
  if (NIL_P(kind))                                                                         \
    kind = cDataShape;                                                                     \
  if (NIL_P(length))                                                                       \
    length = ULL2NUM(1);                                                                   \
  return rb_funcall(kind, rb_intern("new"), 2, previous_offset,                            \
   LL2NUM(NUM2LL(previous_offset) - 1 + NUM2LL(length) * (ptrdiff_t)sizeof(MAPPED_TYPE))); \
}

MAKE_TYPE_SIZE(cHalf, uint16_t);
MAKE_TYPE_SHAPE(cHalf, uint16_t);

static inline float half_to_float_le(uint16_t val) {
  val = unpack_half_le(val);
  union float_u u;
  u.i = half_to_float(val);
  return u.f;
}

static inline float half_to_float_be(uint16_t val) {
  val = unpack_half_be(val);
  union float_u u;
  u.i = half_to_float(val);
  return u.f;
}

static inline uint16_t float_to_half_le(float f) {
  uint16_t res;
  union float_u u;
  u.f = f;
  res = half_from_float(u.i);
  res = pack_half_le(res);
  return res;
}

static inline uint16_t float_to_half_be(float f) {
  uint16_t res;
  union float_u u;
  u.f = f;
  res = half_from_float(u.i);
  res = pack_half_be(res);
  return res;
}

/* load(input, input_big = LibBin::default_big?, parent = nil, index = nil, length = nil) */
static VALUE cHalf_load(int argc, VALUE* argv, VALUE self)
{
  (void)self;
  VALUE input;
  VALUE input_big;
  VALUE length;
  unsigned little;
  rb_scan_args(argc, argv, "14", &input, &input_big, NULL, NULL, &length);
  if (NIL_P(input_big))
    input_big = rb_funcall(mLibBin, rb_intern("default_big?"), 0);
  little = RTEST(input_big) ? 0 : 1;

  VALUE str;
  VALUE res;
  long n = RTEST(length) ? NUM2LONG(length) : 1;
  size_t cnt = sizeof(uint16_t) * n;
  str = rb_funcall(input, rb_intern("read"), 1, ULL2NUM(cnt));
  uint16_t *data = (uint16_t *)RSTRING_PTR(str);
  if (little) {
    if (RTEST(length)) {
      res = rb_ary_new_capa(n);
      long i = 0;
      for (uint16_t *p = data; p != data + n; p++, i++)
        rb_ary_store(res, i, DBL2NUM(half_to_float_le(*p)));
    } else
      res = DBL2NUM(half_to_float_le(*data));
  } else {
    if (RTEST(length)) {
      res = rb_ary_new_capa(n);
      long i = 0;
      for (uint16_t *p = data; p != data + n; p++, i++) {
        rb_ary_store(res, i, DBL2NUM(half_to_float_be(*p)));
      }
    } else
      res = DBL2NUM(half_to_float_be(*data));
  }
  return res;
}

/* dump(value, output, output_big = LibBin::default_big?, parent = nil, index = nil, length = nil) */
static VALUE cHalf_dump(int argc, VALUE* argv, VALUE self)
{
  (void)self;
  VALUE value;
  VALUE output;
  VALUE output_big;
  VALUE length;
  unsigned little;
  rb_scan_args(argc, argv, "24", &value, &output, &output_big, NULL, NULL, &length);
  if (NIL_P(output_big))
    output_big = rb_funcall(mLibBin, rb_intern("default_big?"), 0);
  little = RTEST(output_big) ? 0 : 1;

  VALUE str;
  long n = RTEST(length) ? NUM2LONG(length) : 1;
  size_t cnt = sizeof(uint16_t) * n;
  str = rb_str_buf_new((long)cnt);
  if (little) {
    if (RTEST(length)) {
      for (long i = 0; i < n; i++) {
        uint16_t v = float_to_half_le(NUM2DBL(rb_ary_entry(value, i)));
        rb_str_cat(str, (char *)&v, sizeof(uint16_t));
      }
    } else {
      uint16_t v = float_to_half_le(NUM2DBL(value));
      rb_str_cat(str, (char *)&v, sizeof(uint16_t));
    }
  } else {
    if (RTEST(length)) {
      for (long i = 0; i < n; i++) {
        uint16_t v = float_to_half_be(NUM2DBL(rb_ary_entry(value, i)));
        rb_str_cat(str, (char *)&v, sizeof(uint16_t));
      }
    } else {
      uint16_t v = float_to_half_be(NUM2DBL(value));
      rb_str_cat(str, (char *)&v, sizeof(uint16_t));
    }
  }
  rb_funcall(output, rb_intern("write"), 1, str);
  return Qnil;
}

/* convert(input, output, input_big = LibBin::default_big?, output_big = !input_big, parent = nil, index = nil, length = nil) */
static VALUE cHalf_convert(int argc, VALUE* argv, VALUE self)
{
  (void)self;
  VALUE input;
  VALUE output;
  VALUE input_big;
  VALUE output_big;
  VALUE length;
  unsigned little_input, little_output;
  rb_scan_args(argc, argv, "25", &input, &output, &input_big, &output_big, NULL, NULL, &length);
  if (NIL_P(input_big))
    input_big = rb_funcall(mLibBin, rb_intern("default_big?"), 0);
  little_input = RTEST(input_big) ? 0 : 1;
  if (NIL_P(output_big))
    output_big = little_input ? Qtrue : Qfalse;
  little_output = RTEST(output_big) ? 0 : 1;

  VALUE str;
  VALUE res;
  long n = RTEST(length) ? NUM2LONG(length) : 1;
  size_t cnt = sizeof(uint16_t) * n;
  str = rb_funcall(input, rb_intern("read"), 1, ULL2NUM(cnt));
  uint16_t *data = (uint16_t *)RSTRING_PTR(str);
  if (little_input) {
    if (RTEST(length)) {
      res = rb_ary_new_capa(n);
      long i = 0;
      for (uint16_t *p = data; p != data + n; p++, i++) {
        rb_ary_store(res, i, DBL2NUM(half_to_float_le(*p)));
        if (!little_output)
          *p = bswap_uint16(*p);
      }
    } else {
      res = DBL2NUM(half_to_float_le(*data));
      if (!little_output)
        *data = bswap_uint16(*data);
    }
  } else {
    if (RTEST(length)) {
      res = rb_ary_new_capa(n);
      long i = 0;
      for (uint16_t *p = data; p != data + n; p++, i++) {
        rb_ary_store(res, i, DBL2NUM(half_to_float_be(*p)));
        if (little_output)
          *p = bswap_uint16(*p);
      }
    } else {
      res = DBL2NUM(half_to_float_be(*data));
      if (little_output)
        *data = bswap_uint16(*data);
    }
  }
  rb_funcall(output, rb_intern("write"), 1, str);
  return res;
}

static void define_cHalf() {
  cHalf = rb_define_class_under(cDataConverter, "Half", cScalar);
  rb_define_singleton_method(cHalf, "size", cHalf_size, -1);
  rb_define_singleton_method(cHalf, "shape", cHalf_shape, -1);
  rb_define_singleton_method(cHalf, "load", cHalf_load, -1);
  rb_define_singleton_method(cHalf, "dump", cHalf_dump, -1);
  rb_define_singleton_method(cHalf, "convert", cHalf_convert, -1);
}

void define_cScalar() {
  cScalar = rb_define_class_under(cDataConverter, "Scalar", rb_cObject);
  define_cHalf();
}


