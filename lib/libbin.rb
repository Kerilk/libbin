def silence_warnings(&block)
  warn_level = $VERBOSE
  $VERBOSE = nil
  result = block.call
  $VERBOSE = warn_level
  result
end

silence_warnings { require 'float-formats' }

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

    def convert_data_field(field, type, count, offset, sequence, condition)
      unless sequence
        off = decode_seek_offset(offset)
        return nil if off == false
        cond = decode_condition(condition)
        return nil unless cond
        typ = decode_type(type)
      end
      c = decode_count(count)
      vs = c.times.collect { |it|
        @__iterator = it
        if sequence
          off = decode_seek_offset(offset)
          cond = decode_condition(condition)
          if off == false || !cond
            nil
          else
            typ = decode_type(type)
            typ::convert(@input, @output, @input_big, @output_big, self, it)
          end
        else
          typ::convert(@input, @output, @input_big, @output_big, self, it)
        end
      }
      @__iterator = nil
      vs = vs.first unless count
      vs
    end

    def load_field(field, type, count, offset, sequence, condition)
      unless sequence
        off = decode_seek_offset(offset)
        return nil if off == false
        cond = decode_condition(condition)
        return nil unless cond
        typ = decode_type(type)
        unless typ.kind_of?(Class)
          rl, _ = DATA_ENDIAN[@input_big][typ]
          sz = DATA_SIZES[typ]
        else
          rl = nil
          sz = nil
        end
      end
      c = decode_count(count)

      vs = c.times.collect do |it|
        @__iterator = it
        if sequence
          off = decode_seek_offset(offset)
          cond = decode_condition(condition)
          if off == false || !cond
            nil
          else
            typ = decode_type(type)
            if typ.kind_of?(Class)
              typ::load(@input, @input_big, self, it)
            else
              rl, _ = DATA_ENDIAN[@input_big][typ]
              sz = DATA_SIZES[typ]
              s = (sz < 0 ? @input.readline("\x00") : @input.read(sz) )
              rl[s]
            end
          end
        elsif rl
          s = (sz < 0 ? @input.readline("\x00") : @input.read(sz) )
          rl[s]
        else
          typ::load(@input, @input_big, self, it)
        end
      end
      @__iterator = nil
      vs = vs.first unless count
      send("#{field}=", vs)
    end

    def dump_data_field(vs, field, type, count, offset, sequence, condition)
      unless sequence
        off = decode_seek_offset(offset)
        return nil if off == false
        cond = decode_condition(condition)
      end
      c = decode_count(count)
      vs = [vs] unless count
      vs.each_with_index { |v, it|
        @__iterator = it
        if sequence
          off = decode_seek_offset(offset)
          cond = decode_condition(condition)
          if off == false || !cond
            nil
          else
            v.dump(@output, @output_big, self, it)
          end
        else
          v.dump(@output, @output_big, self, it)
        end
      }
      @__iterator = nil
    end

    def convert_scalar_field(field, type, count, offset, sequence, condition)
      unless sequence
        off = decode_seek_offset(offset)
        return nil if off == false
        cond = decode_condition(condition)
        return nil unless cond
      end

      c = decode_count(count)
      rl, sl = DATA_ENDIAN[@input_big][type]
      vs = c.times.collect { |it|
        @__iterator = it
        if sequence
          off = decode_seek_offset(offset)
          cond = decode_condition(condition)
          if off == false || !cond
            nil
          else
            sz = DATA_SIZES[type]
            s = (sz < 0 ? @input.readline("\x00") : @input.read(sz) )
            v = rl[s]
            s.reverse! if @input_big != @output_big && type[0] != 'a'
            @output.write(s)
            v
          end
        else
          sz = DATA_SIZES[type]
          s = (sz < 0 ? @input.readline("\x00") : @input.read(sz) )
          v = rl[s]
          s.reverse! if @input_big != @output_big && type[0] != 'a'
          @output.write(s)
          v
        end
      }
      @__iterator = nil
      vs = vs.first unless count
      vs
    end

    def dump_scalar_field(vs, field, type, count, offset, sequence, condition)
      unless sequence
        off = decode_seek_offset(offset)
        return nil if off == false
        cond = decode_condition(condition)
        return nil unless cond
      end

      c = decode_count(count)
      rl, sl = DATA_ENDIAN[@output_big][type]
      vs = [vs] unless count
      vs.each_with_index { |v, it|
        @__iterator = it
        if sequence
          off = decode_seek_offset(offset)
          cond = decode_condition(condition)
          if off == false || !cond
            nil
          else
            @output.write(sl[v])
          end
        else
          @output.write(sl[v])
        end
      }
      @__iterator = nil
    end

    def range_scalar_field(previous_offset, field, type, count, offset, sequence, condition)
      off = nil
      unless sequence
        off = decode_seek_offset(offset)
        return [nil, nil] if off == false
        cond = decode_condition(condition)
        return [nil, nil] unless cond
      end
      if off
        start_offset = off
        end_offset = off
      else previous_offset
        start_offset = previous_offset
        end_offset = previous_offset
      end

      c = decode_count(count)
      s_offset = start_offset
      e_offset = end_offset
      c.times { |it|
        @__iterator = it
        if sequence
          off = decode_seek_offset(offset)
          cond = decode_condition(condition)
          if off == false || !cond
            next
          else
            if off
              s_offset = off
            else
              s_offset = e_offset
            end
            e_offset = s_offset + DATA_SIZES[type]
            start_offset = s_offset if s_offset < start_offset
            end_offset = e_offset if e_offset > end_offset
          end
        else
          end_offset += DATA_SIZES[type]
        end
      }
      @__iterator = nil
      return [start_offset, end_offset]
    end

    def range_data_field(previous_offset, vs, field, type, count, offset, sequence, condition)
      off = nil
      unless sequence
        off = decode_seek_offset(offset)
        return [nil, nil] if off == false
        cond = decode_condition(condition)
        return [nil, nil] unless cond
      end
      if off
        start_offset = off
        end_offset = off
      else previous_offset
        start_offset = previous_offset
        end_offset = previous_offset
      end

      c = decode_count(count)
      s_offset = start_offset
      e_offset = end_offset
      vs = [vs] unless count
      vs.each_with_index { |v, it|
        @__iterator = it
        if sequence
          off = decode_seek_offset(offset)
          cond = decode_condition(condition)
          if off == false || !cond
            next
          else
            if off
              s_offset = off
            else
              s_offset = e_offset
            end
            e_offset = s_offset + v.size(s_offset, self, it)
            start_offset = s_offset if s_offset < start_offset
            end_offset = e_offset if e_offset > end_offset
          end
        else
          end_offset += v.size(end_offset, self, it)
        end
      }
      @__iterator = nil
      return [start_offset, end_offset]
    end

    def convert_field(*args)
      field = args[0]
      type = args[1]
      if ( type.kind_of?(Class) && type < DataConverter ) || type.kind_of?(String)
        vs = convert_data_field(*args)
      elsif type.kind_of?(Symbol)
        vs = convert_scalar_field(*args)
      else
        raise "Unsupported type: #{type.inspect}!"
      end
      send("#{field}=", vs)
    end

    def dump_field(*args)
      field = args[0]
      type = args[1]
      vs = send("#{field}")
      if ( type.kind_of?(Class) && type < DataConverter ) || type.kind_of?(String)
        s = dump_data_field(vs, *args)
      elsif type.kind_of?(Symbol)
        s = dump_scalar_field(vs, *args)
      else
        raise "Unsupported type: #{type.inspect}!"
      end
    end

    def range_field(previous_offset, *args)
      field = args[0]
      type = args[1]
      vs = send("#{field}")
      if ( type.kind_of?(Class) && type < DataConverter ) || type.kind_of?(String)
        range_data_field(previous_offset, vs, *args)
      elsif type.kind_of?(Symbol)
        range_scalar_field(previous_offset, *args)
      else
        raise "Unsupported type: #{type.inspect}!"
      end
    end

    def size(previous_offset = 0, parent = nil, index = nil)
      set_size_type(previous_offset, parent, index)
      first_offset = Float::INFINITY
      last_offset = -1
      size = 0
      self.class.instance_variable_get(:@fields).each { |args|
        start_offset, end_offset = range_field(previous_offset, *args)
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
          convert_field(*args)
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
          load_field(*args)
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
          dump_field(*args)
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

  end

end
