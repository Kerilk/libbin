module LibBin

  module RangeRefinement
    refine Range do
      def +(other)
        return other.dup unless min
        return self.dup unless other.min
        Range::new(min <= other.min ? min : other.min,
                   max >= other.max ? max : other.max)
      end
    end
  end

  class DataShape
    using RangeRefinement
    attr_reader :range
    attr_reader :members

    def method_missing(m, *arg, &block)
      return @members[m] if @members && @members[m]
      super
    end

    def initialize(*args)
      if args.length == 2
        @range = Range::new(args[0], args[1])
        @members = nil
      else
        @members = args[0]
        if @members.kind_of?(Hash)
          @range = @members.values.compact.collect(&:range).reduce(:+)
        else
          @range = @members.compact.collect(&:range).reduce(:+)
        end
      end
    end

    def first
      @range.first
    end

    def last
      @range.last
    end

    def size
      @range.size
    end

  end

  class DataRange
    using RangeRefinement
    attr_reader :range

    def initialize(*args)
      if args.length == 2
        @range = Range::new(args[0], args[1])
      else
        if args[0].kind_of?(Hash)
          @range = args[0].values.compact.collect(&:range).reduce(:+)
        else
          @range = args[0].compact.collect(&:range).reduce(:+)
        end
      end
    end

    def first
      @range.first
    end

    def last
      @range.last
    end

    def size
      @range.size
    end

  end

#  class Field
#    attr_reader :name,
#                :type,
#                :length,
#                :count,
#                :offset,
#                :sequence,
#                :condition
#
#    def sequence?
#      @sequence
#    end
#
#    def relative_offset?
#      @relative_offset
#    end
#
#    def initialize(name, type, length, count, offset, sequence, condition, relative_offset)
#      @name = name
#      @type = type
#      @length = length
#      @count = count
#      @offset = offset
#      @sequence = sequence
#      @condition = condition
#      @relative_offset = relative_offset
#    end
#
#  end

  class DataConverter

    SCALAR_TYPES = {
      :c => [:Int8, :int8],
      :C => [:UInt8, :uint8],
      :s => [:Int16, :int16],
      :"s<" => [:Int16_LE, :int16_le],
      :"s>" => [:Int16_BE, :int16_be],
      :S => [:UInt16, :uint16],
      :"S<" => [:UInt16_LE, :uint16_le],
      :"S>" => [:UInt16_BE, :uint16_be],
      :v => [:UInt16_LE, :uint16_le],
      :n => [:UInt16_BE, :uint16_be],
      :l => [:Int32, :int32],
      :"l<" => [:Int32_LE, :int32_le],
      :"l>" => [:Int32_BE, :int32_be],
      :L => [:UInt32, :uint32],
      :"L<" => [:UInt32_LE, :uint32_le],
      :"L>" => [:UInt32_BE, :uint32_be],
      :V => [:UInt32_LE, :uint32_le],
      :N => [:UInt32_BE, :uint32_be],
      :q => [:Int64, :int64],
      :"q<" => [:Int64_LE, :int64_le],
      :"q>" => [:Int64_BE, :int64_be],
      :Q => [:UInt64, :uint64],
      :"Q<" => [:UInt64_LE, :uint64_le],
      :"Q>" => [:UInt64_BE, :uint64_be],
      :F => [:Flt, :float],
      :e => [:Flt_LE, :float_le],
      :g => [:Flt_BE, :float_be],
      :D => [:Double, :double],
      :E => [:Double_LE, :double_le],
      :G => [:Double_BE, :double_be],
      :half => [:Half, :half],
      :half_le => [:Half_LE, :half_le],
      :half_be => [:Half_BE, :half_be],
      :pghalf => [:PGHalf, :pghalf],
      :pghalf_le => [:PGHalf_LE, :pghalf_le],
      :pghalf_be => [:PGHalf_BE, :pghalf_be]
    }

    def self.register_field(field, type, length: nil, count: nil, offset: nil, sequence: false, condition: nil, relative_offset: false)
      if type.kind_of?(Symbol)
        if type[0] == 'a'
          real_type = Str
        else
          real_type = const_get(SCALAR_TYPES[type][0])
        end
      else
        real_type = type
      end
      @fields.push(Field::new(field, real_type, length, count, offset, sequence, condition, relative_offset))
      attr_accessor field
    end

    class << self
      alias field register_field
    end

    def self.create_scalar_accessor(symbol)
      klassname, name = SCALAR_TYPES[symbol]
      eval <<EOF
    def self.#{name}(field, length: nil, count: nil, offset: nil, sequence: false, condition: nil, relative_offset: false)
      @fields.push(Field::new(field, #{klassname}, length, count, offset, sequence, condition, relative_offset))
      attr_accessor field
    end
EOF
    end

    [:c,
     :C,
     :s, :"s<", :"s>",
     :S, :"S<", :"S>",
     :l, :"l<", :"l>",
     :L, :"L<", :"L>",
     :q, :"q<", :"q>",
     :Q, :"Q<", :"Q>",
     :F, :e, :g,
     :D, :E, :G,
     :half, :half_le, :half_be,
     :pghalf, :pghalf_le, :pghalf_be].each { |c|
      create_scalar_accessor(c)
    }

    def self.string( field, length = nil, count: nil, offset: nil, sequence: false, condition: nil, relative_offset: false)
      @fields.push(Field::new(field, Str, length, count, offset, sequence, condition, relative_offset))
      attr_accessor field
    end

  end

end
