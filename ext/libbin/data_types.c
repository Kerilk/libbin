#include "ruby.h"
#include "./libbin_c.h"

VALUE cDataShape;
VALUE cScalar;

static ID id_read, id_write;

static VALUE cScalar_always_align(VALUE self) {
  return Qfalse;
}

/* align() */
#define MAKE_TYPE_ALIGN(CLASS, MAPPED_TYPE) \
static VALUE CLASS ## _align(VALUE self) {  \
  (void)self;                               \
  return INT2FIX(sizeof(MAPPED_TYPE));      \
}

/* size(value = nil, previous_offset = 0, parent = nil, index = nil, length = nil) */
#define MAKE_TYPE_SIZE(CLASS, MAPPED_TYPE)                         \
static VALUE CLASS ## _size(int argc, VALUE* argv, VALUE self) {   \
  (void)self;                                                      \
  VALUE length;                                                    \
  rb_scan_args(argc, argv, "05", NULL, NULL, NULL, NULL, &length); \
  if (RTEST(length))                                               \
    return ULL2NUM(sizeof(MAPPED_TYPE)*NUM2ULL(length));           \
  else                                                             \
    return INT2FIX(sizeof(MAPPED_TYPE));                           \
}

/* shape(value = nil, previous_offset = 0, parent = nil, index = nil, kind = DataShape, length = nil) */
#define MAKE_TYPE_SHAPE(CLASS, MAPPED_TYPE)                                                  \
static VALUE CLASS ## _shape(int argc, VALUE* argv, VALUE self) {                            \
  (void)self;                                                                                \
  VALUE previous_offset;                                                                     \
  VALUE kind;                                                                                \
  VALUE length;                                                                              \
  rb_scan_args(argc, argv, "06", NULL, &previous_offset, NULL, NULL, &kind, &length);        \
  if (NIL_P(previous_offset))                                                                \
    previous_offset = INT2FIX(0);                                                            \
  if (NIL_P(kind))                                                                           \
    kind = cDataShape;                                                                       \
  if (NIL_P(length))                                                                         \
    length = INT2FIX(1);                                                                     \
  VALUE args[] = {  previous_offset,                                                         \
    LL2NUM(NUM2LL(previous_offset) - 1 + NUM2LL(length) * (ptrdiff_t)sizeof(MAPPED_TYPE)) }; \
  return rb_class_new_instance(2, args, kind);                                               \
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

#define MAKE_LOAD_ENDIAN(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT)        \
do {                                                                       \
  if (little)                                                              \
    MAKE_LOAD_LOOP(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT ## _ ## le ); \
  else                                                                     \
    MAKE_LOAD_LOOP(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT ## _ ## be ); \
} while (0)

#define MAKE_LOAD(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT) \
  MAKE_LOAD_LOOP(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT)

/* load(input, input_big = LibBin::default_big?, parent = nil, index = nil, length = nil) */
#define MAKE_TYPE_LOAD(CLASS, MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT, LOAD) \
static VALUE CLASS ## _load(int argc, VALUE* argv, VALUE self) {               \
  (void)self;                                                                  \
  VALUE input;                                                                 \
  VALUE input_big;                                                             \
  VALUE length;                                                                \
  rb_scan_args(argc, argv, "14", &input, &input_big, NULL, NULL, &length);     \
  if (NIL_P(input_big))                                                        \
    input_big = rb_funcall(mLibBin, rb_intern("default_big?"), 0);             \
  unsigned little = RTEST(input_big) ? 0 : 1;                                  \
  long n = RTEST(length) ? NUM2LONG(length) : 1;                               \
  size_t cnt = sizeof(MAPPED_TYPE) * n;                                        \
  VALUE res;                                                                   \
  VALUE str = rb_funcall(input, id_read, 1, ULL2NUM(cnt));                     \
  if (NIL_P(str) || RSTRING_LEN(str) < (long)cnt)                              \
    rb_raise(rb_eRuntimeError,                                                 \
        "could not read enough data: got %ld needed %zu",                      \
        NIL_P(str) ? 0 : RSTRING_LEN(str), cnt);                               \
  MAPPED_TYPE *data = (MAPPED_TYPE *)RSTRING_PTR(str);                         \
  LOAD(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT);                             \
  RB_GC_GUARD(str);                                                            \
  return res;                                                                  \
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

#define MAKE_DUMP_ENDIAN(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT)        \
do {                                                                       \
  if (little)                                                              \
    MAKE_DUMP_LOOP(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT ## _ ## le ); \
  else                                                                     \
    MAKE_DUMP_LOOP(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT ## _ ## be ); \
} while (0)

#define MAKE_DUMP(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT) \
  MAKE_DUMP_LOOP(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT)

/* dump(value, output, output_big = LibBin::default_big?, parent = nil, index = nil, length = nil) */
#define MAKE_TYPE_DUMP(CLASS, MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT, DUMP)       \
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
  DUMP(MAPPED_TYPE, RUBY_CONVERT, NATIVE_CONVERT);                                   \
  rb_funcall(output, id_write, 1, str);                                              \
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

#define MAKE_CONVERT_ENDIAN(MAPPED_TYPE, MAPPED_SWAP, RUBY_CONVERT, NATIVE_CONVERT)                        \
do {                                                                                                       \
  if (little_input)                                                                                        \
    MAKE_CONVERT_LOOP(MAPPED_TYPE, MAPPED_SWAP, !little_output, RUBY_CONVERT, NATIVE_CONVERT ## _ ## le ); \
  else                                                                                                     \
    MAKE_CONVERT_LOOP(MAPPED_TYPE, MAPPED_SWAP, little_output, RUBY_CONVERT, NATIVE_CONVERT ## _ ## be );  \
} while (0)

#define MAKE_CONVERT(MAPPED_TYPE, MAPPED_SWAP, RUBY_CONVERT, NATIVE_CONVERT) \
  MAKE_CONVERT_LOOP(MAPPED_TYPE, MAPPED_SWAP, 0, RUBY_CONVERT, NATIVE_CONVERT)


/* convert(input, output, input_big = LibBin::default_big?, output_big = !input_big, parent = nil, index = nil, length = nil) */
#define MAKE_TYPE_CONVERT(CLASS, MAPPED_TYPE, MAPPED_SWAP, RUBY_CONVERT, NATIVE_CONVERT, CONVERT)          \
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
  VALUE str = rb_funcall(input, id_read, 1, ULL2NUM(cnt));                                                 \
  if (NIL_P(str)  || RSTRING_LEN(str) < (long)cnt)                                                         \
    rb_raise(rb_eRuntimeError, "could not read enough data: got %ld needed %zu",                           \
        NIL_P(str) ? 0 : RSTRING_LEN(str), cnt);                                                           \
  MAPPED_TYPE *data = (MAPPED_TYPE *)RSTRING_PTR(str);                                                     \
  CONVERT(MAPPED_TYPE, MAPPED_SWAP, RUBY_CONVERT, NATIVE_CONVERT);                                         \
  rb_funcall(output, id_write, 1, str);                                                                    \
  return res;                                                                                              \
}

#define MAKE_CLASS_DEFINE(CLASS_NAME, CLASS)                                  \
static void define_ ## CLASS() {                                              \
  CLASS = rb_define_class_under(cDataConverter, #CLASS_NAME, cScalar);        \
  rb_define_singleton_method(CLASS, "align", CLASS ## _align, 0);             \
  rb_define_singleton_method(CLASS, "size", CLASS ## _size, -1);              \
  rb_define_singleton_method(CLASS, "shape", CLASS ## _shape, -1);            \
  rb_define_singleton_method(CLASS, "load", CLASS ## _load, -1);              \
  rb_define_singleton_method(CLASS, "dump", CLASS ## _dump, -1);              \
  rb_define_singleton_method(CLASS, "convert", CLASS ## _convert, -1);        \
}

#define MAKE_STATIC_OBJECT(CLASS) \
static VALUE CLASS;

#define MAKE_CLASS_TYPE_ENDIAN_EX(CLASS_NAME, CLASS, MAPPED_TYPE, MAPPED_SWAP, RUBY_CONVERT_TO, RUBY_CONVERT_FROM, NATIVE_CONVERT_TO, NATIVE_CONVERT_FROM, ENDIAN) \
  MAKE_STATIC_OBJECT(CLASS)                                                                                      \
  MAKE_TYPE_ALIGN(CLASS, MAPPED_TYPE)                                                                            \
  MAKE_TYPE_SIZE(CLASS, MAPPED_TYPE)                                                                             \
  MAKE_TYPE_SHAPE(CLASS, MAPPED_TYPE)                                                                            \
  MAKE_TYPE_LOAD(CLASS, MAPPED_TYPE, RUBY_CONVERT_TO, NATIVE_CONVERT_TO, MAKE_LOAD ## ENDIAN)                    \
  MAKE_TYPE_DUMP(CLASS, MAPPED_TYPE, RUBY_CONVERT_FROM, NATIVE_CONVERT_FROM, MAKE_DUMP ## ENDIAN)                \
  MAKE_TYPE_CONVERT(CLASS, MAPPED_TYPE, MAPPED_SWAP, RUBY_CONVERT_TO, NATIVE_CONVERT_TO, MAKE_CONVERT ## ENDIAN) \
  MAKE_CLASS_DEFINE(CLASS_NAME, CLASS)

#define MAKE_CLASS_TYPE_ENDIAN(CLASS_NAME, CLASS, MAPPED_TYPE, MAPPED_SWAP, RUBY_CONVERT_TO, RUBY_CONVERT_FROM, NATIVE_CONVERT_TO, NATIVE_CONVERT_FROM, ENDIAN) \
  MAKE_CLASS_TYPE_ENDIAN_EX(CLASS_NAME, CLASS, MAPPED_TYPE, MAPPED_SWAP, RUBY_CONVERT_TO, RUBY_CONVERT_FROM, NATIVE_CONVERT_TO, NATIVE_CONVERT_FROM, ENDIAN)

#define ENDIAN_little
#define ENDIAN_big
#define ENDIAN_nil _ENDIAN
#define ENDIAN_suffix_little _le
#define ENDIAN_suffix_big _be
#define ENDIAN_suffix_nil
#define ENDIAN_SUFFIX_little _LE
#define ENDIAN_SUFFIX_big _BE
#define ENDIAN_SUFFIX_nil

#define MAKE_ENDIAN(ENDIAN) ENDIAN_ ## ENDIAN
#define MAKE_ENDIAN_suffix(ENDIAN) ENDIAN_suffix_ ## ENDIAN
#define MAKE_ENDIAN_SUFFIX(ENDIAN) ENDIAN_SUFFIX_ ## ENDIAN
#define MAKE_CONVERT_NAME_SUFFIX_EX(NAME, SUFFIX) NAME ## SUFFIX
#define MAKE_CONVERT_NAME_SUFFIX(NAME, SUFFIX) MAKE_CONVERT_NAME_SUFFIX_EX(NAME, SUFFIX)
#define MAKE_CONVERT_NAME_MAKE_SUFFIX(NAME, ENDIAN, MAKE_SUFFIX) MAKE_CONVERT_NAME_SUFFIX(NAME, MAKE_SUFFIX(ENDIAN))
#define MAKE_CLASS_NAME(CLASS_NAME, ENDIAN) MAKE_CONVERT_NAME_MAKE_SUFFIX(CLASS_NAME, ENDIAN, MAKE_ENDIAN_SUFFIX)
#define MAKE_C_CLASS_NAME(CLASS_NAME, ENDIAN) MAKE_CONVERT_NAME_MAKE_SUFFIX(c ## CLASS_NAME, ENDIAN, MAKE_ENDIAN_SUFFIX)
#define MAKE_CONVERT_NAME(NAME, ENDIAN) MAKE_CONVERT_NAME_MAKE_SUFFIX(NAME, ENDIAN, MAKE_ENDIAN_suffix)

#define MAKE_CLASS(CLASS_NAME, SIZE, RUBY_CONVERT_TO, RUBY_CONVERT_FROM, NATIVE_CONVERT_TO, NATIVE_CONVERT_FROM, ENDIAN) \
  MAKE_CLASS_TYPE_ENDIAN(MAKE_CLASS_NAME(CLASS_NAME, ENDIAN),            \
                         MAKE_C_CLASS_NAME(CLASS_NAME, ENDIAN),          \
                         uint ## SIZE ## _t,                             \
                         bswap_uint ## SIZE,                             \
                         RUBY_CONVERT_TO,                                \
                         RUBY_CONVERT_FROM,                              \
                         MAKE_CONVERT_NAME(NATIVE_CONVERT_TO, ENDIAN),   \
                         MAKE_CONVERT_NAME(NATIVE_CONVERT_FROM, ENDIAN), \
                         MAKE_ENDIAN(ENDIAN))

#define MAKE_CLASSES(CLASS_NAME, SIZE, RUBY_CONVERT_TO, RUBY_CONVERT_FROM, NATIVE_CONVERT_TO, NATIVE_CONVERT_FROM) \
  MAKE_CLASS(CLASS_NAME, SIZE, RUBY_CONVERT_TO, RUBY_CONVERT_FROM, NATIVE_CONVERT_TO, NATIVE_CONVERT_FROM, nil)    \
  MAKE_CLASS(CLASS_NAME, SIZE, RUBY_CONVERT_TO, RUBY_CONVERT_FROM, NATIVE_CONVERT_TO, NATIVE_CONVERT_FROM, little) \
  MAKE_CLASS(CLASS_NAME, SIZE, RUBY_CONVERT_TO, RUBY_CONVERT_FROM, NATIVE_CONVERT_TO, NATIVE_CONVERT_FROM, big)

#define MAKE_CALL_DEFINE(CLASS_NAME) \
do {                                 \
  define_ ## CLASS_NAME();           \
} while (0)

#define MAKE_CALL_DEFINE_EX(CLASS_NAME) \
  MAKE_CALL_DEFINE(CLASS_NAME)

#define MAKE_CALL_DEFINES(CLASS_NAME)                         \
do {                                                          \
  MAKE_CALL_DEFINE_EX(MAKE_C_CLASS_NAME(CLASS_NAME, nil));    \
  MAKE_CALL_DEFINE_EX(MAKE_C_CLASS_NAME(CLASS_NAME, little)); \
  MAKE_CALL_DEFINE_EX(MAKE_C_CLASS_NAME(CLASS_NAME, big));    \
} while (0)

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


MAKE_CLASSES(Half, 16, DBL2NUM, NUM2DBL, half_to_float, float_to_half)

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

MAKE_CLASSES(PGHalf, 16, DBL2NUM, NUM2DBL, pghalf_to_float, float_to_pghalf)

MAKE_CLASSES(Int8, 8, INT2FIX, NUM2SHORT, unpack_int8, pack_int8)
MAKE_CLASSES(UInt8, 8, USHORT2NUM, NUM2USHORT, unpack_uint8, pack_uint8)
MAKE_CLASSES(Int16, 16, INT2FIX, NUM2SHORT, unpack_int16, pack_int16)
MAKE_CLASSES(UInt16, 16, USHORT2NUM, NUM2USHORT, unpack_uint16, pack_uint16)
MAKE_CLASSES(Int32, 32, INT2NUM, NUM2INT, unpack_int32, pack_int32)
MAKE_CLASSES(UInt32, 32, UINT2NUM, NUM2UINT, unpack_uint32, pack_uint32)
MAKE_CLASSES(Int64, 64, LL2NUM, NUM2LL, unpack_int64, pack_int64)
MAKE_CLASSES(UInt64, 64, ULL2NUM, NUM2ULL, unpack_uint64, pack_uint64)
MAKE_CLASSES(Flt, 32, DBL2NUM, NUM2DBL, unpack_float, pack_float)
MAKE_CLASSES(Double, 64, DBL2NUM, NUM2DBL, unpack_double, pack_double)

static VALUE cStr;

static VALUE cStr_align(VALUE self) {
  (void)self;
  return INT2FIX(sizeof(char));
}

static VALUE cStr_size(int argc, VALUE* argv, VALUE self) {
  (void)self;
  VALUE value;
  VALUE length;
  rb_scan_args(argc, argv, "14", &value, NULL, NULL, NULL, &length);
  if (RTEST(length))
    return length;
  else
    return rb_funcall(value, rb_intern("bytesize"), 0);
}

static VALUE cStr_shape(int argc, VALUE* argv, VALUE self) {
  (void)self;
  VALUE value;
  VALUE previous_offset;
  VALUE kind;
  VALUE length;
  rb_scan_args(argc, argv, "15", &value, &previous_offset, NULL, NULL, &kind, &length);
  if (NIL_P(previous_offset))
    previous_offset = INT2FIX(0);
  if (NIL_P(kind))
    kind = cDataShape;
  if (NIL_P(length)) {
    VALUE args[] = { previous_offset, LL2NUM(NUM2LL(previous_offset) - 1 + RSTRING_LEN(value)) };
    return rb_class_new_instance(2, args, kind);
  } else {
    VALUE args[] = { previous_offset, LL2NUM(NUM2LL(previous_offset) - 1 + NUM2LL(length)) };
    return rb_class_new_instance(2, args, kind);
  }
}

static VALUE cStr_load(int argc, VALUE* argv, VALUE self) {
  (void)self;
  VALUE input;
  VALUE length;
  rb_scan_args(argc, argv, "14", &input, NULL, NULL, NULL, &length);
  if (NIL_P(length))
    return rb_funcall(input, rb_intern("readline"), 1, rb_str_new_static("", 1));
  else {
    VALUE str = rb_funcall(input, id_read, 1, length);
    if (NIL_P(str) || RSTRING_LEN(str) < NUM2LONG(length))
      rb_raise(rb_eRuntimeError,
        "could not read enough data: got %ld needed %zu", RSTRING_LEN(str), NUM2LONG(length));
    return str;
  }
}

static VALUE cStr_dump(int argc, VALUE* argv, VALUE self) {
  (void)self;
  VALUE value;
  VALUE output;
  VALUE length;
  rb_scan_args(argc, argv, "24", &value, &output, NULL, NULL, NULL, &length);
  if (NIL_P(length))
    rb_funcall(output, id_write, 1, value);
  else {
    long l = NUM2LONG(length);
    long vl = RSTRING_LEN(value);
    VALUE str;
    if (vl > l)
      str = rb_str_new(RSTRING_PTR(value), l);
    else {
      str = rb_str_buf_new(l);
      rb_str_cat(str, RSTRING_PTR(value), vl);
      for (long i = 0; i < l - vl; i++)
        rb_str_cat(str, "", 1);
    }
    rb_funcall(output, id_write, 1, str);
  }
  return Qnil;
}

static VALUE cStr_convert(int argc, VALUE* argv, VALUE self) {
  (void)self;
  VALUE input;
  VALUE output;
  VALUE length;
  rb_scan_args(argc, argv, "25", &input, &output, NULL, NULL, NULL, NULL, &length);
  VALUE str;
  if (NIL_P(length))
    str = rb_funcall(input, rb_intern("readline"), 1, rb_str_new_static("", 1));
  else {
    str = rb_funcall(input, id_read, 1, length);
    if (NIL_P(str) || RSTRING_LEN(str) < NUM2LONG(length))
      rb_raise(rb_eRuntimeError,
        "could not read enough data: got %ld needed %zu", RSTRING_LEN(str), NUM2LONG(length));
  }
  rb_funcall(output, id_write, 1, str);
  return str;
}

static void define_cStr() {
  cStr = rb_define_class_under(cDataConverter, "Str", cScalar);
  rb_define_singleton_method(cStr, "align", cStr_align, 0);
  rb_define_singleton_method(cStr, "size", cStr_size, -1);
  rb_define_singleton_method(cStr, "shape", cStr_shape, -1);
  rb_define_singleton_method(cStr, "load", cStr_load, -1);
  rb_define_singleton_method(cStr, "dump", cStr_dump, -1);
  rb_define_singleton_method(cStr, "convert", cStr_convert, -1);
}

void define_cScalar() {
  id_read = rb_intern("read");
  id_write = rb_intern("write");
  cScalar = rb_define_class_under(cDataConverter, "Scalar", rb_cObject);
  rb_define_singleton_method(cScalar, "always_align", cScalar_always_align, 0);
  MAKE_CALL_DEFINES(Half);
  MAKE_CALL_DEFINES(PGHalf);
  MAKE_CALL_DEFINES(Int8);
  MAKE_CALL_DEFINES(UInt8);
  MAKE_CALL_DEFINES(Int16);
  MAKE_CALL_DEFINES(UInt16);
  MAKE_CALL_DEFINES(Int32);
  MAKE_CALL_DEFINES(UInt32);
  MAKE_CALL_DEFINES(Int64);
  MAKE_CALL_DEFINES(UInt64);
  MAKE_CALL_DEFINES(Flt);
  MAKE_CALL_DEFINES(Double);
  define_cStr();
}


