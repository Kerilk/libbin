#include "ruby.h"
#include "./half.h"
#include "./pghalf.h"

static VALUE mLibBin;

union float_u {
  float f;
  uint32_t i;
};

static VALUE pghalf_from_string_p(VALUE self, VALUE str, VALUE pack_str) {
  Check_Type(str, T_STRING);
  Check_Type(pack_str, T_STRING);
  VALUE arr = rb_funcall(str, rb_intern("unpack"), 1, pack_str);
  uint16_t val = NUM2USHORT(rb_funcall(arr, rb_intern("first"), 0));
  union float_u res;

  res.i = pghalf_to_float(val);
  return DBL2NUM(res.f);
}

static VALUE half_from_string_p(VALUE self, VALUE str, VALUE pack_str) {
  Check_Type(str, T_STRING);
  Check_Type(pack_str, T_STRING);
  VALUE arr = rb_funcall(str, rb_intern("unpack"), 1, pack_str);
  uint16_t val = NUM2USHORT(rb_funcall(arr, rb_intern("first"), 0));
  union float_u res;

  res.i = half_to_float(val);
  return DBL2NUM(res.f);
}

static VALUE pghalf_to_string_p(VALUE self, VALUE number, VALUE pack_str) {
  Check_Type(number, T_FLOAT);
  union float_u val;
  uint16_t res;

  val.f = NUM2DBL(number);
  res = pghalf_from_float(val.i);
  VALUE arr = rb_ary_new3(1, UINT2NUM(res) );

  return rb_funcall(arr, rb_intern("pack"), 1, pack_str);
}

static VALUE half_to_string_p(VALUE self, VALUE number, VALUE pack_str) {
  Check_Type(number, T_FLOAT);
  union float_u val;
  uint16_t res;

  val.f = NUM2DBL(number);
  res = half_from_float(val.i);
  VALUE arr = rb_ary_new3(1, UINT2NUM(res) );

  return rb_funcall(arr, rb_intern("pack"), 1, pack_str);
}
}

void Init_libbin_c() {
  mLibBin = rb_define_module("LibBin");
  rb_define_module_function(mLibBin, "half_from_string", half_from_string_p, 2);
  rb_define_module_function(mLibBin, "half_to_string", half_to_string_p, 2);
  rb_define_module_function(mLibBin, "pghalf_from_string", pghalf_from_string_p, 2);
  rb_define_module_function(mLibBin, "pghalf_to_string", pghalf_to_string_p, 2);
}
