module LibBin
end
require "libbin_c.so"

require_relative 'libbin/alignment'
require_relative 'libbin/data_types'

module LibBin

  @__big_architecture = [0x12345678].pack("i") == "\x12\x34\x56\x78"
  @__big = nil

  def self.default_big?
    if @__big.nil?
      @__big_architecture
    else
      @__big
    end
  end

  class DataConverter
    include Alignment

    def inspect
      to_s
    end

    attr_reader :__parent
    attr_reader :__index
    attr_reader :__iterator
    attr_reader :__position
    def __set_convert_type(input, output, input_big, output_big, parent, index)
      @__input_big = input_big
      @__output_big = output_big
      @__input = input
      @__output = output
      @__parent = parent
      @__index = index
      @__position = input.tell
      @__cur_position = @__position
    end

    def __set_size_type(position, parent, index)
      @__parent = parent
      @__index = index
      @__position = position
      @__cur_position = @__position
    end

    def __set_load_type(input, input_big, parent, index)
      @__input_big = input_big
      @__input = input
      @__parent = parent
      @__index = index
      @__position = input.tell
      @__cur_position = @__position
    end

    def __set_dump_type(output, output_big, parent, index)
      @__output_big = output_big
      @__output = output
      @__parent = parent
      @__index = index
      @__position = output.tell
      @__cur_position = @__position
    end

    def __unset_convert_type
      @__input_big = nil
      @__output_big = nil
      @__input = nil
      @__output = nil
      @__parent = nil
      @__index = nil
      @__position = nil
      @__cur_position = nil
    end

    def __unset_size_type
      @__parent = nil
      @__index = nil
      @__position = nil
      @__cur_position = nil
    end

    def __unset_load_type
      @__input_big = nil
      @__input = nil
      @__parent = nil
      @__index = nil
      @__position = nil
      @__cur_position = nil
    end

    def __unset_dump_type
      @__output_big = nil
      @__output = nil
      @__parent = nil
      @__index = nil
      @__position = nil
      @__cur_position = nil
    end

    def self.inherited(subclass)
      subclass.instance_variable_set(:@fields, [])
    end

    def __decode_expression(sym)
      case sym
      when Proc
        return sym.call
      when String
        exp = sym.gsub("..","__parent").gsub("\\",".")
        return eval(exp)
      else
        return sym
      end
    end

    def __decode_seek_offset(offset, relative_offset)
      return nil unless offset
      offset = __decode_expression(offset)
      return false if offset == 0x0
      offset += @__position if relative_offset
      @__cur_position = offset
      @__input.seek(offset) if @__input
      @__output.seek(offset) if @__output
      offset
    end

    def __decode_condition(condition)
      return true unless condition
      __decode_expression(condition)
    end

    def __decode_count(count)
      return 1 unless count
      __decode_expression(count)
    end

    def __decode_type(type)
      return __decode_expression(type)
    end

    def __decode_length(length)
      __decode_expression(length)
    end

    def __decode_static_conditions(field)
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
    end

    def __decode_dynamic_conditions(field)
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
    end

    def __restore_context
      @__iterator = nil
      @__type = nil
      @__length = nil
      @__count = nil
      @__offset = nil
      @__condition = nil
    end

    def __convert_field(field)
      instance_exec(field, &field.decode_static_conditions)
      vs = @__count.times.collect do |it|
        @__iterator = it
        if instance_exec(field, &field.decode_dynamic_conditions)
          @__type::convert(@__input, @__output, @__input_big, @__output_big, self, it, @__length)
        else
          nil
        end
      end
      @__iterator = nil
      @__type = nil
      @__length = nil
      @__count = nil
      @__offset = nil
      @__condition = nil
      vs = vs.first unless field.count
      vs
    end

    def __load_field(field)
      instance_exec(field, &field.decode_static_conditions)
      vs = @__count.times.collect do |it|
        @__iterator = it
        if instance_exec(field, &field.decode_dynamic_conditions)
          @__type::load(@__input, @__input_big, self, it, @__length)
        else
          nil
        end
      end
      @__iterator = nil
      @__type = nil
      @__length = nil
      @__count = nil
      @__offset = nil
      @__condition = nil
      vs = vs.first unless field.count
      vs
    end

    def __dump_field(vs, field)
      instance_exec(field, &field.decode_static_conditions)
      vs = [vs] unless field.count
      vs.each_with_index do |v, it|
        @__iterator = it
        if instance_exec(field, &field.decode_dynamic_conditions)
          @__type::dump(v, @__output, @__output_big, self, it, @__length)
        end
      end
      @__iterator = nil
      @__type = nil
      @__length = nil
      @__count = nil
      @__offset = nil
      @__condition = nil
    end

    def __shape_field(vs, previous_offset, kind, field)
      instance_exec(field, &field.decode_static_conditions)
      vs = [vs] unless field.count
      vs = vs.each_with_index.collect do |v, it|
        @__iterator = it
        if instance_exec(field, &field.decode_dynamic_conditions)
          sh = @__type::shape(v, @__cur_position, self, it, kind, @__length)
          @__cur_position = sh.last + 1 if sh.last && sh.last >= 0
          sh
        end
      end
      @__iterator = nil
      @__type = nil
      @__length = nil
      @__count = nil
      @__offset = nil
      @__condition = nil
      vs = vs.first unless field.count
      vs
    end

    def __size(previous_offset = 0, parent = nil, index = nil)
      __shape(previous_offset, parent, index, DataRange).size
    end

    def __shape(previous_offset = 0, parent = nil, index = nil, kind = DataShape)
      __set_size_type(previous_offset, parent, index)
      members = {}
      self.class.instance_variable_get(:@fields).each { |field|
        begin
          vs = send(field.name)
          member = catch(:ignored) do
            __shape_field(vs, previous_offset, kind, field)
          end
          members[field.name] = member
        rescue
          STDERR.puts "#{self.class}: #{field.name}(#{field.type})"
          raise
        end
      }
      __unset_size_type
      return nil if members.values.flatten.compact.size <= 0
      kind::new(members)
    end

    def __convert_fields
      self.class.instance_variable_get(:@fields).each { |field|
        begin
          vs = catch(:ignored) do
            __convert_field(field)
          end
          send("#{field.name}=", vs)
        rescue
          STDERR.puts "#{self.class}: #{field.name}(#{field.type})"
          raise
        end
      }
      self
    end

    def __load_fields
      self.class.instance_variable_get(:@fields).each { |field|
        begin
          vs = catch(:ignored) do
            __load_field(field)
          end
          send("#{field.name}=", vs)
        rescue
          STDERR.puts "#{self.class}: #{field.name}(#{field.type})"
          raise
        end
      }
      self
    end

    def __dump_fields
      self.class.instance_variable_get(:@fields).each { |field|
        begin
          vs = send(field.name)
          catch(:ignored) do
            __dump_field(vs, field)
          end
        rescue
          STDERR.puts "#{self.class}: #{field.name}(#{field.type})"
          raise
        end
      }
      self
    end

    def __convert(input, output, input_big, output_big, parent = nil, index = nil)
      __set_convert_type(input, output, input_big, output_big, parent, index)
      __convert_fields
      __unset_convert_type
      self
    end

    def __load(input, input_big, parent = nil, index = nil)
      __set_load_type(input, input_big, parent, index)
      __load_fields
      __unset_load_type
      self
    end

    def __dump(output, output_big, parent = nil, index = nil)
      __set_dump_type(output, output_big, parent, index)
      __dump_fields
      __unset_dump_type
      self
    end
      
    def self.convert(input, output, input_big = LibBin::default_big?, output_big = !LibBin::default_big?, parent = nil, index = nil, length = nil)
      if length
        length.times.collect {
          h = self::new
          h.__load(input, input_big, parent, index)
        }
      else
        h = self::new
        h.__convert(input, output, input_big, output_big, parent, index)
      end
    end

    def self.load(input, input_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
      if length
        length.times.collect {
          h = self::new
          h.__load(input, input_big, parent, index)
        }
      else
        h = self::new
        h.__load(input, input_big, parent, index)
      end
    end

    def self.dump(value, output, output_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
      if length
        length.times.collect { |i|
          value[i].__dump(output, output_big, parent, index)
        }
        value
      else
        value.__dump(output, output_big, parent, index)
      end
    end

    def self.size(value, previous_offset = 0, parent = nil, index = nil, length = nil)
      if length
        shape(value, previous_offset, parent, index, length).size
      else
        value.__shape(previous_offset, parent, index).size
      end
    end

    def self.shape(value, previous_offset = 0, parent = nil, index = nil, kind = DataShape, length = nil)
      if length
        kind::new(length.times.collect { |i|
          value[i].__shape(previous_offset, parent, index, kind)
        })
      else
        value.__shape(previous_offset, parent, index, kind)
      end
    end

  end

end
