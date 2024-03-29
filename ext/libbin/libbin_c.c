#include "ruby.h"
#include "./libbin_c.h"

VALUE cField;
VALUE mLibBin;
VALUE cStructure;

static VALUE rb_str_dot_dot;
static VALUE rb_str___parent;
static VALUE rb_str_backslash;
static VALUE rb_str_dot;

struct cField_data {
  VALUE name;
  VALUE type;
  VALUE length;
  VALUE count;
  VALUE offset;
  VALUE sequence;
  VALUE condition;
  VALUE relative_offset;
  VALUE align;
  VALUE expect;
  ID getter;
  ID setter;
};

static void cField_mark(void* data) {
  void *start = data;
  void *end = &((struct cField_data*)data)->getter;
  rb_gc_mark_locations((VALUE *)start, (VALUE *)end);
}

static size_t cField_size(const void* data) {
  (void)data;
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

static ID id_gsub;

static inline VALUE cField_preprocess_expression(VALUE self, VALUE expression) {
  if (T_STRING == TYPE(expression)) {
    VALUE proc = rb_str_new_cstr("proc {");
    rb_str_buf_append(proc, rb_funcall(rb_funcall(expression, id_gsub, 2, rb_str_dot_dot, rb_str___parent), id_gsub, 2, rb_str_backslash, rb_str_dot));
    rb_str_cat_cstr(proc, "}");
    return rb_obj_instance_eval(1, &proc, self);
  } else
    return expression;
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
    VALUE relative_offset,
    VALUE align,
    VALUE expect) {
  VALUE tmp;
  struct cField_data *data;
  TypedData_Get_Struct(self, struct cField_data, &cField_type, data);
  if (RTEST(align)) {
    size_t a = NUM2LL(align);
    if (a & (a-1))
      rb_raise(rb_eRuntimeError, "alignment is not a power of 2: %zu", a);
  }
  data->name = name;
  tmp = rb_str_dup(rb_obj_as_string(name));
  data->getter = rb_intern_str(tmp);
  data->setter = rb_intern_str(rb_str_cat(tmp, "=", 1));
  data->type = cField_preprocess_expression(self, type);
  data->length = cField_preprocess_expression(self, length);
  data->count = cField_preprocess_expression(self, count);
  data->offset = cField_preprocess_expression(self, offset);
  data->sequence = sequence;
  data->condition = cField_preprocess_expression(self, condition);
  data->relative_offset = relative_offset;
  data->align = align;
  data->expect = expect;
  return self;
}

#define FIELD_STATE_GETTER(propname) cField_get_ ## propname

#define CREATE_FIELD_STATE_GETTER(propname)                           \
static VALUE FIELD_STATE_GETTER(propname) (VALUE self) {              \
  struct cField_data *data;                                           \
  TypedData_Get_Struct(self, struct cField_data, &cField_type, data); \
  return data->propname ;                                             \
}

#define DEFINE_FIELD_STATE_GETTER(propname) \
  rb_define_method(cField, #propname, FIELD_STATE_GETTER(propname), 0)

#define DEFINE_FIELD_STATE_GETTER_BOOL(propname) \
  rb_define_method(cField, #propname "?", FIELD_STATE_GETTER(propname), 0)

CREATE_FIELD_STATE_GETTER(name)
CREATE_FIELD_STATE_GETTER(type)
CREATE_FIELD_STATE_GETTER(length)
CREATE_FIELD_STATE_GETTER(count)
CREATE_FIELD_STATE_GETTER(offset)
CREATE_FIELD_STATE_GETTER(sequence)
CREATE_FIELD_STATE_GETTER(condition)
CREATE_FIELD_STATE_GETTER(relative_offset)
CREATE_FIELD_STATE_GETTER(align)
CREATE_FIELD_STATE_GETTER(expect)

struct cStructure_data {
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
  VALUE __value;
};

static void cStructure_mark(void* data) {
  void *start = data;
  void *end = (char *)data + sizeof(struct cStructure_data);
  rb_gc_mark_locations((VALUE *)start, (VALUE *)end);
}

static size_t cStructure_size(const void* data) {
  (void)data;
  return sizeof(struct cStructure_data);
}

static const rb_data_type_t cStructure_type = {
  .wrap_struct_name = "cStructure_data",
  .function = {
    .dmark = cStructure_mark,
    .dfree = RUBY_DEFAULT_FREE,
    .dsize = cStructure_size,
  },
  .data = NULL,
  .flags = RUBY_TYPED_FREE_IMMEDIATELY
};

static VALUE cStructure_alloc(VALUE self) {
  struct cStructure_data *data;
  VALUE res = TypedData_Make_Struct(self, struct cStructure_data, &cStructure_type, data);
  return res;
}

static VALUE cStructure_initialize(VALUE self) {
  struct cStructure_data *data;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
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

#define STRUCT_STATE_GETTER(propname) cStructure_get_ ## propname
#define STRUCT_STATE_SETTER(propname) cStructure_set_ ## propname

#define CREATE_STRUCT_STATE_ACCESSORS(propname)                                       \
static VALUE STRUCT_STATE_GETTER(propname) (VALUE self) {                     \
  struct cStructure_data *data;                                               \
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data); \
  return data->__ ## propname ;                                               \
}                                                                             \
static VALUE STRUCT_STATE_SETTER(propname) (VALUE self, VALUE propname) {     \
  struct cStructure_data *data;                                               \
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data); \
  return data->__ ## propname = propname;                                     \
}

#define DEFINE_STRUCT_STATE_ACCESSORS(propname)                                       \
do {                                                                                  \
  rb_define_method(cStructure, "__" #propname, STRUCT_STATE_GETTER(propname), 0);     \
  rb_define_method(cStructure, "__" #propname "=", STRUCT_STATE_SETTER(propname), 1); \
} while(0)

CREATE_STRUCT_STATE_ACCESSORS(parent)
CREATE_STRUCT_STATE_ACCESSORS(index)
CREATE_STRUCT_STATE_ACCESSORS(position)
CREATE_STRUCT_STATE_ACCESSORS(cur_position)
CREATE_STRUCT_STATE_ACCESSORS(input)
CREATE_STRUCT_STATE_ACCESSORS(output)
CREATE_STRUCT_STATE_ACCESSORS(input_big)
CREATE_STRUCT_STATE_ACCESSORS(output_big)

CREATE_STRUCT_STATE_ACCESSORS(offset)
CREATE_STRUCT_STATE_ACCESSORS(condition)
CREATE_STRUCT_STATE_ACCESSORS(type)
CREATE_STRUCT_STATE_ACCESSORS(length)
CREATE_STRUCT_STATE_ACCESSORS(count)
CREATE_STRUCT_STATE_ACCESSORS(iterator)
CREATE_STRUCT_STATE_ACCESSORS(value)

static ID id_tell;

static ID id___set_convert_state;

static VALUE cStructure_set_convert_state(
    VALUE self,
    VALUE input,
    VALUE output,
    VALUE input_big,
    VALUE output_big,
    VALUE parent,
    VALUE index)
{
  struct cStructure_data *data;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  data->__input = input;
  data->__output = output;
  data->__input_big = input_big;
  data->__output_big = output_big;
  data->__parent = parent;
  data->__index = index;
  data->__position = rb_funcall(input, id_tell, 0);
  data->__cur_position = data->__position;
  return Qnil;
}

static ID id___unset_convert_state;

static VALUE cStructure_unset_convert_state(VALUE self) {
  struct cStructure_data *data;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
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

static ID id___set_size_state;

static VALUE cStructure_set_size_state(
    VALUE self,
    VALUE position,
    VALUE parent,
    VALUE index)
{
  struct cStructure_data *data;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  data->__parent = parent;
  data->__index = index;
  data->__position = position;
  data->__cur_position = data->__position;
  return Qnil;
}

static ID id___unset_size_state;

static VALUE cStructure_unset_size_state(VALUE self) {
  struct cStructure_data *data;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  data->__parent = Qnil;
  data->__index = Qnil;
  data->__position = Qnil;
  data->__cur_position = Qnil;
  return Qnil;
}

static ID id___set_load_state;

static VALUE cStructure_set_load_state(
    VALUE self,
    VALUE input,
    VALUE input_big,
    VALUE parent,
    VALUE index)
{
  struct cStructure_data *data;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  data->__input = input;
  data->__input_big = input_big;
  data->__parent = parent;
  data->__index = index;
  data->__position = rb_funcall(input, id_tell, 0);
  data->__cur_position = data->__position;
  return Qnil;
}

static ID id___unset_load_state;

static VALUE cStructure_unset_load_state(VALUE self) {
  struct cStructure_data *data;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  data->__input = Qnil;
  data->__input_big = Qnil;
  data->__parent = Qnil;
  data->__index = Qnil;
  data->__position = Qnil;
  data->__cur_position = Qnil;
  return Qnil;
}

static ID id___set_dump_state;

static VALUE cStructure_set_dump_state(
    VALUE self,
    VALUE output,
    VALUE output_big,
    VALUE parent,
    VALUE index)
{
  struct cStructure_data *data;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  data->__output = output;
  data->__output_big = output_big;
  data->__parent = parent;
  data->__index = index;
  data->__position = rb_funcall(output, id_tell, 0);
  data->__cur_position = data->__position;
  return Qnil;
}

static ID id___unset_dump_state;

static VALUE cStructure_unset_dump_state(VALUE self) {
  struct cStructure_data *data;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  data->__output = Qnil;
  data->__output_big = Qnil;
  data->__parent = Qnil;
  data->__index = Qnil;
  data->__position = Qnil;
  data->__cur_position = Qnil;
  return Qnil;
}

static ID id_instance_exec;

static inline VALUE cStructure_decode_expression(VALUE self, VALUE expression) {
  if (rb_obj_is_proc(expression))
    return rb_funcall_with_block(self, id_instance_exec, 0, NULL, expression);
  else
    return expression;
}

static inline VALUE cStructure_decode_expect(VALUE self, VALUE expect, VALUE val) {
  if (NIL_P(expect))
    return val;
  if (rb_obj_is_proc(expect)) {
    if (!RTEST(rb_funcall_with_block(self, id_instance_exec, 1, &val, expect)))
      rb_raise(rb_eRuntimeError, "could not validate field value: %"PRIsVALUE, val);
  } else
    if (!RTEST(rb_equal(expect, val)))
      rb_raise(rb_eRuntimeError, "could not validate field value: %"PRIsVALUE" expected: %"PRIsVALUE, val, expect);
  return val;
}

ID id_seek;

static inline VALUE cStructure_decode_seek_offset(VALUE self, VALUE offset, VALUE relative_offset, VALUE align) {
  struct cStructure_data *data;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  ptrdiff_t cur_pos;
  if (!RTEST(offset)) {
    if (!RTEST(align))
      return Qnil;
    if (RTEST(data->__input))
      cur_pos = NUM2LL(rb_funcall(data->__input, id_tell, 0));
    else if (RTEST(data->__output))
      cur_pos = NUM2LL(rb_funcall(data->__output, id_tell, 0));
    else
      cur_pos = NUM2LL(data->__cur_position);
    ptrdiff_t al = NUM2LL(align);
    ptrdiff_t pad;
    if ((pad = ((-cur_pos) & (al - 1))) == 0)
      return Qnil;
    cur_pos += pad;
  } else {
    cur_pos = NUM2LL(cStructure_decode_expression(self, offset));
    if (cur_pos == 0)
      return Qfalse;
    if (RTEST(relative_offset))
      cur_pos += NUM2LL(data->__position);
    if (RTEST(align)) {
      ptrdiff_t al = NUM2LL(align);
      cur_pos += (-cur_pos) & (al - 1);
    }
  }
  data->__cur_position = LL2NUM(cur_pos);
  if (RTEST(data->__input))
    rb_funcall(data->__input, id_seek, 1, data->__cur_position);
  if (RTEST(data->__output))
    rb_funcall(data->__output, id_seek, 1, data->__cur_position);
  return data->__cur_position;
}

static inline VALUE cStructure_decode_condition(VALUE self, VALUE condition) {
  if (!RTEST(condition))
    return Qtrue;
  return cStructure_decode_expression(self, condition);
}

static inline VALUE cStructure_decode_count(VALUE self, VALUE count) {
  if (!RTEST(count))
    return INT2FIX(1);
  return cStructure_decode_expression(self, count);
}

static inline VALUE cStructure_decode_type(VALUE self, VALUE type) {
  return cStructure_decode_expression(self, type);
}

static inline VALUE cStructure_decode_length(VALUE self, VALUE length) {
  if (NIL_P(length))
    return Qnil;
  return cStructure_decode_expression(self, length);
}

static inline VALUE cStructure_decode_static_conditions(VALUE self, VALUE field) {
  struct cStructure_data *data;
  struct cField_data *field_data;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);
  data->__offset = Qnil;
  data->__condition = Qnil;
  data->__type = Qnil;
  data->__length = Qnil;
  data->__count = Qnil;
  if (!RTEST(field_data->sequence)) {
    data->__offset = cStructure_decode_seek_offset(self, field_data->offset, field_data->relative_offset, field_data->align);
    if (!data->__offset)
      return Qnil;
    data->__condition = cStructure_decode_condition(self, field_data->condition);
    if (!RTEST(data->__condition))
      return Qnil;
    data->__type = cStructure_decode_type(self, field_data->type);
    data->__length = cStructure_decode_length(self, field_data->length);
  }
  data->__count = cStructure_decode_count(self, field_data->count);
  return Qtrue;
}

static inline VALUE cStructure_decode_dynamic_conditions(VALUE self, VALUE field) {
  struct cStructure_data *data;
  struct cField_data *field_data;
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);
  if (!RTEST(field_data->sequence))
    return Qtrue;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  data->__offset = Qnil;
  data->__condition = Qnil;
  data->__type = Qnil;
  data->__length = Qnil;
  data->__offset = cStructure_decode_seek_offset(self, field_data->offset, field_data->relative_offset, field_data->align);
  if (!data->__offset)
    return Qfalse;
  data->__condition = cStructure_decode_condition(self, field_data->condition);
  if (!RTEST(data->__condition))
    return Qfalse;
  data->__type = cStructure_decode_type(self, field_data->type);
  data->__length = cStructure_decode_length(self, field_data->length);
  return Qtrue;
}

static inline VALUE cStructure_restore_context(VALUE self) {
  struct cStructure_data *data;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  data->__iterator = Qnil;
  data->__type = Qnil;
  data->__length = Qnil;
  data->__count = Qnil;
  data->__offset = Qnil;
  data->__condition = Qnil;
  data->__value = Qnil;
  return Qnil;
}

static ID id_convert;
static ID id___convert_field;

static inline VALUE cStructure_convert_field(VALUE self, VALUE field) {
  VALUE res;
  struct cStructure_data *data;
  struct cField_data *field_data;
  if (NIL_P(cStructure_decode_static_conditions(self, field)))
    return Qnil;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);

  if (RTEST(field_data->count)) {
    long count = NUM2LONG(data->__count);
    res = rb_ary_new_capa(count);

    for (long i = 0; i < count; i++) {
      data->__iterator = LONG2NUM(i);
      if (RTEST(cStructure_decode_dynamic_conditions(self, field))) {
        data->__value = rb_funcall(data->__type, id_convert, 7,
            data->__input,
            data->__output,
            data->__input_big,
            data->__output_big,
            self,
            data->__iterator,
            data->__length);
        rb_ary_store(res, i,
          cStructure_decode_expect(self, field_data->expect, data->__value));
      } else
        rb_ary_store(res, i, Qnil);
    }
  } else {
    data->__iterator = INT2FIX(0);
    if (RTEST(cStructure_decode_dynamic_conditions(self, field))) {
      data->__value = rb_funcall(data->__type, id_convert, 7,
        data->__input,
        data->__output,
        data->__input_big,
        data->__output_big,
        self,
        data->__iterator,
        data->__length);
      res = cStructure_decode_expect(self, field_data->expect, data->__value);
    } else
      res = Qnil;
  }

  cStructure_restore_context(self);
  return res;
}

static ID id_load;
static ID id___load_field;

static inline VALUE cStructure_load_field(VALUE self, VALUE field) {
  VALUE res;
  struct cStructure_data *data;
  struct cField_data *field_data;
  if (NIL_P(cStructure_decode_static_conditions(self, field)))
    return Qnil;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);

  if (RTEST(field_data->count)) {
    long count = NUM2LONG(data->__count);
    res = rb_ary_new_capa(count);

    for (long i = 0; i < count; i++) {
      data->__iterator = LONG2NUM(i);
      if (RTEST(cStructure_decode_dynamic_conditions(self, field))) {
        data->__value = rb_funcall(data->__type, id_load, 5,
          data->__input,
          data->__input_big,
          self,
          data->__iterator,
          data->__length);
        rb_ary_store(res, i,
          cStructure_decode_expect(self, field_data->expect, data->__value));
      } else
        rb_ary_store(res, i, Qnil);
    }
  } else {
    data->__iterator = INT2FIX(0);
    if (RTEST(cStructure_decode_dynamic_conditions(self, field))) {
      data->__value = rb_funcall(data->__type, id_load, 5,
        data->__input,
        data->__input_big,
        self,
        data->__iterator,
        data->__length);
      res = cStructure_decode_expect(self, field_data->expect, data->__value);
    } else
      res = Qnil;
  }
  cStructure_restore_context(self);
  return res;
}

static ID id_dump;
static ID id___dump_field;

static inline VALUE cStructure_dump_field(VALUE self, VALUE values, VALUE field) {
  struct cStructure_data *data;
  struct cField_data *field_data;
  if (NIL_P(cStructure_decode_static_conditions(self, field)))
    return Qnil;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);

  if (RTEST(field_data->count)) {
    long count = RARRAY_LEN(values);

    for (long i = 0; i < count; i++) {
      data->__iterator = LONG2NUM(i);
      if (RTEST(cStructure_decode_dynamic_conditions(self, field))) {
        data->__value = rb_ary_entry(values, i);
        cStructure_decode_expect(self, field_data->expect, data->__value);
        rb_funcall(data->__type, id_dump, 6,
          data->__value,
          data->__output,
          data->__output_big,
          self,
          data->__iterator,
          data->__length);
      }
    }
  } else {
    data->__iterator = INT2FIX(0);
    if (RTEST(cStructure_decode_dynamic_conditions(self, field))) {
      data->__value = values;
      cStructure_decode_expect(self, field_data->expect, data->__value);
      rb_funcall(data->__type, id_dump, 6,
        data->__value,
        data->__output,
        data->__output_big,
        self,
        data->__iterator,
        data->__length);
    }
  }
  cStructure_restore_context(self);
  return Qnil;
}

static ID id_shape;
static ID id___shape_field;

static inline VALUE cStructure_shape_field(
    VALUE self,
    VALUE values,
    VALUE kind,
    VALUE field)
{
  VALUE res = Qnil;
  struct cStructure_data *data;
  struct cField_data *field_data;
  if (NIL_P(cStructure_decode_static_conditions(self, field)))
    return Qnil;
  TypedData_Get_Struct(self, struct cStructure_data, &cStructure_type, data);
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);

  if (RTEST(field_data->count)) {
    long count = RARRAY_LEN(values);
    res = rb_ary_new_capa(count);

    for (long i = 0; i < count; i++) {
      data->__iterator = LONG2NUM(i);
      if (RTEST(cStructure_decode_dynamic_conditions(self, field))) {
        VALUE shape = rb_funcall(data->__type, id_shape, 6,
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
    res = rb_class_new_instance(1, &res, kind);
  } else {
    data->__iterator = INT2FIX(0);
    if (RTEST(cStructure_decode_dynamic_conditions(self, field))) {
      res = rb_funcall(data->__type, id_shape, 6,
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
  cStructure_restore_context(self);
  return res;
}

static ID id_fields;

struct fields_state {
  VALUE self;
  VALUE fields;
  VALUE field;
};

static inline VALUE cStructure_fields_rescue(VALUE state_p, VALUE exception) {
  struct fields_state *state = (struct fields_state *)state_p;
  if (NIL_P(state->field)) {
    struct cStructure_data *data;
    TypedData_Get_Struct(state->self, struct cStructure_data, &cStructure_type, data);
    if (!NIL_P(rb_funcall(mLibBin, rb_intern("output"), 0)))
      rb_funcall(rb_funcall(mLibBin, rb_intern("output"), 0), rb_intern("print"), 6,
        rb_obj_class(state->self),
        rb_str_new_cstr(": could not load fields, index: "),
        data->__index,
        rb_str_new_cstr(", current position: "),
        data->__cur_position,
        rb_str_new_cstr("\n"));
  } else {
    struct cField_data *field_data;
    TypedData_Get_Struct(state->field, struct cField_data, &cField_type, field_data);
    if (!NIL_P(rb_funcall(mLibBin, rb_intern("output"), 0)))
      rb_funcall(rb_funcall(mLibBin, rb_intern("output"), 0), rb_intern("print"), 6,
        rb_obj_class(state->self),
        rb_str_new_cstr(": "),
        field_data->name,
        rb_str_new_cstr("("),
        field_data->type,
        rb_str_new_cstr(")\n"));
  }
  rb_exc_raise(exception);
  return state->self;
}

static inline VALUE cStructure_load_fields_wrapper(VALUE state_p) {
  struct fields_state *state = (struct fields_state *)state_p;
  state->fields = rb_ivar_get(rb_obj_class(state->self), id_fields);
  for (long i = 0; i < RARRAY_LEN(state->fields); i++) {
    state->field = rb_ary_entry(state->fields, i);
    struct cField_data *field_data;
    TypedData_Get_Struct(state->field, struct cField_data, &cField_type, field_data);
    rb_funcall(state->self, field_data->setter, 1, rb_funcall(state->self, id___load_field, 1, state->field));
  }
  return state->self;
}

static ID id___load_fields;

static VALUE cStructure_load_fields(VALUE self) {
  struct fields_state state = {self, Qnil, Qnil};
  rb_rescue(&cStructure_load_fields_wrapper, (VALUE)&state,
            &cStructure_fields_rescue, (VALUE)&state);
  return self;
}

static inline VALUE cStructure_dump_fields_wrapper(VALUE state_p) {
  struct fields_state *state = (struct fields_state *)state_p;
  state->fields = rb_ivar_get(rb_obj_class(state->self), id_fields);
  for (long i = 0; i < RARRAY_LEN(state->fields); i++) {
    state->field = rb_ary_entry(state->fields, i);
    struct cField_data *field_data;
    TypedData_Get_Struct(state->field, struct cField_data, &cField_type, field_data);
    rb_funcall(state->self, id___dump_field, 2, rb_funcall(state->self, field_data->getter, 0), state->field);
  }
  return state->self;
}

static ID id___dump_fields;

static VALUE cStructure_dump_fields(VALUE self) {
  struct fields_state state = {self, Qnil, Qnil};
  rb_rescue(&cStructure_dump_fields_wrapper, (VALUE)&state,
            &cStructure_fields_rescue, (VALUE)&state);
  return self;
}

static inline VALUE cStructure_convert_fields_wrapper(VALUE state_p) {
  struct fields_state *state = (struct fields_state *)state_p;
  state->fields = rb_ivar_get(rb_obj_class(state->self), id_fields);
  for (long i = 0; i < RARRAY_LEN(state->fields); i++) {
    state->field = rb_ary_entry(state->fields, i);
    struct cField_data *field_data;
    TypedData_Get_Struct(state->field, struct cField_data, &cField_type, field_data);
    rb_funcall(state->self, field_data->setter, 1, rb_funcall(state->self, id___convert_field, 1, state->field));
  }
  return state->self;
}

static ID id___convert_fields;

static VALUE cStructure_convert_fields(VALUE self) {
  struct fields_state state = {self, Qnil, Qnil};
  rb_rescue(&cStructure_convert_fields_wrapper, (VALUE)&state,
            &cStructure_fields_rescue, (VALUE)&state);
  return self;
}

struct shape_fields_state {
  VALUE self;
  VALUE fields;
  VALUE field;
  VALUE kind;
};

static inline VALUE cStructure_shape_fields_wrapper(VALUE state_p) {
  struct shape_fields_state *state = (struct shape_fields_state *)state_p;
  state->fields = rb_ivar_get(rb_obj_class(state->self), id_fields);
  VALUE members = rb_hash_new();
  for (long i = 0; i < RARRAY_LEN(state->fields); i++) {
    state->field = rb_ary_entry(state->fields, i);
    struct cField_data *field_data;
    TypedData_Get_Struct(state->field, struct cField_data, &cField_type, field_data);
    rb_hash_aset(members, ID2SYM(field_data->getter),
        rb_funcall(state->self, id___shape_field, 3, rb_funcall(state->self, field_data->getter, 0), state->kind, state->field));
  }
  return members;
}

static ID id___shape_fields;

static VALUE cStructure_shape_fields(VALUE self, VALUE kind) {
  struct shape_fields_state state = {self, Qnil, Qnil, kind};
  return rb_rescue(&cStructure_shape_fields_wrapper, (VALUE)&state,
                   &cStructure_fields_rescue, (VALUE)&state);
}

static ID id___load;

static inline VALUE cStructure_load(int argc, VALUE *argv, VALUE self) {
  VALUE input;
  VALUE input_big;
  VALUE parent;
  VALUE index;
  rb_scan_args(argc, argv, "22", &input, &input_big, &parent, &index);
  rb_funcall(self, id___set_load_state, 4, input, input_big, parent, index);
  rb_funcall(self, id___load_fields, 0);
  rb_funcall(self, id___unset_load_state, 0);
  return self;
}

static ID id___dump;

static inline VALUE cStructure_dump(int argc, VALUE *argv, VALUE self) {
  VALUE output;
  VALUE output_big;
  VALUE parent;
  VALUE index;
  rb_scan_args(argc, argv, "22", &output, &output_big, &parent, &index);
  rb_funcall(self, id___set_dump_state, 4, output, output_big, parent, index);
  rb_funcall(self, id___dump_fields, 0);
  rb_funcall(self, id___unset_dump_state, 0);
  return self;
}

static ID id___convert;

static VALUE cStructure_convert(int argc, VALUE *argv, VALUE self) {
  VALUE input;
  VALUE output;
  VALUE input_big;
  VALUE output_big;
  VALUE parent;
  VALUE index;
  rb_scan_args(argc, argv, "42", &input, &output, &input_big, &output_big, &parent, &index);
  rb_funcall(self, id___set_convert_state, 6, input, output, input_big, output_big, parent, index);
  rb_funcall(self, id___convert_fields, 0);
  rb_funcall(self, id___unset_convert_state, 0);
  return self;
}

static ID id___shape;

static VALUE cStructure_shape(int argc, VALUE *argv, VALUE self) {
  VALUE previous_offset;
  VALUE parent;
  VALUE index;
  VALUE kind;
  rb_scan_args(argc, argv, "04", &previous_offset, &parent, &index, &kind);
  if (NIL_P(previous_offset))
    previous_offset = INT2FIX(0);
  if (NIL_P(kind))
    kind = cDataShape;
  rb_funcall(self, id___set_size_state, 3, previous_offset, parent, index);
  VALUE members = rb_funcall(self, id___shape_fields, 1, kind);
  rb_funcall(self, id___unset_size_state, 0);
  if (RARRAY_LEN(rb_funcall(rb_funcall(members, rb_intern("values"), 0), rb_intern("compact"), 0)) <= 0)
    return Qnil;
  return rb_class_new_instance(1, &members, kind);
}

static VALUE cStructure_singl_load(int argc, VALUE *argv, VALUE self) {
  VALUE input;
  VALUE input_big;
  VALUE parent;
  VALUE index;
  VALUE length;
  rb_scan_args(argc, argv, "14", &input, &input_big, &parent, &index, &length);
  if (NIL_P(input_big))
    input_big = rb_funcall(mLibBin, rb_intern("default_big?"), 0);
  VALUE res;
  if (!NIL_P(length)) {
    long l = NUM2LONG(length);
    res = rb_ary_new_capa(l);
    for (long i = 0; i < l; i++) {
      VALUE obj = rb_class_new_instance(0, NULL, self);
      rb_funcall(obj, id___load, 4, input, input_big, parent, index);
      rb_ary_store(res, i, obj);
    }
    return res;
  } else {
    res = rb_class_new_instance(0, NULL, self);
    rb_funcall(res, id___load, 4, input, input_big, parent, index);
  }
  return res;
}

static VALUE cStructure_singl_dump(int argc, VALUE *argv, VALUE self) {
  VALUE value;
  VALUE output;
  VALUE output_big;
  VALUE parent;
  VALUE index;
  VALUE length;
  rb_scan_args(argc, argv, "24", &value, &output, &output_big, &parent, &index, &length);
  if (NIL_P(output_big))
    output_big = rb_funcall(mLibBin, rb_intern("default_big?"), 0);
  if (!NIL_P(length)) {
    long l = NUM2LONG(length);
    for (long i = 0; i < l; i++)
      rb_funcall(rb_ary_entry(value, i), id___dump, 4, output, output_big, parent, index);
  } else
    rb_funcall(value, id___dump, 4, output, output_big, parent, index);
  return value;
}

static VALUE cStructure_singl_convert(int argc, VALUE *argv, VALUE self) {
  VALUE input;
  VALUE output;
  VALUE input_big;
  VALUE output_big;
  VALUE parent;
  VALUE index;
  VALUE length;
  rb_scan_args(argc, argv, "25", &input, &output, &input_big, &output_big, &parent, &index, &length);
  if (NIL_P(input_big))
    input_big = rb_funcall(mLibBin, rb_intern("default_big?"), 0);
  if (NIL_P(output_big))
    output_big = RTEST(input_big) ? Qfalse : Qtrue;
  VALUE res;
  if (!NIL_P(length)) {
    long l = NUM2LONG(length);
    res = rb_ary_new_capa(l);
    for (long i = 0; i < l; i++) {
      VALUE obj = rb_class_new_instance(0, NULL, self);
      rb_funcall(obj, id___convert, 6, input, output, input_big, output_big, parent, index);
      rb_ary_store(res, i, obj);
    }
  } else {
    res = rb_class_new_instance(0, NULL, self);
    rb_funcall(res, id___convert, 6, input, output, input_big, output_big, parent, index);
  }
  return res;
}

static VALUE cStructure_singl_shape(int argc, VALUE *argv, VALUE self) {
  VALUE value;
  VALUE previous_offset;
  VALUE parent;
  VALUE index;
  VALUE kind;
  VALUE length;
  rb_scan_args(argc, argv, "15", &value, &previous_offset, &parent, &index, &kind, &length);
  if (NIL_P(previous_offset))
    previous_offset = INT2FIX(0);
  if (NIL_P(kind))
    kind = cDataShape;
  VALUE res;
  if (!NIL_P(length)) {
    long l = NUM2LONG(length);
    res = rb_ary_new_capa(l);
    for (long i = 0; i < l; i++)
      rb_ary_store(res, i, rb_funcall(rb_ary_entry(value, i), id___shape, 4, previous_offset, parent, index, kind));
    res = rb_class_new_instance(1, &res, kind);
  } else
    res = rb_funcall(value, id___shape, 4, previous_offset, parent, index, kind);
  return res;
}

static void define_cStructure() {
  id_tell = rb_intern("tell");
  id_seek = rb_intern("seek");
  id_fields = rb_intern("@fields");
  id_instance_exec = rb_intern("instance_exec");

  id___set_load_state = rb_intern("__set_load_state");
  id___unset_load_state = rb_intern("__unset_load_state");
  id___load_field = rb_intern("__load_field");
  id___load_fields = rb_intern("__load_fields");
  id_load = rb_intern("load");
  id___load = rb_intern("__load");

  id___set_dump_state = rb_intern("__set_dump_state");
  id___unset_dump_state = rb_intern("__unset_dump_state");
  id___dump_field = rb_intern("__dump_field");
  id___dump_fields = rb_intern("__dump_fields");
  id_dump = rb_intern("dump");
  id___dump = rb_intern("__dump");

  id___set_convert_state = rb_intern("__set_convert_state");
  id___unset_convert_state = rb_intern("__unset_convert_state");
  id___convert_field = rb_intern("__convert_field");
  id___convert_fields = rb_intern("__convert_fields");
  id_convert = rb_intern("convert");
  id___convert = rb_intern("__convert");

  id___set_size_state = rb_intern("__set_size_state");
  id___unset_size_state = rb_intern("__unset_size_state");
  id___shape_field = rb_intern("__shape_field");
  id___shape_fields = rb_intern("__shape_fields");
  id_shape = rb_intern("shape");
  id___shape = rb_intern("__shape");

  cStructure = rb_define_class_under(mLibBin, "Structure", rb_cObject);
  rb_define_alloc_func(cStructure, cStructure_alloc);
  /**
   * Create new Structure object.
   * @return [Structure] a new Structure instance with state set to nil.
   */
  rb_define_method(cStructure, "initialize", cStructure_initialize, 0);

  DEFINE_STRUCT_STATE_ACCESSORS(parent);
  DEFINE_STRUCT_STATE_ACCESSORS(index);
  DEFINE_STRUCT_STATE_ACCESSORS(position);
  DEFINE_STRUCT_STATE_ACCESSORS(cur_position);
  DEFINE_STRUCT_STATE_ACCESSORS(input);
  DEFINE_STRUCT_STATE_ACCESSORS(output);
  DEFINE_STRUCT_STATE_ACCESSORS(input_big);
  DEFINE_STRUCT_STATE_ACCESSORS(output_big);

  DEFINE_STRUCT_STATE_ACCESSORS(offset);
  DEFINE_STRUCT_STATE_ACCESSORS(condition);
  DEFINE_STRUCT_STATE_ACCESSORS(type);
  DEFINE_STRUCT_STATE_ACCESSORS(length);
  DEFINE_STRUCT_STATE_ACCESSORS(iterator);
  DEFINE_STRUCT_STATE_ACCESSORS(count);
  DEFINE_STRUCT_STATE_ACCESSORS(value);

  /**
   * @overload __set_convert_state(input, output, input_big, output_big, parent, index)
   *   Set attributes for conversion
   *   @param input [IO] the stream to read data from
   *   @param output [IO] the stream to write data to
   *   @param input_big [Boolean] str endianness of +input+
   *   @param output_big [Boolean] str endianness of +output+
   *   @param parent [nil,Structure] the parent if it exists, nil otherwise
   *   @param index [nil,Integer] the index if the structure is repeated, nil otherwise
   *   @return [nil]
   *   @example
   *     # Orignal Ruby implementation
   *     def __set_convert_state(input, output, input_big, output_big, parent, index)
   *       __input_big = input_big
   *       __output_big = output_big
   *       __input = input
   *       __output = output
   *       __parent = parent
   *       __index = index
   *       __position = input.tell
   *       __cur_position = __position
   *     end
   */
  rb_define_method(cStructure, "__set_convert_state", cStructure_set_convert_state, 6);
  /**
   * Unset attributes after conversion.
   * @return [nil]
   * @example
   *   # Orignal Ruby implementation
   *   def __unset_convert_state
   *     __input_big = nil
   *     __output_big = nil
   *     __input = nil
   *     __output = nil
   *     __parent = nil
   *     __index = nil
   *     __position = nil
   *     __cur_position = nil
   *   end
   */
  rb_define_method(cStructure, "__unset_convert_state", cStructure_unset_convert_state, 0);
  /**
   * @overload __set_size_state(position, parent, index)
   *   Set attributes for computing size or shape
   *   @param position [Integer] The position of the field
   *   @param parent [nil,Structure] the parent if it exists, nil otherwise
   *   @param index [nil,Integer] the index if the structure is repeated, nil otherwise
   *   @return [nil]
   *   @example
   *     # Orignal Ruby implementation
   *     def __set_size_state(position, parent, index)
   *       __parent = parent
   *       __index = index
   *       __position = position
   *       __cur_position = __position
   *     end
   */
  rb_define_method(cStructure, "__set_size_state", cStructure_set_size_state, 3);
  /**
   * Unset attributes after size or shape computation
   * @return [nil]
   * @example
   *   # Orignal Ruby implementation
   *   def __unset_size_state
   *     __parent = nil
   *     __index = nil
   *     __position = nil
   *     __cur_position = nil
   *   end
   */
  rb_define_method(cStructure, "__unset_size_state", cStructure_unset_size_state, 0);
  /**
   * @overload __set_load_state(input, input_big, parent, index)
   *   Set attributes for loading
   *   @param input [IO] the stream to read data from
   *   @param input_big [Boolean] str endianness of +input+
   *   @param parent [nil,Structure] the parent if it exists, nil otherwise
   *   @param index [nil,Integer] the index if the structure is repeated, nil otherwise
   *   @return [nil]
   *   @example
   *     # Orignal Ruby implementation
   *     def __set_load_state(input, input_big, parent, index)
   *       __input_big = input_big
   *       __input = input
   *       __parent = parent
   *       __index = index
   *       __position = input.tell
   *       __cur_position = __position
   *     end
   */
  rb_define_method(cStructure, "__set_load_state", cStructure_set_load_state, 4);
  /**
   * Unset attributes after loading
   * @return [nil]
   * @example
   *   # Orignal Ruby implementation
   *   def __unset_load_state
   *     __input_big = nil
   *     __input = nil
   *     __parent = nil
   *     __index = nil
   *     __position = nil
   *     __cur_position = nil
   *   end
   */
  rb_define_method(cStructure, "__unset_load_state", cStructure_unset_load_state, 0);
  /**
   * @overload __set_dump_state(output, output_big, parent, index)
   *   Set attributes for dumping
   *   @param output [IO] the stream to write data to
   *   @param output_big [Boolean] str endianness of +output+
   *   @param parent [nil,Structure] the parent if it exists, nil otherwise
   *   @param index [nil,Integer] the index if the structure is repeated, nil otherwise
   *   @return [nil]
   *   @example
   *     # Orignal Ruby implementation
   *     def __set_dump_state(output, output_big, parent, index)
   *       __output_big = output_big
   *       __output = output
   *       __parent = parent
   *       __index = index
   *       __position = output.tell
   *       __cur_position = __position
   *     end
   */
  rb_define_method(cStructure, "__set_dump_state", cStructure_set_dump_state, 4);
  /**
   * Unset attributes after dumping
   * @return [nil]
   * @example
   *   # Orignal Ruby implementation
   *   def __unset_dump_state
   *     __output_big = nil
   *     __output = nil
   *     __parent = nil
   *     __index = nil
   *     __position = nil
   *     __cur_position = nil
   *   end
   */
  rb_define_method(cStructure, "__unset_dump_state", cStructure_unset_dump_state, 0);

  /**
   * @overload __decode_expression(expression)
   *   Decode the given expression in the context of the receiver.
   *   @param expression [Proc,Object] the expression to decode
   *   @return [Object] the decoded value
   *   @note Do not overload, this is not called through the usual ruby dispatch for performance reasons
   *   @example
   *     # Original Ruby implementation
   *     def __decode_expression(expression)
   *       if expression.is_a?(Proc)
   *         instance_exec &expression
   *       else
   *         expression
   *       end
   *     end
   */
  rb_define_method(cStructure, "__decode_expression", cStructure_decode_expression, 1);
  /**
   * @overload __decode_expect(expect, value)
   *   Decode the given expect expression, given the field value.
   *   @param expect [Proc,Object,nil] the expression to decode
   *   @param value [Object] the field value to validate
   *   @return [Object] returns +value+ if validated. If expect is a Proc, the proc will be passed +value+
   *     as an argument and be evaluated. If expect is a scalar, +expect+ and +value+ will be tested for equality.
   *     If the result of the evaluation or the test is truthy, value is validate. Else an exception is raised.
   *   @example
   *     # Original Ruby implementation
   *     def __decode_expect(expect, value)
   *       return value if expect.nil?
   *       if expect.is_a?(Proc)
   *         raise "could not validate field value: #{value}" unless instance_exec(value, &expect)
   *       else
   *         raise "could not validate field value: #{value} expected: #{expect}" unless expect == value
   *       end
   *       return value
   *     end
   */
  rb_define_method(cStructure, "__decode_expect", cStructure_decode_expect, 2);
  /**
   * @overload __decode_seek_offset(offset, relative_offset, align)
   *   Decode the offset and seek to this position in the active streams.
   *   @param offset [Proc,Integer,nil] the expression to decode
   *   @param relative_offset [Boolean] the offset should be relative to the structure base (+__position+)
   *   @param align [false,Integer] the required alignement of the field
   *   @return [nil,false,Integer] return nil if no offset was specified, an Integer if the active streams
   *     had to seek, or false if a zero offset was computed.
   *   @note Do not overload, this is not called through the usual ruby dispatch for performance reasons
   *   @example
   *     # Original Ruby implementation
   *     def __decode_seek_offset(offset, relative_offset, align)
   *       cur_pos = nil
   *       if !offset
   *         return nil unless align
   *         cur_pos =
   *           if __input
   *             __input.tell
   *           elsif __output
   *             __output.tell
   *           else
   *             __cur_position
   *           end
   *         pad = cur_pos % align
   *         return nil if pad == 0
   *         cur_pos += align - pad
   *       else
   *         cur_pos = __decode_expression(offset)
   *         return false if cur_pos == 0x0
   *         cur_pos += __position if relative_offset
   *         cur_pos +=  align - (cur_pos % align) if align && cur_pos % align > 0
   *       end
   *       __cur_position = cur_pos
   *       __input.seek(cur_pos) if __input
   *       __output.seek(cur_pos) if __output
   *       cur_pos
   *     end
   */
  rb_define_method(cStructure, "__decode_seek_offset", cStructure_decode_seek_offset, 3);
  /**
   * @overload __decode_condition(condition)
   *   Decode the condition expression
   *   @param condition [Proc,Boolean,nil] the expression to decode
   *   @return [Boolean] the field is active
   *   @note Do not overload, this is not called through the usual ruby dispatch for performance reasons
   *   @example
   *     # Original Ruby implementation
   *     def __decode_condition(condition)
   *       return true unless condition
   *       __decode_expression(condition)
   *     end
   */
  rb_define_method(cStructure, "__decode_condition", cStructure_decode_condition, 1);
  /**
   * @overload __decode_count(count)
   *   Decode the count expression.
   *   @param count [Proc,Integer,nil] the expression to decode
   *   @return [Integer] 1 if +count+ is nil, the decoded count otherwise
   *   @note Do not overload, this is not called through the usual ruby dispatch for performance reasons
   *   @example
   *     # Original Ruby implementation
   *     def __decode_count(count)
   *       return 1 unless count
   *       __decode_expression(count)
   *     end
   */
  rb_define_method(cStructure, "__decode_count", cStructure_decode_count, 1);
  /**
   * @overload __decode_type(type)
   *   Decode the type expression.
   *   @param type [Proc,Class] the expression to decode
   *   @return [Class] the decoded type
   *   @note Do not overload, this is not called through the usual ruby dispatch for performance reasons
   *   @example
   *     # Original Ruby implementation
   *     def __decode_type(type)
   *       __decode_expression(type)
   *     end
   */
  rb_define_method(cStructure, "__decode_type", cStructure_decode_type, 1);
  /**
   * @overload __decode_length(length)
   *   Decode the length expression.
   *   @param length [Proc,Integer,nil] the expression to decode
   *   @return [Integer,nil] nil if the field is not a vector, or the
   *     length of the vector
   *   @note Do not overload, this is not called through the usual ruby dispatch for performance reasons
   *   @example
   *     # Original Ruby implementation
   *     def __decode_length(length)
   *       __decode_expression(length)
   *     end
   */
  rb_define_method(cStructure, "__decode_length", cStructure_decode_length, 1);
  /**
   * @overload __decode_static_conditions(field)
   *   Sets field specific context. Return nil if the field is inactive.
   *   If the field is a sequence, only +__count+ is set, if not,
   *   +__offset+, +__condition+, +__type+, and +__count+ are
   *   also set.
   *   @param field [Field] the field descriptor
   *   @return [Integer,nil] nil if the field is inactive, the repetition count otherwise
   *   @note Do not overload, this is not called through the usual ruby dispatch for performance reasons
   *   @example
   *     # Original Ruby implementation
   *     def __decode_static_conditions(field)
   *       __offset = nil
   *       __condition = nil
   *       __type = nil
   *       __length = nil
   *       __count = nil
   *       unless field.sequence?
   *         __offset = __decode_seek_offset(field.offset, field.relative_offset?, field.align)
   *         return nil if __offset == false
   *         __condition = __decode_condition(field.condition)
   *         return nil unless __condition
   *         __type = __decode_type(field.type)
   *         __length = __decode_length(field.length)
   *       end
   *       __count = __decode_count(field.count)
   *     end
   */
  rb_define_method(cStructure, "__decode_static_conditions", cStructure_decode_static_conditions, 1);
  /**
   * @overload __decode_dynamic_conditions(field)
   *   Sets field repetition specific context.
   *   Namely +__offset+, +__condition+, +__type+, and +__length+
   *   should be set if the field repetition is active.
   *   @param field [Field] the field descriptor
   *   @return [Boolean] field repetition is active
   *   @note Do not overload, this is not called through the usual ruby dispatch for performance reasons.
   *   @example
   *     # Original Ruby implementation
   *     def __decode_dynamic_conditions(field)
   *       return true unless field.sequence?
   *       __offset = nil
   *       __condition = nil
   *       __type = nil
   *       __length = nil
   *       __offset = __decode_seek_offset(field.offset, field.relative_offset?, field.align)
   *       return false if __offset == false
   *       __condition = __decode_condition(field.condition)
   *       return false unless __condition
   *       __type = __decode_type(field.type)
   *       __length = __decode_length(field.length)
   *       return true
   *     end
   */
  rb_define_method(cStructure, "__decode_dynamic_conditions", cStructure_decode_dynamic_conditions, 1);

  /**
   * Restore the field specific context.
   * @return nil
   * @example
   *   # Original Ruby implementation
   *   def __restore_context
   *     __iterator = nil
   *     __type = nil
   *     __length = nil
   *     __count = nil
   *     __offset = nil
   *     __condition = nil
   *     __value = nil
   *   end
   */
  rb_define_method(cStructure, "__restore_context", cStructure_restore_context, 0);

  /**
   * @overload __convert_field(field)
   *   Load and dump the value of a structure field.
   *   @param field [Field] the field descriptor
   *   @return the field value or nil if the field was inactive
   *   @example
   *     # Original Ruby implementation
   *     def __convert_field(field)
   *       return nil if __decode_static_conditions(field).nil?
   *       vs = __count.times.collect do |it|
   *         __iterator = it
   *         if __decode_dynamic_conditions(field)
   *           __value = __type::convert(__input, __output, __input_big, __output_big, self, it, __length)
   *           __decode_expect(field.expect, __value)
   *         else
   *           nil
   *         end
   *       end
   *       __restore_context
   *       vs = vs.first unless field.count
   *       vs
   *     end
   */
  rb_define_method(cStructure, "__convert_field", cStructure_convert_field, 1);
  /**
   * @overload __load_field(field)
   *   Load the value of a structure field.
   *   @param field [Field] the field descriptor
   *   @return the field value or nil if the field was inactive
   *   @example
   *     # Original Ruby implementation
   *     def __load_field(field)
   *       return nil if __decode_static_conditions(field).nil?
   *       vs = __count.times.collect do |it|
   *         __iterator = it
   *         if __decode_dynamic_conditions(field)
   *           __value = __type::load(__input, __input_big, self, it, __length)
   *           __decode_expect(field.expect, __value)
   *         else
   *           nil
   *         end
   *       end
   *       __restore_context
   *       vs = vs.first unless field.count
   *       vs
   *     end
   */
  rb_define_method(cStructure, "__load_field", cStructure_load_field, 1);
  /**
   * @overload __dump_field(value, field)
   *   Dump the value of a structure field.
   *   @param value the field value
   *   @param field [Field] the field descriptor
   *   @return nil
   *   @example
   *     # Original Ruby implementation
   *     def __dump_field(vs, field)
   *       return nil if __decode_static_conditions(field).nil?
   *       vs = [vs] unless field.count
   *       __count.times do |it|
   *         __iterator = it
   *         if __decode_dynamic_conditions(field)
   *           __value = vs[it]
   *           __decode_expect(field.expect, __value)
   *           __type::dump(__value, __output, __output_big, self, it, __length)
   *         end
   *       end
   *       __restore_context
   *     end
   */
  rb_define_method(cStructure, "__dump_field", cStructure_dump_field, 2);
  /**
   * @overload __shape_field(value, kind, field)
   *   Return the shape of the structure field.
   *   @param value the field value
   *   @param kind [Class] the class of the shape required
   *   @param field [Field] the field descriptor
   *   @return [nil,kind,Array<kind>] the field shape, or nil if the field was inactive
   *   @example
   *     # Original Ruby implementation
   *     def __shape_field(vs, kind, field)
   *       return nil if __decode_static_conditions(field).nil?
   *       vs = [vs] unless field.count
   *       vs = vs.each_with_index.collect do |v, it|
   *         __iterator = it
   *         if __decode_dynamic_conditions(field)
   *           sh = __type::shape(v, __cur_position, self, it, kind, __length)
   *           __cur_position = sh.last + 1 if sh.last && sh.last >= 0
   *           sh
   *         end
   *       end
   *       __restore_context
   *       vs = field.count ? kind.new(vs) : vs.first
   *       vs
   *     end
   */
  rb_define_method(cStructure, "__shape_field", cStructure_shape_field, 3);

  /**
   * Load the fields of the structure. The load state must
   * have been set beforehand.
   * @return [Structure] self
   * @example
   *   # Original Ruby implementation
   *   def __load_fields
   *     field = nil
   *     begin
   *       __fields.each { |field|
   *         send("#{field.name}=", __load_field(field))
   *       }
   *     rescue
   *       STDERR.puts "#{self.class}: #{field.name}(#{field.type})"
   *       raise
   *     end
   *     self
   *   end
   */
  rb_define_method(cStructure, "__load_fields", cStructure_load_fields, 0);
  /**
   * Dump the fields of the structure. The dump state must
   * have been set beforehand.
   * @return [Structure] self
   * @example
   *   # Original Ruby implementation
   *   def __dump_fields
   *     field = nil
   *     begin
   *       __fields.each { |field|
   *         __dump_field(send(field.name), field)
   *       }
   *     rescue
   *       STDERR.puts "#{self.class}: #{field.name}(#{field.type})"
   *       raise
   *     end
   *     self
   *   end
   */
  rb_define_method(cStructure, "__dump_fields", cStructure_dump_fields, 0);
  /**
   * Convert the fields of the structure. The conversion
   * state must have been set beforehand.
   * @return [Structure] self
   * @example
   *   # Original Ruby implementation
   *   def __convert_fields
   *     field = nil
   *     begin
   *       __fields.each { |field|
   *         send("#{field.name}=", __convert_field(field))
   *       }
   *     rescue
   *       STDERR.puts "#{self.class}: #{field.name}(#{field.type})"
   *       raise
   *     end
   *     self
   *   end
   */
  rb_define_method(cStructure, "__convert_fields", cStructure_convert_fields, 0);
  /**
   * @overload __shape_fields(kind)
   *   Return the shape of the structure fields in a Hash, indexed by the fields' names.
   *   The size state must have been set beforehand.
   *   @param kind [Class] the kind of structure to create
   *   @return [Hash{Symbol=>kind}] the fields shape
   *   @example
   *     # Original Ruby implementation
   *     def __shape_fields(kind)
   *       members = {}
   *       field = nil
   *       begin
   *         __fields.each { |field|
   *           members[field.name] = __shape_field(send(field.name), kind, field)
   *         }
   *       rescue
   *         LibBin.output.puts "#{self.class}: #{field.name}(#{field.type})" if LibBin.output
   *         raise
   *       end
   *       return members
   *     end
   */
  rb_define_method(cStructure, "__shape_fields", cStructure_shape_fields, 1);

  /**
   * @overload __load(input, input_big, parent = nil, index = nil)
   *   Fill in the structure by loading it from +input+.
   *   @param input [IO] the stream to load the structure from
   *   @param input_big [Boolean] the endianness of +input+
   *   @param parent [Structure] if given, the parent of the structure
   *   @param index [Integer] if given, the structure is repeated and +index+ is the rank this structure
   *   @return [Structure] self
   *   @example
   *     # Original Ruby implementation
   *     def __load(input, input_big, parent = nil, index = nil)
   *       __set_load_state(input, input_big, parent, index)
   *       __load_fields
   *       __unset_load_state
   *       self
   *     end
   */
  rb_define_method(cStructure, "__load", cStructure_load, -1);
  /**
   * @overload __dump(output, output_big, parent = nil, index = nil)
   *   Dump the structure to +output+.
   *   @param output [IO] the stream to dump the structure to
   *   @param output_big [Boolean] the endianness of +output+
   *   @param parent [Structure] if given, the parent of the structure
   *   @param index [Integer] if given, the structure is repeated and +index+ is the rank this structure
   *   @return [Structure] self
   *   @example
   *     # Original Ruby implementation
   *     def __dump(output, output_big, parent = nil, index = nil)
   *       __set_dump_state(output, output_big, parent, index)
   *       __dump_fields
   *       __unset_dump_state
   *       self
   *     end
   */
  rb_define_method(cStructure, "__dump", cStructure_dump, -1);
  /**
   * @overload __convert(input, output, input_big, output_big, parent = nil, index = nil)
   *   Fill in the structure by loading it from +input+ and
   *   dumping it to +output+.
   *   @param input [IO] the stream to load the structure from
   *   @param output [IO] the stream to dump the structure to
   *   @param input_big [Boolean] the endianness of +input+
   *   @param output_big [Boolean] the endianness of +output+
   *   @param parent [Structure] if given, the parent of the structure
   *   @param index [Integer] if given, the structure is repeated and +index+ is the rank this structure
   *   @return [Structure] self
   *   @example
   *     # Original Ruby implementation
   *     def __convert(input, output, input_big, output_big, parent = nil, index = nil)
   *       __set_convert_state(input, output, input_big, output_big, parent, index)
   *       __convert_fields
   *       __unset_convert_state
   *       self
   *     end
   */
  rb_define_method(cStructure, "__convert", cStructure_convert, -1);
  /**
   * @overload __shape(offset = 0, parent = nil, index = nil, kind = DataShape)
   *   Return the shape of the structure.
   *   @param offset [Integer] the base position of the field
   *   @param parent [Structure] if given, the parent of the structure
   *   @param index [Integer] if given, the structure is repeated and +index+ is the rank this structure
   *   @param kind [Class] the kind of structure to create
   *   @return [kind] the the shape of the structure
   *   @example
   *     # Original Ruby implementation
   *     def __shape(previous_offset = 0, parent = nil, index = nil, kind = DataShape)
   *       __set_size_state(previous_offset, parent, index)
   *       members = __shape_fields(kind)
   *       __unset_size_state
   *       return nil if members.values.compact.size <= 0
   *       kind::new(members)
   *     end
   */
  rb_define_method(cStructure, "__shape", cStructure_shape, -1);

  /**
   * @overload load(input, input_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
   *   Load a structure from +input+.
   *   @param input [IO] the stream to load the structure from
   *   @param input_big [Boolean] the endianness of +input+
   *   @param parent [Structure] if given, the parent of the structure
   *   @param index [Integer] if given, the structure is repeated and +index+ is the rank this structure
   *   @param length [Integer] if given, the length of the vector of structure
   *   @return [Structure,Array<Structure>] a new strcuture, or
   *     an array of structures if length was specified
   *   @example
   *     # Original Ruby implementation
   *     def self.load(input, input_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
   *       if length
   *         length.times.collect {
   *           self.new.__load(input, input_big, parent, index)
   *         }
   *       else
   *         self.new.__load(input, input_big, parent, index)
   *       end
   *     end
   */
  rb_define_singleton_method(cStructure, "load", cStructure_singl_load, -1);
  /**
   * @overload dump(value, output, output_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
   *   Dump a structure to +output+.
   *   @param value [Structure,Array<Structure>] the value of structure to dump
   *   @param output [IO] the stream to dump the structure to
   *   @param output_big [Boolean] the endianness of +output+
   *   @param parent [Structure] if given, the parent of the structure
   *   @param index [Integer] if given, the structure is repeated and +index+ is the rank this structure
   *   @param length [Integer] if given, the length of the vector of structure
   *   @return [Structure,Array<Structure>] +value+
   *   @example
   *     # Original Ruby implementation
   *     def self.dump(value, output, output_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
   *       if length
   *         length.times.each { |i|
   *           value[i].__dump(output, output_big, parent, index)
   *         }
   *         value
   *       else
   *         value.__dump(output, output_big, parent, index)
   *       end
   *     end   */
  rb_define_singleton_method(cStructure, "dump", cStructure_singl_dump, -1);
  /**
   * @overload convert(input, output, input_big = LibBin::default_big?, output_big = !input_big, parent = nil, index = nil, length = nil)
   *   Convert a structure by loading it from +input+ and
   *   dumping it to +output+. Returns the loaded structure.
   *   @param input [IO] the stream to load the structure from
   *   @param output [IO] the stream to dump the structure to
   *   @param input_big [Boolean] the endianness of +input+
   *   @param output_big [Boolean] the endianness of +output+
   *   @param parent [Structure] if given, the parent of the structure
   *   @param index [Integer] if given, the structure is repeated and +index+ is the rank this structure
   *   @param length [Integer] if given, the length of the vector of structure
   *   @return [Structure,Array<Structure>] a new strcuture, or
   *     an array of structures if length was specified
   *   @example
   *     # Original Ruby implementation
   *     def self.convert(input, output, input_big = LibBin::default_big?, output_big = !input_big, parent = nil, index = nil, length = nil)
   *       if length
   *         length.times.collect {
   *           self.new.__convert(input, output, input_big, output_big, parent, index)
   *         }
   *       else
   *         self.new.__convert(input, output, input_big, output_big, parent, index)
   *       end
   *     end
   */
  rb_define_singleton_method(cStructure, "convert", cStructure_singl_convert, -1);
  /**
   * @overload shape(value, offset = 0, parent = nil, index = nil, kind = DataShape, length = nil)
   *   Return the shape of the value.
   *   @param value [Structure,Array<Structure>] the value of structure to get the shape of
   *   @param offset [Integer] the base position of the field
   *   @param parent [Structure] if given, the parent of the structure
   *   @param index [Integer] if given, the structure is repeated and +index+ is the rank this structure
   *   @param kind [Class] the kind of structure to create
   *   @param length [Integer] if given, the length of the vector of structure
   *   @return [kind] the the shape of the structure
   *   @example
   *     # Original Ruby implementation
   *     def self.shape(value, previous_offset = 0, parent = nil, index = nil, kind = DataShape, length = nil)
   *       if length
   *         kind.new(length.times.collect { |i|
   *           value[i].__shape(previous_offset, parent, index, kind)
   *         })
   *       else
   *         value.__shape(previous_offset, parent, index, kind)
   *       end
   *     end
   */
  rb_define_singleton_method(cStructure, "shape", cStructure_singl_shape, -1);
}

static VALUE pghalf_from_string_p(VALUE self, VALUE str, VALUE pack_str) {
  (void)self;
  Check_Type(str, T_STRING);
  Check_Type(pack_str, T_STRING);
  VALUE arr = rb_funcall(str, rb_intern("unpack"), 1, pack_str);
  uint16_t val = NUM2USHORT(rb_funcall(arr, rb_intern("first"), 0));
  union float_u res;

  res.i = pghalf_to_float(val);
  return DBL2NUM(res.f);
}

static VALUE half_from_string_p(VALUE self, VALUE str, VALUE pack_str) {
  (void)self;
  Check_Type(str, T_STRING);
  Check_Type(pack_str, T_STRING);
  VALUE arr = rb_funcall(str, rb_intern("unpack"), 1, pack_str);
  uint16_t val = NUM2USHORT(rb_funcall(arr, rb_intern("first"), 0));
  union float_u res;

  res.i = half_to_float(val);
  return DBL2NUM(res.f);
}

static VALUE pghalf_to_string_p(VALUE self, VALUE number, VALUE pack_str) {
  (void)self;
  Check_Type(number, T_FLOAT);
  union float_u val;
  uint16_t res;

  val.f = NUM2DBL(number);
  res = pghalf_from_float(val.i);
  VALUE arr = rb_ary_new3(1, UINT2NUM(res) );

  return rb_funcall(arr, rb_intern("pack"), 1, pack_str);
}

static VALUE half_to_string_p(VALUE self, VALUE number, VALUE pack_str) {
  (void)self;
  Check_Type(number, T_FLOAT);
  union float_u val;
  uint16_t res;

  val.f = NUM2DBL(number);
  res = half_from_float(val.i);
  VALUE arr = rb_ary_new3(1, UINT2NUM(res) );

  return rb_funcall(arr, rb_intern("pack"), 1, pack_str);
}

static void define_cField() {
  id_gsub = rb_intern("gsub");

  rb_str_dot_dot = rb_obj_freeze(rb_str_new_cstr(".."));
  rb_gc_register_mark_object(rb_str_dot_dot);
  rb_str___parent = rb_obj_freeze(rb_str_new_cstr("__parent"));
  rb_gc_register_mark_object(rb_str___parent);
  rb_str_backslash = rb_obj_freeze(rb_str_new_cstr("\\"));
  rb_gc_register_mark_object(rb_str_backslash);
  rb_str_dot = rb_obj_freeze(rb_str_new_cstr("."));
  rb_gc_register_mark_object(rb_str_dot);

  cField = rb_define_class_under(cStructure, "Field", rb_cObject);
  rb_define_alloc_func(cField, cField_alloc);
  /**
   * @overload initialize(name, type, length, count, offset, sequence, condition, relative_offset, align, expect)
   *   @param name [Symbol, String] the name of the field.
   *   @param type [Class, String, Proc] the type of the field, as a Class, or
   *     as a String or Proc that will be evaluated in the context of the
   *     {Structure} instance.
   *   @param length [nil, Integer, String, Proc] if given, consider the field a
   *     vector of the type. The length is either a constant Integer of a
   *     String or Proc that will be evaluated in the context of the
   *     {Structure} instance.
   *   @param count [nil, Integer, String, Proc] if given, consider the field is
   *     repeated count times. The count is either a constant Integer of a
   *     String or Proc that will be evaluated in the context of the
   *     {Structure} instance.
   *   @param offset [nil, integer, String, Proc] if given, the absolute offset in
   *     the file, or the offset from the parent position, where the field can
   *     be found. See relative offset. The offset is either a constant
   *     Integer of a String or Proc that will be evaluated in the context
   *     of the {Structure} instance.
   *   @param sequence [Boolean] if true, +type+, +length+, +offset+, and
   *     +condition+ are evaluated for each repetition.
   *   @param condition [nil, String, Proc] if given, the field, or repetition of the
   *     field can be conditionally present. The condition will be evaluated in
   *     the context of the {Structure} instance.
   *   @param relative_offset [Boolean] consider the +offset+ relative to
   *     the field +parent+.
   *   @param align [nil, Integer] if given, align the field. If given as an
   *     Integer it must be a power of 2. Else the field is aligned to the
   *     field's type preferred alignment
   *   @param expect [nil, Proc, Object] if given as a Proc the proc must
   *     evaluate to a truthy value or an exception will be raised. Else, the
   *     object will be tested for equality, and an exception will be raised if
   *     objects were not equal. Proc is evaluated in the context of the {Structure}
   *     instance and #__value will be set. Each repetition is validated.
   *   @return [Field] new Field
   */
  rb_define_method(cField, "initialize", cField_initialize, 10);
  DEFINE_FIELD_STATE_GETTER(name);
  DEFINE_FIELD_STATE_GETTER(type);
  DEFINE_FIELD_STATE_GETTER(length);
  DEFINE_FIELD_STATE_GETTER(count);
  DEFINE_FIELD_STATE_GETTER(offset);
  DEFINE_FIELD_STATE_GETTER_BOOL(sequence);
  DEFINE_FIELD_STATE_GETTER(condition);
  DEFINE_FIELD_STATE_GETTER_BOOL(relative_offset);
  DEFINE_FIELD_STATE_GETTER(align);
  DEFINE_FIELD_STATE_GETTER(expect);
}

void Init_libbin_c() {
  mLibBin = rb_define_module("LibBin");
  /**
   * @overload half_from_string(str, unpack_str)
   *   Load (unpacks) a half precision floating point.
   *   @param str [String] the strin to read the  number from
   *   @param unpack_str [String] the unpack format of the underlying 16bit integer
   *   @return [Float] the ruby representation of the value
   *   @example
   *     # Read a half precision floating point value stored in 'str' in little endian fashion
   *     value = half_from_string(str, "S<")
   */
  rb_define_module_function(mLibBin, "half_from_string", half_from_string_p, 2);
  /**
   * @overload half_to_string(value, pack_str)
   *   Convert a Numeric value to a half precision floating point and pack it into a string.
   *   @param value [Numeric] the number to convert
   *   @param pack_str [String] the pack format to store the underlying 16 bit integer representation of the value
   *   @return [String] the packed half precision value
   *   @example
   *     # Stores a number as a half precision floating point value in a big endian fashion
   *     str = half_to_string(value, "S>")
   */
  rb_define_module_function(mLibBin, "half_to_string", half_to_string_p, 2);
  /**
   * @overload pghalf_from_string(str, unpack_str)
   *   Load (unpacks) a half precision floating point as used by PlatinumGames in some formats.
   *   @param str [String] the strin to read the  number from
   *   @param unpack_str [String] the unpack format of the underlying 16bit integer
   *   @return [Float] the ruby representation of the value
   *   @example
   *     # Read a half precision floating point value stored in 'str' in little endian fashion
   *     half_from_string(str, "S<")
   */
  rb_define_module_function(mLibBin, "pghalf_from_string", pghalf_from_string_p, 2);
  /**
   * @overload pghalf_to_string(value, pack_str)
   *   Convert a Numeric value to a half precision floating point as used by PlatinumGames in some formats, and pack it into a string.
   *   @param value [Numeric] the number to convert
   *   @param pack_str [String] the pack format to store the underlying 16 bit integer representation of the value
   *   @return [String] the packed half precision value
   *   @example
   *     # Stores a number as a half precision floating point value in a big endian fashion
   *     str = half_to_string(value, "S>")
   */
  rb_define_module_function(mLibBin, "pghalf_to_string", pghalf_to_string_p, 2);
  cDataRange = rb_define_class_under(mLibBin, "DataRange", rb_cObject);
  cDataShape = rb_define_class_under(mLibBin, "DataShape", cDataRange);
  define_cStructure();
  define_cField();
  define_cScalar();
}
