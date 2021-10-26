require "libbin_c.so"

require_relative 'libbin/alignment'
require_relative 'libbin/data_types'

module LibBin

  BIG_ARCHITECTURE = [0x12345678].pack("i") == "\x12\x34\x56\x78"
  @__big = nil
  @__output = $stderr

  class << self
    attr_accessor :__output
  end

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
      members = __shape_fields(kind)
      __unset_size_type
      return nil if members.values.compact.size <= 0
      kind::new(members)
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
