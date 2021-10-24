require "libbin_c.so"

require_relative 'libbin/alignment'
require_relative 'libbin/data_types'

module LibBin

  BIG_ARCHITECTURE = [0x12345678].pack("i") == "\x12\x34\x56\x78"
  @__big = nil

  def self.default_big?
    if @__big.nil?
      BIG_ARCHITECTURE
    else
      @__big
    end
  end

  class DataConverter
    include Alignment

    def inspect
      to_s
    end

    def self.inherited(subclass)
      subclass.instance_variable_set(:@fields, [])
    end

    def __size(previous_offset = 0, parent = nil, index = nil)
      __shape(previous_offset, parent, index, DataRange).size
    end

    def __shape(previous_offset = 0, parent = nil, index = nil, kind = DataShape)
      __set_size_type(previous_offset, parent, index)
      members = {}
      self.class.instance_variable_get(:@fields).each { |field|
        begin
          members[field.name] = __shape_field(send(field.name), kind, field)
        rescue
          STDERR.puts "#{self.class}: #{field.name}(#{field.type})"
          raise
        end
      }
      __unset_size_type
      return nil if members.values.compact.size <= 0
      kind::new(members)
    end

    def __convert_fields
      self.class.instance_variable_get(:@fields).each { |field|
        begin
          send("#{field.name}=", __convert_field(field))
        rescue
          STDERR.puts "#{self.class}: #{field.name}(#{field.type})"
          raise
        end
      }
      self
    end

#    def __dump_fields
#      self.class.instance_variable_get(:@fields).each { |field|
#        begin
#          __dump_field(send(field.name), field)
#        rescue
#          STDERR.puts "#{self.class}: #{field.name}(#{field.type})"
#          raise
#        end
#      }
#      self
#    end

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
      
    def self.convert(input, output, input_big = LibBin::default_big?, output_big = !input_big, parent = nil, index = nil, length = nil)
      if length
        length.times.collect {
          h = self::new
          h.__convert(input, output, input_big, output_big, parent, index)
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
