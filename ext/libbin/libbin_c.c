#include "ruby.h"
#include "./half.h"
#include "./pghalf.h"
#include "./libbin_endian.h"

static VALUE mLibBin;
static VALUE cField;

struct cField_data {
  VALUE name;
  VALUE type;
  VALUE length;
  VALUE count;
  VALUE offset;
  VALUE sequence;
  VALUE condition;
  VALUE relative_offset;
};

static void cField_mark(void* data) {
  void *start = data;
  void *end = (char *)data + sizeof(struct cField_data);
  rb_gc_mark_locations((VALUE *)start, (VALUE *)end);
}

static size_t cField_size(const void* data) {
  return sizeof(struct cField_data);
}

static const rb_data_type_t cField_type = {
  .wrap_struct_name = "cField_data",
  .function = {
    .dmark = cField_mark,
    .dfree = RUBY_DEFAULT_FREE,
    .dsize = cField_size,
  },
  .data = NULL,
  .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE cField_alloc(VALUE self) {
  struct cField_data *data;
  VALUE res = TypedData_Make_Struct(self, struct cField_data, &cField_type, data);
  return res;
}

static VALUE cField_initialize(
    VALUE self,
    VALUE name,
    VALUE type,
    VALUE length,
    VALUE count,
    VALUE offset,
    VALUE sequence,
    VALUE condition,
    VALUE relative_offset) {
  struct cField_data *data;
  TypedData_Get_Struct(self, struct cField_data, &cField_type, data);
  data->name = name;
  data->type = type;
  data->length = length;
  data->count = count;
  data->offset = offset;
  data->sequence = sequence;
  data->condition = condition;
  data->relative_offset = relative_offset;
  return self;
}

static VALUE cField_name(VALUE self) {
  struct cField_data *data;
  TypedData_Get_Struct(self, struct cField_data, &cField_type, data);
  return data->name;
}

static VALUE cField_get_type(VALUE self) {
  struct cField_data *data;
  TypedData_Get_Struct(self, struct cField_data, &cField_type, data);
  return data->type;
}

static VALUE cField_length(VALUE self) {
  struct cField_data *data;
  TypedData_Get_Struct(self, struct cField_data, &cField_type, data);
  return data->length;
}

static VALUE cField_count(VALUE self) {
  struct cField_data *data;
  TypedData_Get_Struct(self, struct cField_data, &cField_type, data);
  return data->count;
}

static VALUE cField_offset(VALUE self) {
  struct cField_data *data;
  TypedData_Get_Struct(self, struct cField_data, &cField_type, data);
  return data->offset;
}

static VALUE cField_sequence(VALUE self) {
  struct cField_data *data;
  TypedData_Get_Struct(self, struct cField_data, &cField_type, data);
  return data->sequence;
}

static VALUE cField_condition(VALUE self) {
  struct cField_data *data;
  TypedData_Get_Struct(self, struct cField_data, &cField_type, data);
  return data->condition;
}

static VALUE cField_relative_offset(VALUE self) {
  struct cField_data *data;
  TypedData_Get_Struct(self, struct cField_data, &cField_type, data);
  return data->relative_offset;
}

static void define_cField() {
  cField = rb_define_class_under(mLibBin, "Field", rb_cObject);
  rb_define_alloc_func(cField, cField_alloc);
  rb_define_method(cField, "initialize", cField_initialize, 8);
  rb_define_method(cField, "name", cField_name, 0);
  rb_define_method(cField, "type", cField_get_type, 0);
  rb_define_method(cField, "length", cField_length, 0);
  rb_define_method(cField, "count", cField_count, 0);
  rb_define_method(cField, "offset", cField_offset, 0);
  rb_define_method(cField, "sequence", cField_sequence, 0);
  rb_define_method(cField, "sequence?", cField_sequence, 0);
  rb_define_method(cField, "condition", cField_condition, 0);
  rb_define_method(cField, "relative_offset?", cField_relative_offset, 0);
}

static VALUE cDataConverter;

struct cDataConverter_data {
  VALUE __input;
  VALUE __output;
  VALUE __input_big;
  VALUE __output_big;
  VALUE __parent;
  VALUE __index;
  VALUE __position;
  VALUE __cur_position;
  // field specific data
  VALUE __offset;
  VALUE __condition;
  VALUE __type;
  VALUE __length;
  VALUE __count;
  VALUE __iterator;
};

static void cDataConverter_mark(void* data) {
  void *start = data;
  void *end = (char *)data + sizeof(struct cDataConverter_data);
  rb_gc_mark_locations((VALUE *)start, (VALUE *)end);
}

static size_t cDataConverter_size(const void* data) {
  return sizeof(struct cDataConverter_data);
}

static const rb_data_type_t cDataConverter_type = {
  .wrap_struct_name = "cDataConverter_data",
  .function = {
    .dmark = cDataConverter_mark,
    .dfree = RUBY_DEFAULT_FREE,
    .dsize = cDataConverter_size,
  },
  .data = NULL,
  .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE cDataConverter_alloc(VALUE self) {
  struct cDataConverter_data *data;
  VALUE res = TypedData_Make_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  return res;
}

static VALUE cDataConverter_initialize(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  data->__input = Qnil;
  data->__output = Qnil;
  data->__input_big = Qnil;
  data->__output_big = Qnil;
  data->__parent = Qnil;
  data->__index = Qnil;
  data->__position = Qnil;
  data->__cur_position = Qnil;
  data->__offset = Qnil;
  data->__condition = Qnil;
  data->__type = Qnil;
  data->__length = Qnil;
  data->__count = Qnil;
  data->__iterator = Qnil;
  return self;
}

static VALUE cDataConverter_parent(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  return data->__parent;
}

static VALUE cDataConverter_index(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  return data->__index;
}

static VALUE cDataConverter_iterator(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  return data->__iterator;
}

static VALUE cDataConverter_position(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  return data->__position;
}

static VALUE cDataConverter_set_convert_type(
    VALUE self,
    VALUE input,
    VALUE output,
    VALUE input_big,
    VALUE output_big,
    VALUE parent,
    VALUE index)
{
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  data->__input = input;
  data->__output = output;
  data->__input_big = input_big;
  data->__output_big = output_big;
  data->__parent = parent;
  data->__index = index;
  data->__position = rb_funcall(input, rb_intern("tell"), 0);
  data->__cur_position = data->__position;
  return Qnil;
}

static VALUE cDataConverter_unset_convert_type(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  data->__input = Qnil;
  data->__output = Qnil;
  data->__input_big = Qnil;
  data->__output_big = Qnil;
  data->__parent = Qnil;
  data->__index = Qnil;
  data->__position = Qnil;
  data->__cur_position = Qnil;
  return Qnil;
}

static VALUE cDataConverter_set_size_type(
    VALUE self,
    VALUE position,
    VALUE parent,
    VALUE index)
{
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  data->__parent = parent;
  data->__index = index;
  data->__position = position;
  data->__cur_position = data->__position;
  return Qnil;
}

static VALUE cDataConverter_unset_size_type(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  data->__parent = Qnil;
  data->__index = Qnil;
  data->__position = Qnil;
  data->__cur_position = Qnil;
  return Qnil;
}

static VALUE cDataConverter_set_load_type(
    VALUE self,
    VALUE input,
    VALUE input_big,
    VALUE parent,
    VALUE index)
{
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  data->__input = input;
  data->__input_big = input_big;
  data->__parent = parent;
  data->__index = index;
  data->__position = rb_funcall(input, rb_intern("tell"), 0);
  data->__cur_position = data->__position;
  return Qnil;
}

static VALUE cDataConverter_unset_load_type(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  data->__input = Qnil;
  data->__input_big = Qnil;
  data->__parent = Qnil;
  data->__index = Qnil;
  data->__position = Qnil;
  data->__cur_position = Qnil;
  return Qnil;
}

static VALUE cDataConverter_set_dump_type(
    VALUE self,
    VALUE output,
    VALUE output_big,
    VALUE parent,
    VALUE index)
{
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  data->__output = output;
  data->__output_big = output_big;
  data->__parent = parent;
  data->__index = index;
  data->__position = rb_funcall(output, rb_intern("tell"), 0);
  data->__cur_position = data->__position;
  return Qnil;
}

static VALUE cDataConverter_unset_dump_type(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  data->__output = Qnil;
  data->__output_big = Qnil;
  data->__parent = Qnil;
  data->__index = Qnil;
  data->__position = Qnil;
  data->__cur_position = Qnil;
  return Qnil;
}

static inline VALUE cDataConverter_decode_expression(VALUE self, VALUE expression) {
  if (T_STRING == TYPE(expression)) {
    VALUE tmp = rb_funcall(expression, rb_intern("gsub"), 2, rb_str_new_cstr(".."), rb_str_new_cstr("__parent"));
    tmp = rb_funcall(tmp, rb_intern("gsub"), 2, rb_str_new_cstr("\\"), rb_str_new_cstr("."));
    return rb_funcall(self, rb_intern("instance_eval"), 1, tmp);
  } else if (rb_obj_is_kind_of(expression, rb_cProc)) {
    return rb_funcall(expression, rb_intern("call"), 0);
  } else
    return expression;
}

static VALUE cDataConverter_decode_seek_offset(VALUE self, VALUE offset, VALUE relative_offset) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  if (!RTEST(offset))
    return Qnil;
  ptrdiff_t off = NUM2LL(cDataConverter_decode_expression(self, offset));
  if (off == 0)
    return Qfalse;
  if (RTEST(relative_offset))
    off += NUM2LL(data->__position);
  data->__cur_position = LL2NUM(off);
  if (RTEST(data->__input))
    rb_funcall(data->__input, rb_intern("seek"), 1, data->__cur_position);
  if (RTEST(data->__output))
    rb_funcall(data->__output, rb_intern("seek"), 1, data->__cur_position);
  return data->__cur_position;
}

static inline VALUE cDataConverter_decode_condition(VALUE self, VALUE condition) {
  if (!RTEST(condition))
    return Qtrue;
  return cDataConverter_decode_expression(self, condition);
}

static inline VALUE cDataConverter_decode_count(VALUE self, VALUE count) {
  if (!RTEST(count))
    return INT2NUM(1);
  return cDataConverter_decode_expression(self, count);
}

static inline VALUE cDataConverter_decode_type(VALUE self, VALUE type) {
  return cDataConverter_decode_expression(self, type);
}

static inline VALUE cDataConverter_decode_length(VALUE self, VALUE length) {
  return cDataConverter_decode_expression(self, length);
}

static VALUE cDataConverter_decode_static_conditions(VALUE self, VALUE field) {
  struct cDataConverter_data *data;
  struct cField_data *field_data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);
  data->__offset = Qnil;
  data->__condition = Qnil;
  data->__type = Qnil;
  data->__length = Qnil;
  data->__count = Qnil;
  if (!RTEST(field_data->sequence)) {
    data->__offset = cDataConverter_decode_seek_offset(self, field_data->offset, field_data->relative_offset);
    if (!data->__offset)
      rb_throw_obj(ID2SYM(rb_intern("ignored")), Qnil);
    data->__condition = cDataConverter_decode_condition(self, field_data->condition);
    if (!RTEST(data->__condition))
      rb_throw_obj(ID2SYM(rb_intern("ignored")), Qnil);
    data->__type = cDataConverter_decode_type(self, field_data->type);
    data->__length = cDataConverter_decode_length(self, field_data->length);
  }
  data->__count = cDataConverter_decode_count(self, field_data->count);
  return Qnil;
}

static VALUE cDataConverter_decode_dynamic_conditions(VALUE self, VALUE field) {
  struct cDataConverter_data *data;
  struct cField_data *field_data;
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);
  if (!RTEST(field_data->sequence))
    return Qtrue;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  data->__offset = Qnil;
  data->__condition = Qnil;
  data->__type = Qnil;
  data->__length = Qnil;
  data->__offset = cDataConverter_decode_seek_offset(self, field_data->offset, field_data->relative_offset);
  if (!data->__offset)
    return Qfalse;
  data->__condition = cDataConverter_decode_condition(self, field_data->condition);
  if (!RTEST(data->__condition))
    return Qfalse;
  data->__type = cDataConverter_decode_type(self, field_data->type);
  data->__length = cDataConverter_decode_length(self, field_data->length);
  return Qtrue;
}

static inline VALUE cDataConverter_restore_context(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  data->__iterator = Qnil;
  data->__type = Qnil;
  data->__length = Qnil;
  data->__count = Qnil;
  data->__offset = Qnil;
  data->__condition = Qnil;
  return Qnil;
}

static VALUE cDataConverter_convert_field(VALUE self, VALUE field) {
  VALUE res;
  struct cDataConverter_data *data;
  struct cField_data *field_data;
  cDataConverter_decode_static_conditions(self, field);
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);

  if (RTEST(field_data->count)) {
    long count = NUM2LONG(data->__count);
    res = rb_ary_new_capa(count);

    for (long i = 0; i < count; i++) {
      data->__iterator = LONG2NUM(i);
      if (RTEST(cDataConverter_decode_dynamic_conditions(self, field)))
        rb_ary_store(res, i, rb_funcall(data->__type, rb_intern("convert"), 7,
          data->__input,
          data->__output,
          data->__input_big,
          data->__output_big,
          self,
          data->__iterator,
          data->__length));
      else
        rb_ary_store(res, i, Qnil);
    }
  } else {
    data->__iterator = LONG2NUM(0);
    if (RTEST(cDataConverter_decode_dynamic_conditions(self, field)))
      res = rb_funcall(data->__type, rb_intern("convert"), 7,
        data->__input,
        data->__output,
        data->__input_big,
        data->__output_big,
        self,
        data->__iterator,
        data->__length);
    else
      res = Qnil;
  }

  cDataConverter_restore_context(self);
  return res;
}

static VALUE cDataConverter_load_field(VALUE self, VALUE field) {
  VALUE res;
  struct cDataConverter_data *data;
  struct cField_data *field_data;
  cDataConverter_decode_static_conditions(self, field);
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);

  if (RTEST(field_data->count)) {
    long count = NUM2LONG(data->__count);
    res = rb_ary_new_capa(count);

    for (long i = 0; i < count; i++) {
      data->__iterator = LONG2NUM(i);
      if (RTEST(cDataConverter_decode_dynamic_conditions(self, field)))
        rb_ary_store(res, i, rb_funcall(data->__type, rb_intern("load"), 5,
          data->__input,
          data->__input_big,
          self,
          data->__iterator,
          data->__length));
      else
        rb_ary_store(res, i, Qnil);
    }
  } else {
    data->__iterator = LONG2NUM(0);
    if (RTEST(cDataConverter_decode_dynamic_conditions(self, field)))
      res = rb_funcall(data->__type, rb_intern("load"), 5,
        data->__input,
        data->__input_big,
        self,
        data->__iterator,
        data->__length);
    else
      res = Qnil;
  }
  cDataConverter_restore_context(self);
  return res;
}

static VALUE cDataConverter_dump_field(VALUE self, VALUE values,  VALUE field) {
  VALUE res;
  struct cDataConverter_data *data;
  struct cField_data *field_data;
  cDataConverter_decode_static_conditions(self, field);
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);

  if (RTEST(field_data->count)) {
    long count = RARRAY_LEN(values);

    for (long i = 0; i < count; i++) {
      data->__iterator = LONG2NUM(i);
      if (RTEST(cDataConverter_decode_dynamic_conditions(self, field)))
        rb_funcall(data->__type, rb_intern("dump"), 6,
          rb_ary_entry(values, i),
          data->__output,
          data->__output_big,
          self,
          data->__iterator,
          data->__length);
    }
  } else {
    data->__iterator = LONG2NUM(0);
    if (RTEST(cDataConverter_decode_dynamic_conditions(self, field)))
      rb_funcall(data->__type, rb_intern("dump"), 6,
        values,
        data->__output,
        data->__output_big,
        self,
        data->__iterator,
        data->__length);
  }
  return cDataConverter_restore_context(self);
}

static VALUE cDataConverter_shape_field(
    VALUE self,
    VALUE values,
    VALUE kind,
    VALUE field)
{
  VALUE res;
  struct cDataConverter_data *data;
  struct cField_data *field_data;
  cDataConverter_decode_static_conditions(self, field);
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);

  if (RTEST(field_data->count)) {
    long count = RARRAY_LEN(values);
    res = rb_ary_new_capa(count);

    for (long i = 0; i < count; i++) {
      data->__iterator = LONG2NUM(i);
      if (RTEST(cDataConverter_decode_dynamic_conditions(self, field))) {
        VALUE shape = rb_funcall(data->__type, rb_intern("shape"), 6,
          rb_ary_entry(values, i),
          data->__cur_position,
          self,
          data->__iterator,
          kind,
          data->__length);
        rb_ary_store(res, i, shape);
        VALUE last = rb_funcall(shape, rb_intern("last"), 0);
        if (RTEST(last)) {
          ptrdiff_t pos = NUM2LL(last);
          if (pos >= 0)
            data->__cur_position = LL2NUM(pos + 1);
        }
      }
    }
  } else {
    data->__iterator = LONG2NUM(0);
    if (RTEST(cDataConverter_decode_dynamic_conditions(self, field))) {
      res = rb_funcall(data->__type, rb_intern("shape"), 6,
        values,
        data->__cur_position,
        self,
        data->__iterator,
        kind,
        data->__length);
      VALUE last = rb_funcall(res, rb_intern("last"), 0);
      if (RTEST(last)) {
        ptrdiff_t pos = NUM2LL(last);
        if (pos >= 0)
          data->__cur_position = LL2NUM(pos + 1);
      }
    }
  }
  cDataConverter_restore_context(self);
  return res;
}

static void define_cDataConverter() {
  cDataConverter = rb_define_class_under(mLibBin, "DataConverter", rb_cObject);
  rb_define_alloc_func(cDataConverter, cDataConverter_alloc);
  rb_define_method(cDataConverter, "initialize", cDataConverter_initialize, 0);
  rb_define_method(cDataConverter, "__parent", cDataConverter_parent, 0);
  rb_define_method(cDataConverter, "__index", cDataConverter_index, 0);
  rb_define_method(cDataConverter, "__iterator", cDataConverter_iterator, 0);
  rb_define_method(cDataConverter, "__position", cDataConverter_position, 0);

  rb_define_method(cDataConverter, "__set_convert_type", cDataConverter_set_convert_type, 6);
  rb_define_method(cDataConverter, "__unset_convert_type", cDataConverter_unset_convert_type, 0);
  rb_define_method(cDataConverter, "__set_size_type", cDataConverter_set_size_type, 3);
  rb_define_method(cDataConverter, "__unset_size_type", cDataConverter_unset_size_type, 0);
  rb_define_method(cDataConverter, "__set_load_type", cDataConverter_set_load_type, 4);
  rb_define_method(cDataConverter, "__unset_load_type", cDataConverter_unset_load_type, 0);
  rb_define_method(cDataConverter, "__set_dump_type", cDataConverter_set_dump_type, 4);
  rb_define_method(cDataConverter, "__unset_dump_type", cDataConverter_unset_dump_type, 0);

  rb_define_method(cDataConverter, "__decode_expression", cDataConverter_decode_expression, 1);
  rb_define_method(cDataConverter, "__decode_seek_offset", cDataConverter_decode_seek_offset, 2);
  rb_define_method(cDataConverter, "__decode_condition", cDataConverter_decode_condition, 1);
  rb_define_method(cDataConverter, "__decode_count", cDataConverter_decode_count, 1);
  rb_define_method(cDataConverter, "__decode_type", cDataConverter_decode_type, 1);
  rb_define_method(cDataConverter, "__decode_length", cDataConverter_decode_length, 1);
  rb_define_method(cDataConverter, "__decode_static_conditions", cDataConverter_decode_static_conditions, 1);
  rb_define_method(cDataConverter, "__decode_dynamic_conditions", cDataConverter_decode_dynamic_conditions, 1);

  rb_define_method(cDataConverter, "__restore_context", cDataConverter_restore_context, 0);

  rb_define_method(cDataConverter, "__convert_field", cDataConverter_convert_field, 1);
  rb_define_method(cDataConverter, "__load_field", cDataConverter_load_field, 1);
  rb_define_method(cDataConverter, "__dump_field", cDataConverter_dump_field, 2);
  rb_define_method(cDataConverter, "__shape_field", cDataConverter_shape_field, 3);
}

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

static VALUE cScalar;
static VALUE cHalf;
static VALUE cDataShape;

/* size(value, previous_offset = 0, parent = nil, index = nil, length = nil)*/
static VALUE cHalf_size(int argc, VALUE* argv, VALUE self)
{
  VALUE length;
  rb_scan_args(argc, argv, "14", NULL, NULL, NULL, NULL, &length);
  if (RTEST(length))
    return ULL2NUM(sizeof(uint16_t)*NUM2ULL(length));
  else
    return ULL2NUM(sizeof(uint16_t));
}

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

static inline void half_str_le(void *p, float f) {
  uint16_t res;
  union float_u u;
  u.f = f;
  res = half_from_float(u.i);
  res = pack_half_le(res);
  *(uint16_t *)p = res;
}

static inline void half_str_be(void *p, float f) {
  uint16_t res;
  union float_u u;
  u.f = f;
  res = half_from_float(u.i);
  res = pack_half_be(res);
  *(uint16_t *)p = res;
}

/* load(input, input_big = LibBin::default_big?, parent = nil, index = nil, length = nil) */
static VALUE cHalf_load(int argc, VALUE* argv, VALUE self)
{
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
  char *str_data = RSTRING_PTR(str);
  if (little) {
    if (RTEST(length)) {
      for (long i = 0; i < n; i++) {
        uint16_t v;
        half_str_le(&v, NUM2DBL(rb_ary_entry(value, i)));
        rb_str_cat(str, (char *)&v, sizeof(uint16_t));
      }
    } else {
      uint16_t v;
      half_str_le(&v, NUM2DBL(value));
      rb_str_cat(str, (char *)&v, sizeof(uint16_t));
    }
  } else {
    if (RTEST(length)) {
      for (long i = 0; i < n; i++) {
        uint16_t v;
        half_str_be(&v, NUM2DBL(rb_ary_entry(value, i)));
        rb_str_cat(str, (char *)&v, sizeof(uint16_t));
      }
    } else {
      uint16_t v;
      half_str_be(&v, NUM2DBL(value));
      rb_str_cat(str, (char *)&v, sizeof(uint16_t));
    }
  }
  rb_funcall(output, rb_intern("write"), 1, str);
  return Qnil;
}

void define_cScalar() {
  cScalar = rb_define_class_under(cDataConverter, "Scalar", rb_cObject);
  cHalf = rb_define_class_under(cDataConverter, "Half", cScalar);
  rb_define_singleton_method(cHalf, "size", cHalf_size, -1);
  rb_define_singleton_method(cHalf, "load", cHalf_load, -1);
  rb_define_singleton_method(cHalf, "dump", cHalf_dump, -1);
}

void Init_libbin_c() {
  mLibBin = rb_define_module("LibBin");
  rb_define_module_function(mLibBin, "half_from_string", half_from_string_p, 2);
  rb_define_module_function(mLibBin, "half_to_string", half_to_string_p, 2);
  rb_define_module_function(mLibBin, "pghalf_from_string", pghalf_from_string_p, 2);
  rb_define_module_function(mLibBin, "pghalf_to_string", pghalf_to_string_p, 2);
  cDataShape = rb_define_class_under(mLibBin, "DataShape", rb_cObject);
  define_cField();
  define_cDataConverter();
  define_cScalar();
}
