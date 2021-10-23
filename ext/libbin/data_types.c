#include "ruby.h"
#include "./libbin_c.h"

VALUE cDataShape;
VALUE cScalar;
VALUE cHalf;
VALUE cPGHalf;

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

#define MAKE_LOAD_LOOP(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT) \
do {                                                              \
  if (RTEST(length)) {                                            \
    res = rb_ary_new_capa(n);                                     \
    long i = 0;                                                   \
    for (MAPPED_TYPE *p = data; p != data + n; p++, i++)          \
       rb_ary_store(res, i, RUBY_CONVERT(NATIVE_CONVERT(*p)));    \
  } else                                                          \
    res = RUBY_CONVERT(NATIVE_CONVERT(*data));                    \
} while (0)

/* load(input, input_big = LibBin::default_big?, parent = nil, index = nil, length = nil) */
#define MAKE_TYPE_LOAD(CLASS, MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT)   \
static VALUE CLASS ## _load(int argc, VALUE* argv, VALUE self) {           \
  (void)self;                                                              \
  VALUE input;                                                             \
  VALUE input_big;                                                         \
  VALUE length;                                                            \
  rb_scan_args(argc, argv, "14", &input, &input_big, NULL, NULL, &length); \
  if (NIL_P(input_big))                                                    \
    input_big = rb_funcall(mLibBin, rb_intern("default_big?"), 0);         \
  unsigned little = RTEST(input_big) ? 0 : 1;                              \
  long n = RTEST(length) ? NUM2LONG(length) : 1;                           \
  size_t cnt = sizeof(MAPPED_TYPE) * n;                                    \
  VALUE res;                                                               \
  VALUE str = rb_funcall(input, rb_intern("read"), 1, ULL2NUM(cnt));       \
  MAPPED_TYPE *data = (MAPPED_TYPE *)RSTRING_PTR(str);                     \
  if (little)                                                              \
    MAKE_LOAD_LOOP(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT ## _ ## le ); \
  else                                                                     \
    MAKE_LOAD_LOOP(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT ## _ ## be ); \
  return res;                                                              \
}

#define MAKE_DUMP_LOOP(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT)           \
do {                                                                        \
  if (RTEST(length))                                                        \
    for (long i = 0; i < n; i++) {                                          \
      MAPPED_TYPE v = NATIVE_CONVERT(RUBY_CONVERT(rb_ary_entry(value, i))); \
      rb_str_cat(str, (char *)&v, sizeof(MAPPED_TYPE));                     \
    }                                                                       \
  else {                                                                    \
    MAPPED_TYPE v = NATIVE_CONVERT(RUBY_CONVERT(value));                    \
    rb_str_cat(str, (char *)&v, sizeof(MAPPED_TYPE));                       \
  }                                                                         \
} while (0)

/* dump(value, output, output_big = LibBin::default_big?, parent = nil, index = nil, length = nil) */
#define MAKE_TYPE_DUMP(CLASS, MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT)             \
static VALUE CLASS ## _dump(int argc, VALUE* argv, VALUE self) {                     \
  (void)self;                                                                        \
  VALUE value;                                                                       \
  VALUE output;                                                                      \
  VALUE output_big;                                                                  \
  VALUE length;                                                                      \
  rb_scan_args(argc, argv, "24", &value, &output, &output_big, NULL, NULL, &length); \
  if (NIL_P(output_big))                                                             \
    output_big = rb_funcall(mLibBin, rb_intern("default_big?"), 0);                  \
  unsigned little = RTEST(output_big) ? 0 : 1;                                       \
  long n = RTEST(length) ? NUM2LONG(length) : 1;                                     \
  size_t cnt = sizeof(MAPPED_TYPE) * n;                                              \
  VALUE str = rb_str_buf_new((long)cnt);                                             \
  if (little)                                                                        \
    MAKE_DUMP_LOOP(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT ## _ ## le );           \
  else                                                                               \
    MAKE_DUMP_LOOP(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT ## _ ## be );           \
  rb_funcall(output, rb_intern("write"), 1, str);                                    \
  return Qnil;                                                                       \
}

#define MAKE_CONVERT_LOOP(MAPPED_TYPE, MAPPED_SWAP, SWAP_COND, RUBY_CONVERT, NATIVE_CONVERT) \
do {                                                                                         \
  if (RTEST(length)) {                                                                       \
    res = rb_ary_new_capa(n);                                                                \
    long i = 0;                                                                              \
    for (MAPPED_TYPE *p = data; p != data + n; p++, i++) {                                   \
      rb_ary_store(res, i, RUBY_CONVERT(NATIVE_CONVERT(*p)));                                \
      if (SWAP_COND)                                                                         \
        *p = MAPPED_SWAP(*p);                                                                \
    }                                                                                        \
  } else {                                                                                   \
    res = RUBY_CONVERT(NATIVE_CONVERT(*data));                                               \
    if (SWAP_COND)                                                                           \
      *data = MAPPED_SWAP(*data);                                                            \
  }                                                                                          \
} while(0)

/* convert(input, output, input_big = LibBin::default_big?, output_big = !input_big, parent = nil, index = nil, length = nil) */
#define MAKE_TYPE_CONVERT(CLASS, MAPPED_TYPE, MAPPED_SWAP, RUBY_CONVERT, NATIVE_CONVERT)                   \
static VALUE CLASS ## _convert(int argc, VALUE* argv, VALUE self) {                                        \
  (void)self;                                                                                              \
  VALUE input;                                                                                             \
  VALUE output;                                                                                            \
  VALUE input_big;                                                                                         \
  VALUE output_big;                                                                                        \
  VALUE length;                                                                                            \
  rb_scan_args(argc, argv, "25", &input, &output, &input_big, &output_big, NULL, NULL, &length);           \
  if (NIL_P(input_big))                                                                                    \
    input_big = rb_funcall(mLibBin, rb_intern("default_big?"), 0);                                         \
  unsigned little_input = RTEST(input_big) ? 0 : 1;                                                        \
  if (NIL_P(output_big))                                                                                   \
    output_big = little_input ? Qtrue : Qfalse;                                                            \
  unsigned little_output = RTEST(output_big) ? 0 : 1;                                                      \
  long n = RTEST(length) ? NUM2LONG(length) : 1;                                                           \
  size_t cnt = sizeof(MAPPED_TYPE) * n;                                                                    \
  VALUE res;                                                                                               \
  VALUE str = rb_funcall(input, rb_intern("read"), 1, ULL2NUM(cnt));                                       \
  MAPPED_TYPE *data = (MAPPED_TYPE *)RSTRING_PTR(str);                                                     \
  if (little_input)                                                                                        \
    MAKE_CONVERT_LOOP(MAPPED_TYPE, MAPPED_SWAP, !little_output, RUBY_CONVERT, NATIVE_CONVERT ## _ ## le ); \
  else                                                                                                     \
    MAKE_CONVERT_LOOP(MAPPED_TYPE, MAPPED_SWAP, little_output, RUBY_CONVERT, NATIVE_CONVERT ## _ ## be );  \
  rb_funcall(output, rb_intern("write"), 1, str);                                                          \
  return res;                                                                                              \
}

#define MAKE_CLASS_TYPE(CLASS_NAME, CLASS, MAPPED_TYPE, MAPPED_SWAP, RUBY_CONVERT_TO, RUBY_CONVERT_FROM, NATIVE_CONVERT_TO, NATIVE_CONVERT_FROM) \
  MAKE_TYPE_SIZE(CLASS, MAPPED_TYPE)                                                     \
  MAKE_TYPE_SHAPE(CLASS, MAPPED_TYPE)                                                    \
  MAKE_TYPE_LOAD(CLASS, MAPPED_TYPE, RUBY_CONVERT_TO, NATIVE_CONVERT_TO)                 \
  MAKE_TYPE_DUMP(CLASS, MAPPED_TYPE, RUBY_CONVERT_FROM, NATIVE_CONVERT_FROM)             \
  MAKE_TYPE_CONVERT(CLASS, MAPPED_TYPE, MAPPED_SWAP, RUBY_CONVERT_TO, NATIVE_CONVERT_TO) \
                                                                                         \
static void define_ ## CLASS() {                                                         \
  CLASS = rb_define_class_under(cDataConverter, #CLASS_NAME, cScalar);                   \
  rb_define_singleton_method(CLASS, "size", CLASS ## _size, -1);                         \
  rb_define_singleton_method(CLASS, "shape", CLASS ## _shape, -1);                       \
  rb_define_singleton_method(CLASS, "load", CLASS ## _load, -1);                         \
  rb_define_singleton_method(CLASS, "dump", CLASS ## _dump, -1);                         \
  rb_define_singleton_method(CLASS, "convert", CLASS ## _convert, -1);                   \
}



#define MAKE_CLASS(CLASS_NAME, SIZE, RUBY_CONVERT_TO, RUBY_CONVERT_FROM, NATIVE_CONVERT_TO, NATIVE_CONVERT_FROM) \
  MAKE_CLASS_TYPE(CLASS_NAME, c ## CLASS_NAME, uint ## SIZE ## _t, bswap_uint ## SIZE, RUBY_CONVERT_TO, RUBY_CONVERT_FROM, NATIVE_CONVERT_TO, NATIVE_CONVERT_FROM)

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

MAKE_CLASS(Half, 16, DBL2NUM, NUM2DBL, half_to_float, float_to_half)

static inline float pghalf_to_float_le(uint16_t val) {
  val = unpack_pghalf_le(val);
  union float_u u;
  u.i = pghalf_to_float(val);
  return u.f;
}

static inline float pghalf_to_float_be(uint16_t val) {
  val = unpack_pghalf_be(val);
  union float_u u;
  u.i = pghalf_to_float(val);
  return u.f;
}

static inline uint16_t float_to_pghalf_le(float f) {
  uint16_t res;
  union float_u u;
  u.f = f;
  res = pghalf_from_float(u.i);
  res = pack_pghalf_le(res);
  return res;
}

static inline uint16_t float_to_pghalf_be(float f) {
  uint16_t res;
  union float_u u;
  u.f = f;
  res = pghalf_from_float(u.i);
  res = pack_pghalf_be(res);
  return res;
}

MAKE_CLASS(PGHalf, 16, DBL2NUM, NUM2DBL, pghalf_to_float, float_to_pghalf)

void define_cScalar() {
  cScalar = rb_define_class_under(cDataConverter, "Scalar", rb_cObject);
  define_cHalf();
  define_cPGHalf();
}


