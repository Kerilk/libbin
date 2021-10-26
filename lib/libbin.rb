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

    def self.size(value, previous_offset = 0, parent = nil, index = nil, length = nil)
      if length
        shape(value, previous_offset, parent, index, length).size
      else
        value.__shape(previous_offset, parent, index).size
      end
    end

  end

end
