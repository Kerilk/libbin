warn_level = $VERBOSE
$VERBOSE = nil
require 'float-formats'
$VERBOSE = warn_level

Flt::IEEE.binary :IEEE_binary16_pg, significand: 9, exponent: 6, bias: 47
Flt::IEEE.binary :IEEE_binary16_pg_BE, significand: 9, exponent: 6, bias: 47, endianness: :big_endian

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
    def set_convert_type(input, output, input_big, output_big, parent, index)
      @input_big = input_big
      @output_big = output_big
      @input = input
      @output = output
      @__parent = parent
      @__index = index
      @__position = input.tell
    end

    def set_size_type(position, parent, index)
      @__parent = parent
      @__index = index
      @__position = position
    end

    def set_load_type(input, input_big, parent, index)
      @input_big = input_big
      @input = input
      @__parent = parent
      @__index = index
      @__position = input.tell
    end

    def set_dump_type(output, output_big, parent, index)
      @output_big = output_big
      @output = output
      @__parent = parent
      @__index = index
      @__position = output.tell
    end

    def unset_convert_type
      @input_big = nil
      @output_big = nil
      @input = nil
      @output = nil
      @__parent = nil
      @__index = nil
      @__position = nil
    end

    def unset_size_type
      @__parent = nil
      @__index = nil
      @__position = nil
    end

    def unset_load_type
      @input_big = nil
      @input = nil
      @__parent = nil
      @__index = nil
      @__position = nil
    end

    def unset_dump_type
      @output_big = nil
      @output = nil
      @__parent = nil
      @__index = nil
      @__position = nil
    end

    def self.inherited(subclass)
      subclass.instance_variable_set(:@fields, [])
    end

    def decode_expression(sym)
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

    def decode_seek_offset(offset)
      if offset
        offset = decode_expression(offset)
        return false if offset == 0x0
        @input.seek(offset) if @input
        @output.seek(offset) if @output
        return offset
      else
        return nil
      end
    end

    def decode_condition(condition)
      return true unless condition
      return decode_expression(condition)
    end

    def decode_count(count)
      if count
        return decode_expression(count)
      else
        return 1
      end
    end

    def decode_type(type)
      return decode_expression(type)
    end

    def decode_static_conditions(type, count, offset, sequence, condition)
      @__offset = nil
      @__condition = nil
      @__type = nil
      @__count = nil
      unless sequence
        @__offset = decode_seek_offset(offset)
        throw :ignored, nil if @__offset == false
        @__condition = decode_condition(condition)
        throw :ignored, nil unless @__condition
        @__type = decode_type(type)
      end
      @__count = decode_count(count)
    end

    def decode_dynamic_conditions(type, offset, sequence, condition)
      return true unless sequence
      @__offset = nil
      @__condition = nil
      @__type = nil
      @__offset = decode_seek_offset(offset)
      return false if @__offset == false
      @__condition = decode_condition(condition)
      return false unless @__condition
      @__type = decode_type(type)
      return true
    end

    def restore_context
      @__iterator = nil
      @__type = nil
      @__count = nil
      @__offset = nil
      @__condition = nil
    end

    def convert_field(field, type, count, offset, sequence, condition)
      decode_static_conditions(type, count, offset, sequence, condition)
      vs = @__count.times.collect do |it|
        @__iterator = it
        if decode_dynamic_conditions(type, offset, sequence, condition)
          @__type::convert(@input, @output, @input_big, @output_big, self, it)
        else
          nil
        end
      end
      restore_context
      vs = vs.first unless count
      vs
    end

    def load_field(field, type, count, offset, sequence, condition)
      decode_static_conditions(type, count, offset, sequence, condition)
      vs = @__count.times.collect do |it|
        @__iterator = it
        if decode_dynamic_conditions(type, offset, sequence, condition)
          @__type::load(@input, @input_big, self, it)
        else
          nil
        end
      end
      restore_context
      vs = vs.first unless count
      vs
    end

    def dump_field(vs, field, type, count, offset, sequence, condition)
      decode_static_conditions(type, count, offset, sequence, condition)
      vs = [vs] unless count
      vs.each_with_index do |v, it|
        @__iterator = it
        if decode_dynamic_conditions(type, offset, sequence, condition)
          @__type::dump(v, @output, @output_big, self, it)
        end
      end
      restore_context
    end

    def range_field(previous_offset, field, type, count, offset, sequence, condition)
      decode_static_conditions(type, count, offset, sequence, condition)
      vs = send("#{field}")

      if @__offset
        start_offset = @__offset
        end_offset = @__offset
      else previous_offset
        start_offset = previous_offset
        end_offset = previous_offset
      end

      s_offset = start_offset
      e_offset = end_offset
      vs = [vs] unless count
      vs.each_with_index { |v, it|
        @__iterator = it
        if sequence
          decode_dynamic_conditions(type, offset, sequence, condition)
          if @__offset == false || !@__condition
            next
          else
            @__type = decode_type(type)
            if @__offset
              s_offset = @__offset
            else
              s_offset = e_offset
            end
            e_offset = s_offset + @__type::size(v, s_offset, self, it)
            start_offset = s_offset if s_offset < start_offset
            end_offset = e_offset if e_offset > end_offset
          end
        else
          end_offset += @__type::size(v, end_offset, self, it)
        end
      }
      restore_context
      return [start_offset, end_offset]
    end

    def size(previous_offset = 0, parent = nil, index = nil)
      set_size_type(previous_offset, parent, index)
      first_offset = Float::INFINITY
      last_offset = -1
      size = 0
      self.class.instance_variable_get(:@fields).each { |args|
        res = catch(:ignored) do
          range_field(previous_offset, *args)
        end
        next unless res
        start_offset, end_offset = res
        first_offset = start_offset if start_offset && start_offset < first_offset
        last_offset = end_offset if end_offset && end_offset > last_offset
        previous_offset = end_offset if end_offset
      }
      unset_size_type
      return last_offset - first_offset
    end

    def convert_fields
      self.class.instance_variable_get(:@fields).each { |args|
        begin
          vs = catch(:ignored) do
            convert_field(*args)
          end
          send("#{args[0]}=", vs)
        rescue
          STDERR.puts "#{self.class}: #{args[0]}(#{args[1]})"
          raise
        end
      }
      self
    end

    def load_fields
      self.class.instance_variable_get(:@fields).each { |args|
        begin
          vs = catch(:ignored) do
            load_field(*args)
          end
          send("#{args[0]}=", vs)
        rescue
          STDERR.puts "#{self.class}: #{args[0]}(#{args[1]})"
          raise
        end
      }
      self
    end

    def dump_fields
      self.class.instance_variable_get(:@fields).each { |args|
        begin
          vs = send("#{args[0]}")
          catch(:ignored) do
            dump_field(vs, *args)
          end
        rescue
          STDERR.puts "#{self.class}: #{args[0]}(#{args[1]})"
          raise
        end
      }
      self
    end

    def convert(input, output, input_big, output_big, parent = nil, index = nil)
      set_convert_type(input, output, input_big, output_big, parent, index)
      convert_fields
      unset_convert_type
      self
    end

    def load(input, input_big, parent = nil, index = nil)
      set_load_type(input, input_big, parent, index)
      load_fields
      unset_load_type
      self
    end

    def dump(output, output_big, parent = nil, index = nil)
      set_dump_type(output, output_big, parent, index)
      dump_fields
      unset_dump_type
      self
    end
      
    def self.convert(input, output, input_big = LibBin::default_big?, output_big = !LibBin::default_big?, parent = nil, index = nil)
      h = self::new
      h.convert(input, output, input_big, output_big, parent, index)
      h
    end

    def self.load(input, input_big = LibBin::default_big?, parent = nil, index = nil)
      h = self::new
      h.load(input, input_big, parent, index)
      h
    end

    def self.dump(value, output, output_big = LibBin::default_big?, parent = nil, index = nil)
      value.dump(output, output_big, parent, index)
    end

    def self.size(value, previous_offset = 0, parent = nil, index = nil)
      value.size(previous_offset, parent, index)
    end

  end

end
