#include "ruby.h"
#include "./libbin_c.h"

VALUE cField;
VALUE mLibBin;
VALUE cDataConverter;

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
    VALUE relative_offset) {
  VALUE tmp;
  struct cField_data *data;
  TypedData_Get_Struct(self, struct cField_data, &cField_type, data);
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
  VALUE ary = rb_ary_new_capa(4);
  id_gsub = rb_intern("gsub");

  rb_str_dot_dot = rb_str_new_cstr("..");
  rb_ary_store(ary, 0, rb_str_dot_dot);
  rb_str___parent = rb_str_new_cstr("__parent");
  rb_ary_store(ary, 1, rb_str___parent);
  rb_str_backslash = rb_str_new_cstr("\\");
  rb_ary_store(ary, 2, rb_str_backslash);
  rb_str_dot = rb_str_new_cstr(".");
  rb_ary_store(ary, 3, rb_str_dot);

  cField = rb_define_class_under(mLibBin, "Field", rb_cObject);
  rb_define_alloc_func(cField, cField_alloc);
  rb_const_set(cField, rb_intern("STRINGS"), ary);
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
  (void)data;
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

/*  attr_reader :__parent
    attr_reader :__index
    attr_reader :__iterator
    attr_reader :__position */

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

/*  attr_reader :__input
    attr_reader :__output
    attr_reader :__input_big
    attr_reader :__output_big */

static VALUE cDataConverter_input(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  return data->__input;
}

static VALUE cDataConverter_output(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  return data->__output;
}

static VALUE cDataConverter_input_big(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  return data->__input_big;
}

static VALUE cDataConverter_output_big(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  return data->__output_big;
}

ID id_tell;

/*  def __set_convert_type(input, output, input_big, output_big, parent, index)
      @__input_big = input_big
      @__output_big = output_big
      @__input = input
      @__output = output
      @__parent = parent
      @__index = index
      @__position = input.tell
      @__cur_position = @__position
    end */

static inline VALUE cDataConverter_set_convert_type(
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
  data->__position = rb_funcall(input, id_tell, 0);
  data->__cur_position = data->__position;
  return Qnil;
}

/*  def __unset_convert_type
      @__input_big = nil
      @__output_big = nil
      @__input = nil
      @__output = nil
      @__parent = nil
      @__index = nil
      @__position = nil
      @__cur_position = nil
    end */

static inline VALUE cDataConverter_unset_convert_type(VALUE self) {
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

/*  def __set_size_type(position, parent, index)
      @__parent = parent
      @__index = index
      @__position = position
      @__cur_position = @__position
    end */

static inline VALUE cDataConverter_set_size_type(
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

/*  def __unset_size_type
      @__parent = nil
      @__index = nil
      @__position = nil
      @__cur_position = nil
    end */

static inline VALUE cDataConverter_unset_size_type(VALUE self) {
  struct cDataConverter_data *data;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  data->__parent = Qnil;
  data->__index = Qnil;
  data->__position = Qnil;
  data->__cur_position = Qnil;
  return Qnil;
}

/*  def __set_load_type(input, input_big, parent, index)
      @__input_big = input_big
      @__input = input
      @__parent = parent
      @__index = index
      @__position = input.tell
      @__cur_position = @__position
    end */

static inline VALUE cDataConverter_set_load_type(
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
  data->__position = rb_funcall(input, id_tell, 0);
  data->__cur_position = data->__position;
  return Qnil;
}

/*  def __unset_load_type
      @__input_big = nil
      @__input = nil
      @__parent = nil
      @__index = nil
      @__position = nil
      @__cur_position = nil
    end */

static inline VALUE cDataConverter_unset_load_type(VALUE self) {
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

/*  def __set_dump_type(output, output_big, parent, index)
      @__output_big = output_big
      @__output = output
      @__parent = parent
      @__index = index
      @__position = output.tell
      @__cur_position = @__position
    end */

static inline VALUE cDataConverter_set_dump_type(
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
  data->__position = rb_funcall(output, id_tell, 0);
  data->__cur_position = data->__position;
  return Qnil;
}

/*  def __unset_dump_type
      @__output_big = nil
      @__output = nil
      @__parent = nil
      @__index = nil
      @__position = nil
      @__cur_position = nil
    end */

static inline VALUE cDataConverter_unset_dump_type(VALUE self) {
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

/*  def __decode_expression(sym)
      case sym
      when Proc
        return sym.call
      when String
        exp = sym.gsub("..","__parent").gsub("\\",".")
        return eval(exp)
      else
        return sym
      end
    end */

static ID id_instance_exec;

static inline VALUE cDataConverter_decode_expression(VALUE self, VALUE expression) {
  if (rb_obj_is_proc(expression))
    return rb_funcall_with_block(self, id_instance_exec, 0, NULL, expression);
  else
    return expression;
}

/*  def __decode_seek_offset(offset, relative_offset)
      return nil unless offset
      offset = __decode_expression(offset)
      return false if offset == 0x0
      offset += @__position if relative_offset
      @__cur_position = offset
      @__input.seek(offset) if @__input
      @__output.seek(offset) if @__output
      offset
    end */

ID id_seek;

static inline VALUE cDataConverter_decode_seek_offset(VALUE self, VALUE offset, VALUE relative_offset) {
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
    rb_funcall(data->__input, id_seek, 1, data->__cur_position);
  if (RTEST(data->__output))
    rb_funcall(data->__output, id_seek, 1, data->__cur_position);
  return data->__cur_position;
}

/*  def __decode_condition(condition)
      return true unless condition
      __decode_expression(condition)
    end */

static inline VALUE cDataConverter_decode_condition(VALUE self, VALUE condition) {
  if (!RTEST(condition))
    return Qtrue;
  return cDataConverter_decode_expression(self, condition);
}

/*  def __decode_count(count)
      return 1 unless count
      __decode_expression(count)
    end */

static inline VALUE cDataConverter_decode_count(VALUE self, VALUE count) {
  if (!RTEST(count))
    return INT2FIX(1);
  return cDataConverter_decode_expression(self, count);
}

/*  def __decode_type(type)
      __decode_expression(type)
    end */

static inline VALUE cDataConverter_decode_type(VALUE self, VALUE type) {
  return cDataConverter_decode_expression(self, type);
}

/*  def __decode_length(length)
      __decode_expression(length)
    end */

static inline VALUE cDataConverter_decode_length(VALUE self, VALUE length) {
  if (NIL_P(length))
    return Qnil;
  return cDataConverter_decode_expression(self, length);
}

/*  def __decode_static_conditions(field)
      @__offset = nil
      @__condition = nil
      @__type = nil
      @__length = nil
      @__count = nil
      unless field.sequence?
        @__offset = __decode_seek_offset(field.offset, field.relative_offset?)
        throw :ignored, nil if @__offset == false
        @__condition = __decode_condition(field.condition)
        throw :ignored, nil unless @__condition
        @__type = __decode_type(field.type)
        @__length = __decode_length(field.length)
      end
      @__count = __decode_count(field.count)
    end */

static inline VALUE cDataConverter_decode_static_conditions(VALUE self, VALUE field) {
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
      return Qnil;
    data->__condition = cDataConverter_decode_condition(self, field_data->condition);
    if (!RTEST(data->__condition))
      return Qnil;
    data->__type = cDataConverter_decode_type(self, field_data->type);
    data->__length = cDataConverter_decode_length(self, field_data->length);
  }
  data->__count = cDataConverter_decode_count(self, field_data->count);
  return Qtrue;
}

/*  def __decode_dynamic_conditions(field)
      return true unless field.sequence?
      @__offset = nil
      @__condition = nil
      @__type = nil
      @__length = nil
      @__offset = __decode_seek_offset(field.offset, field.relative_offset?)
      return false if @__offset == false
      @__condition = __decode_condition(field.condition)
      return false unless @__condition
      @__type = __decode_type(field.type)
      @__length = __decode_length(field.length)
      return true
    end */

static inline VALUE cDataConverter_decode_dynamic_conditions(VALUE self, VALUE field) {
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

/*  def __restore_context
      @__iterator = nil
      @__type = nil
      @__length = nil
      @__count = nil
      @__offset = nil
      @__condition = nil
    end */

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

/*  def __convert_field(field)
      __decode_static_conditions(field)
      vs = @__count.times.collect do |it|
        @__iterator = it
        if __decode_dynamic_conditions(field)
          @__type::convert(@__input, @__output, @__input_big, @__output_big, self, it, @__length)
        else
          nil
        end
      end
      __restore_context
      vs = vs.first unless field.count
      vs
    end */

static ID id_convert;

static inline VALUE cDataConverter_convert_field(VALUE self, VALUE field) {
  VALUE res;
  struct cDataConverter_data *data;
  struct cField_data *field_data;
  if (NIL_P(cDataConverter_decode_static_conditions(self, field)))
    return Qnil;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);

  if (RTEST(field_data->count)) {
    long count = NUM2LONG(data->__count);
    res = rb_ary_new_capa(count);

    for (long i = 0; i < count; i++) {
      data->__iterator = LONG2NUM(i);
      if (RTEST(cDataConverter_decode_dynamic_conditions(self, field)))
        rb_ary_store(res, i, rb_funcall(data->__type, id_convert, 7,
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
    data->__iterator = INT2FIX(0);
    if (RTEST(cDataConverter_decode_dynamic_conditions(self, field)))
      res = rb_funcall(data->__type, id_convert, 7,
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

/*  def __load_field(field)
      __decode_static_conditions(field)
      vs = @__count.times.collect do |it|
        @__iterator = it
        if __decode_dynamic_conditions(field)
          @__type::load(@__input, @__input_big, self, it, @__length)
        else
          nil
        end
      end
      __restore_context
      vs = vs.first unless field.count
      vs
    end */

static ID id_load;

static inline VALUE cDataConverter_load_field(VALUE self, VALUE field) {
  VALUE res;
  struct cDataConverter_data *data;
  struct cField_data *field_data;
  if (NIL_P(cDataConverter_decode_static_conditions(self, field)))
    return Qnil;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);

  if (RTEST(field_data->count)) {
    long count = NUM2LONG(data->__count);
    res = rb_ary_new_capa(count);

    for (long i = 0; i < count; i++) {
      data->__iterator = LONG2NUM(i);
      if (RTEST(cDataConverter_decode_dynamic_conditions(self, field)))
        rb_ary_store(res, i, rb_funcall(data->__type, id_load, 5,
          data->__input,
          data->__input_big,
          self,
          data->__iterator,
          data->__length));
      else
        rb_ary_store(res, i, Qnil);
    }
  } else {
    data->__iterator = INT2FIX(0);
    if (RTEST(cDataConverter_decode_dynamic_conditions(self, field)))
      res = rb_funcall(data->__type, id_load, 5,
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

/*  def __dump_field(vs, field)
      __decode_static_conditions(field)
      vs = [vs] unless field.count
      vs.each_with_index do |v, it|
        @__iterator = it
        if __decode_dynamic_conditions(field)
          @__type::dump(v, @__output, @__output_big, self, it, @__length)
        end
      end
      __restore_context
    end */

static ID id_dump;

static inline VALUE cDataConverter_dump_field(VALUE self, VALUE values, VALUE field) {
  struct cDataConverter_data *data;
  struct cField_data *field_data;
  if (NIL_P(cDataConverter_decode_static_conditions(self, field)))
    return Qnil;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);

  if (RTEST(field_data->count)) {
    long count = RARRAY_LEN(values);

    for (long i = 0; i < count; i++) {
      data->__iterator = LONG2NUM(i);
      if (RTEST(cDataConverter_decode_dynamic_conditions(self, field)))
        rb_funcall(data->__type, id_dump, 6,
          rb_ary_entry(values, i),
          data->__output,
          data->__output_big,
          self,
          data->__iterator,
          data->__length);
    }
  } else {
    data->__iterator = INT2FIX(0);
    if (RTEST(cDataConverter_decode_dynamic_conditions(self, field)))
      rb_funcall(data->__type, id_dump, 6,
        values,
        data->__output,
        data->__output_big,
        self,
        data->__iterator,
        data->__length);
  }
  cDataConverter_restore_context(self);
  return Qnil;
}

/*  def __shape_field(vs, kind, field)
      __decode_static_conditions(field)
      vs = [vs] unless field.count
      vs = vs.each_with_index.collect do |v, it|
        @__iterator = it
        if __decode_dynamic_conditions(field)
          sh = @__type::shape(v, @__cur_position, self, it, kind, @__length)
          @__cur_position = sh.last + 1 if sh.last && sh.last >= 0
          sh
        end
      end
      __restore_context
      vs = field.count ? kind.new(vs) : vs.first
      vs
    end */

static ID id_shape;

static inline VALUE cDataConverter_shape_field(
    VALUE self,
    VALUE values,
    VALUE kind,
    VALUE field)
{
  VALUE res = Qnil;
  struct cDataConverter_data *data;
  struct cField_data *field_data;
  if (NIL_P(cDataConverter_decode_static_conditions(self, field)))
    return Qnil;
  TypedData_Get_Struct(self, struct cDataConverter_data, &cDataConverter_type, data);
  TypedData_Get_Struct(field, struct cField_data, &cField_type, field_data);

  if (RTEST(field_data->count)) {
    long count = RARRAY_LEN(values);
    res = rb_ary_new_capa(count);

    for (long i = 0; i < count; i++) {
      data->__iterator = LONG2NUM(i);
      if (RTEST(cDataConverter_decode_dynamic_conditions(self, field))) {
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
    if (RTEST(cDataConverter_decode_dynamic_conditions(self, field))) {
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
  cDataConverter_restore_context(self);
  return res;
}

/*  def __load_fields
      self.class.instance_variable_get(:@fields).each { |field|
        begin
          send("#{field.name}=", __load_field(field))
        rescue
          STDERR.puts "#{self.class}: #{field.name}(#{field.type})"
          raise
        end
      }
      self
    end */

static ID id_fields;

struct fields_state {
  VALUE self;
  VALUE fields;
  VALUE field;
};

static inline VALUE cDataConverter_fields_rescue(VALUE state_p, VALUE exception) {
  struct fields_state *state = (struct fields_state *)state_p;
  if (NIL_P(state->field)) {
    struct cDataConverter_data *data;
    TypedData_Get_Struct(state->self, struct cDataConverter_data, &cDataConverter_type, data);
    if (!NIL_P(rb_ivar_get(mLibBin, rb_intern("@__output"))))
      rb_funcall(rb_ivar_get(mLibBin, rb_intern("@__output")), rb_intern("print"), 6,
        rb_obj_class(state->self),
        rb_str_new_cstr(": could not load fields, index: "),
        data->__index,
        rb_str_new_cstr(", current position: "),
        data->__cur_position,
        rb_str_new_cstr("\n"));
  } else {
    struct cField_data *field_data;
    TypedData_Get_Struct(state->field, struct cField_data, &cField_type, field_data);
    if (!NIL_P(rb_ivar_get(mLibBin, rb_intern("@__output"))))
      rb_funcall(rb_ivar_get(mLibBin, rb_intern("@__output")), rb_intern("print"), 6,
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

static inline VALUE cDataConverter_load_fields_wrapper(VALUE state_p) {
  struct fields_state *state = (struct fields_state *)state_p;
  state->fields = rb_ivar_get(rb_obj_class(state->self), id_fields);
  for (long i = 0; i < RARRAY_LEN(state->fields); i++) {
    state->field = rb_ary_entry(state->fields, i);
    struct cField_data *field_data;
    TypedData_Get_Struct(state->field, struct cField_data, &cField_type, field_data);
    rb_funcall(state->self, field_data->setter, 1, cDataConverter_load_field(state->self, state->field));
  }
  return state->self;
}

static ID id___load_fields;

static inline VALUE cDataConverter_load_fields(VALUE self) {
  struct fields_state state = {self, Qnil, Qnil};
  rb_rescue(&cDataConverter_load_fields_wrapper, (VALUE)&state,
            &cDataConverter_fields_rescue, (VALUE)&state);
  return self;
}

/*  def __dump_fields
      self.class.instance_variable_get(:@fields).each { |field|
        begin
          __dump_field(send(field.name), field)
        rescue
          STDERR.puts "#{self.class}: #{field.name}(#{field.type})"
          raise
        end
      }
      self
    end */

static inline VALUE cDataConverter_dump_fields_wrapper(VALUE state_p) {
  struct fields_state *state = (struct fields_state *)state_p;
  state->fields = rb_ivar_get(rb_obj_class(state->self), id_fields);
  for (long i = 0; i < RARRAY_LEN(state->fields); i++) {
    state->field = rb_ary_entry(state->fields, i);
    struct cField_data *field_data;
    TypedData_Get_Struct(state->field, struct cField_data, &cField_type, field_data);
    cDataConverter_dump_field(state->self, rb_funcall(state->self, field_data->getter, 0), state->field);
  }
  return state->self;
}

static ID id___dump_fields;

static inline VALUE cDataConverter_dump_fields(VALUE self) {
  struct fields_state state = {self, Qnil, Qnil};
  rb_rescue(&cDataConverter_dump_fields_wrapper, (VALUE)&state,
            &cDataConverter_fields_rescue, (VALUE)&state);
  return self;
}

/*  def __convert_fields
      self.class.instance_variable_get(:@fields).each { |field|
        begin
          send("#{field.name}=", __convert_field(field))
        rescue
          STDERR.puts "#{self.class}: #{field.name}(#{field.type})"
          raise
        end
      }
      self
    end */

static inline VALUE cDataConverter_convert_fields_wrapper(VALUE state_p) {
  struct fields_state *state = (struct fields_state *)state_p;
  state->fields = rb_ivar_get(rb_obj_class(state->self), id_fields);
  for (long i = 0; i < RARRAY_LEN(state->fields); i++) {
    state->field = rb_ary_entry(state->fields, i);
    struct cField_data *field_data;
    TypedData_Get_Struct(state->field, struct cField_data, &cField_type, field_data);
    rb_funcall(state->self, field_data->setter, 1, cDataConverter_convert_field(state->self, state->field));
  }
  return state->self;
}

static ID id___convert_fields;

static inline VALUE cDataConverter_convert_fields(VALUE self) {
  struct fields_state state = {self, Qnil, Qnil};
  rb_rescue(&cDataConverter_convert_fields_wrapper, (VALUE)&state,
            &cDataConverter_fields_rescue, (VALUE)&state);
  return self;
}

/*  def __shape_fields
      members = {}
      self.class.instance_variable_get(:@fields).each { |field|
        begin
          members[field.name] = __shape_field(send(field.name), kind, field)
        rescue
          STDERR.puts "#{self.class}: #{field.name}(#{field.type})"
          raise
        end
      }
      return members
    end */

struct shape_fields_state {
  VALUE self;
  VALUE fields;
  VALUE field;
  VALUE kind;
};

static inline VALUE cDataConverter_shape_fields_wrapper(VALUE state_p) {
  struct shape_fields_state *state = (struct shape_fields_state *)state_p;
  state->fields = rb_ivar_get(rb_obj_class(state->self), id_fields);
  VALUE members = rb_hash_new();
  for (long i = 0; i < RARRAY_LEN(state->fields); i++) {
    state->field = rb_ary_entry(state->fields, i);
    struct cField_data *field_data;
    TypedData_Get_Struct(state->field, struct cField_data, &cField_type, field_data);
    rb_hash_aset(members, ID2SYM(field_data->getter), cDataConverter_shape_field(state->self, rb_funcall(state->self, field_data->getter, 0), state->kind, state->field));
  }
  return members;
}

static ID id___shape_fields;

static inline VALUE cDataConverter_shape_fields(VALUE self, VALUE kind) {
  struct shape_fields_state state = {self, Qnil, Qnil, kind};
  return rb_rescue(&cDataConverter_shape_fields_wrapper, (VALUE)&state,
                   &cDataConverter_fields_rescue, (VALUE)&state);
}

/* def __load(input, input_big, parent = nil, index = nil)
      __set_load_type(input, input_big, parent, index)
      __load_fields
      __unset_load_type
      self
    end */

static ID id___load;

static inline VALUE cDataConverter_load(int argc, VALUE *argv, VALUE self) {
  VALUE input;
  VALUE input_big;
  VALUE parent;
  VALUE index;
  rb_scan_args(argc, argv, "22", &input, &input_big, &parent, &index);
  cDataConverter_set_load_type(self, input, input_big, parent, index);
  rb_funcall(self, id___load_fields, 0);
  cDataConverter_unset_load_type(self);
  return self;
}

/* def __dump(output, output_big, parent = nil, index = nil)
      __set_dump_type(output, output_big, parent, index)
      __dump_fields
      __unset_dump_type
      self
    end */

static ID id___dump;

static inline VALUE cDataConverter_dump(int argc, VALUE *argv, VALUE self) {
  VALUE output;
  VALUE output_big;
  VALUE parent;
  VALUE index;
  rb_scan_args(argc, argv, "22", &output, &output_big, &parent, &index);
  cDataConverter_set_dump_type(self, output, output_big, parent, index);
  rb_funcall(self, id___dump_fields, 0);
  cDataConverter_unset_dump_type(self);
  return self;
}

/*  def __convert(input, output, input_big, output_big, parent = nil, index = nil)
      __set_convert_type(input, output, input_big, output_big, parent, index)
      __convert_fields
      __unset_convert_type
      self
    end */

static ID id___convert;

static inline VALUE cDataConverter_convert(int argc, VALUE *argv, VALUE self) {
  VALUE input;
  VALUE output;
  VALUE input_big;
  VALUE output_big;
  VALUE parent;
  VALUE index;
  rb_scan_args(argc, argv, "42", &input, &output, &input_big, &output_big, &parent, &index);
  cDataConverter_set_convert_type(self, input, output, input_big, output_big, parent, index);
  rb_funcall(self, id___convert_fields, 0);
  cDataConverter_unset_convert_type(self);
  return self;
}

/*  def __shape(previous_offset = 0, parent = nil, index = nil, kind = DataShape)
      __set_size_type(previous_offset, parent, index)
      members = __shape_fields(kind)
      __unset_size_type
      return nil if members.values.compact.size <= 0
      kind::new(members)
    end */

static ID id___shape;

static inline VALUE cDataConverter_shape(int argc, VALUE *argv, VALUE self) {
  VALUE previous_offset;
  VALUE parent;
  VALUE index;
  VALUE kind;
  rb_scan_args(argc, argv, "04", &previous_offset, &parent, &index, &kind);
  if (NIL_P(previous_offset))
    previous_offset = INT2FIX(0);
  if (NIL_P(kind))
    kind = cDataShape;
  cDataConverter_set_size_type(self, previous_offset, parent, index);
  VALUE members = rb_funcall(self, id___shape_fields, 1, kind);
  cDataConverter_unset_size_type(self);
  if (RARRAY_LEN(rb_funcall(rb_funcall(members, rb_intern("values"), 0), rb_intern("compact"), 0)) <= 0)
    return Qnil;
  return rb_class_new_instance(1, &members, kind);
}

/*  def self.load(input, input_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
      if length
        length.times.collect {
          h = self::new
          h.__load(input, input_big, parent, index)
        }
      else
        h = self::new
        h.__load(input, input_big, parent, index)
      end
    end */

static inline VALUE cDataConverter_singl_load(int argc, VALUE *argv, VALUE self) {
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

/*  def self.dump(value, output, output_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
      if length
        length.times.each { |i|
          value[i].__dump(output, output_big, parent, index)
        }
        value
      else
        value.__dump(output, output_big, parent, index)
      end
    end */

static inline VALUE cDataConverter_singl_dump(int argc, VALUE *argv, VALUE self) {
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

/*  def self.convert(input, output, input_big = LibBin::default_big?, output_big = !input_big, parent = nil, index = nil, length = nil)
      if length
        length.times.collect {
          h = self::new
          h.__convert(input, output, input_big, output_big, parent, index)
        }
      else
        h = self::new
        h.__convert(input, output, input_big, output_big, parent, index)
      end
    end */

static inline VALUE cDataConverter_singl_convert(int argc, VALUE *argv, VALUE self) {
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

/*  def self.shape(value, previous_offset = 0, parent = nil, index = nil, kind = DataShape, length = nil)
      if length
        kind::new(length.times.collect { |i|
          value[i].__shape(previous_offset, parent, index, kind)
        })
      else
        value.__shape(previous_offset, parent, index, kind)
      end
    end */

static inline VALUE cDataConverter_singl_shape(int argc, VALUE *argv, VALUE self) {
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

static void define_cDataConverter() {
  id_tell = rb_intern("tell");
  id_seek = rb_intern("seek");
  id_fields = rb_intern("@fields");
  id_instance_exec = rb_intern("instance_exec");

  id___load_fields = rb_intern("__load_fields");
  id_load = rb_intern("load");
  id___load = rb_intern("__load");

  id___dump_fields = rb_intern("__dump_fields");
  id_dump = rb_intern("dump");
  id___dump = rb_intern("__dump");

  id___convert_fields = rb_intern("__convert_fields");
  id_convert = rb_intern("convert");
  id___convert = rb_intern("__convert");

  id___shape_fields = rb_intern("__shape_fields");
  id_shape = rb_intern("shape");
  id___shape = rb_intern("__shape");

  cDataConverter = rb_define_class_under(mLibBin, "DataConverter", rb_cObject);
  rb_define_alloc_func(cDataConverter, cDataConverter_alloc);
  rb_define_method(cDataConverter, "initialize", cDataConverter_initialize, 0);
  rb_define_method(cDataConverter, "__parent", cDataConverter_parent, 0);
  rb_define_method(cDataConverter, "__index", cDataConverter_index, 0);
  rb_define_method(cDataConverter, "__iterator", cDataConverter_iterator, 0);
  rb_define_method(cDataConverter, "__position", cDataConverter_position, 0);
  rb_define_method(cDataConverter, "__input", cDataConverter_input, 0);
  rb_define_method(cDataConverter, "__output", cDataConverter_output, 0);
  rb_define_method(cDataConverter, "__input_big", cDataConverter_input_big, 0);
  rb_define_method(cDataConverter, "__output_big", cDataConverter_output_big, 0);

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

  rb_define_method(cDataConverter, "__load_fields", cDataConverter_load_fields, 0);
  rb_define_method(cDataConverter, "__dump_fields", cDataConverter_dump_fields, 0);
  rb_define_method(cDataConverter, "__convert_fields", cDataConverter_convert_fields, 0);
  rb_define_method(cDataConverter, "__shape_fields", cDataConverter_shape_fields, 1);

  rb_define_method(cDataConverter, "__load", cDataConverter_load, -1);
  rb_define_method(cDataConverter, "__dump", cDataConverter_dump, -1);
  rb_define_method(cDataConverter, "__convert", cDataConverter_convert, -1);
  rb_define_method(cDataConverter, "__shape", cDataConverter_shape, -1);

  rb_define_singleton_method(cDataConverter, "load", cDataConverter_singl_load, -1);
  rb_define_singleton_method(cDataConverter, "dump", cDataConverter_singl_dump, -1);
  rb_define_singleton_method(cDataConverter, "convert", cDataConverter_singl_convert, -1);
  rb_define_singleton_method(cDataConverter, "shape", cDataConverter_singl_shape, -1);
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
