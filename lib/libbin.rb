require "libbin_c.so"

require_relative 'libbin/data_types'

module LibBin

  BIG_ARCHITECTURE = [0x12345678].pack("i") == "\x12\x34\x56\x78"
  @big = nil
  @output = $stderr

  class << self
    attr_accessor :output
  end

  # Returns true the default endianness is big
  def self.default_big?
    if @big.nil?
      BIG_ARCHITECTURE
    else
      @big
    end
  end

  class Structure

    # @!parse
    #   attr_accessor :__parent
    #   attr_accessor :__index
    #   attr_accessor :__position
    #   attr_accessor :__cur_position
    #   attr_accessor :__input
    #   attr_accessor :__output
    #   attr_accessor :__input_big
    #   attr_accessor :__output_big
    #
    #   attr_accessor :__offset
    #   attr_accessor :__condition
    #   attr_accessor :__type
    #   attr_accessor :__length
    #   attr_accessor :__count
    #   attr_accessor :__iterator
    #
    #   # @method __dump_fields
    #   # Dump fields according to the internal state of +self+
    #   # @return [Structure] self

    # @!visibility private
    def inspect
      to_s
    end

    # @!visibility private
    def self.inherited(subclass)
      subclass.instance_variable_set(:@fields, [])
    end

    # Returns the size of the structure
    # @param offset [Integer] position in the stream
    # @param parent [Structure] if given, parent structure
    # @param index [Integer] index if part of a repeated field inside parent
    # @return [Integer] size of the structure
    def __size(offset = 0, parent = nil, index = nil)
      __shape(offset, parent, index, DataRange).size
    end

    # Returns the alignement of the structure
    # @return [Integer] alignment of the structure
    def self.align
      return @always_align if @always_align
      align = @fields.collect(&:align).select { |v| v }.max
      align = 0 unless align
      align
    end

    # Set the structure as needing to always be aligned
    # @param align [true, Integer] if true use the fields' maximum alignment
    # @return align
    def self.set_always_align(align)
      if align == true
        @always_align = @fields.collect(&:align).select { |v| v }.max
        @always_align = 0 unless align
      else
        raise "alignement must be a power of 2" if align && (align - 1) & align != 0
        @always_align = align
      end
      align
    end

    class << self
      alias always_align= set_always_align
      attr_reader :always_align
      attr_reader :fields
    end

    # Returns the size of a structure
    # @param value [Structure,Array<Structure>] field or array of field to get the size of
    # @param offset [Integer] position in the stream
    # @param parent [Structure] if given, parent structure
    # @param index [Integer] index if part of a repeated field inside parent
    # @param length [Integer] if given, the length of the vector
    def self.size(value, offset = 0, parent = nil, index = nil, length = nil)
      if length
        shape(value, offset, parent, index, length).size
      else
        value.__shape(offset, parent, index).size
      end
    end

    def __fields
      return self.class.fields
    end

  end

end
