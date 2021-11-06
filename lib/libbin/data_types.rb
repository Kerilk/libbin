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

  class DataConverter

    # List of natively supported scalar types
    SCALAR_TYPES = [
      [:Int8, :int8],
      [:UInt8, :uint8],
      [:Int16, :int16],
      [:Int16_LE, :int16_le],
      [:Int16_BE, :int16_be],
      [:UInt16, :uint16],
      [:UInt16_LE, :uint16_le],
      [:UInt16_BE, :uint16_be],
      [:UInt16_LE, :uint16_le],
      [:UInt16_BE, :uint16_be],
      [:Int32, :int32],
      [:Int32_LE, :int32_le],
      [:Int32_BE, :int32_be],
      [:UInt32, :uint32],
      [:UInt32_LE, :uint32_le],
      [:UInt32_BE, :uint32_be],
      [:UInt32_LE, :uint32_le],
      [:UInt32_BE, :uint32_be],
      [:Int64, :int64],
      [:Int64_LE, :int64_le],
      [:Int64_BE, :int64_be],
      [:UInt64, :uint64],
      [:UInt64_LE, :uint64_le],
      [:UInt64_BE, :uint64_be],
      [:Flt, :float],
      [:Flt_LE, :float_le],
      [:Flt_BE, :float_be],
      [:Double, :double],
      [:Double_LE, :double_le],
      [:Double_BE, :double_be],
      [:Half, :half],
      [:Half_LE, :half_le],
      [:Half_BE, :half_be],
      [:PGHalf, :pghalf],
      [:PGHalf_LE, :pghalf_le],
      [:PGHalf_BE, :pghalf_be]
    ]


    def self.field(field, type, length: nil, count: nil, offset: nil, sequence: false, condition: nil, relative_offset: false, align: false)
      if type.respond_to?(:always_align) && type.always_align
        al = type.align
        if align.kind_of?(Integer)
          align = align >= al ? align : al
        else
          align = al
        end
      end
      if align == true
        if type.respond_to?(:align) # Automatic alignment not supported for dynamic types
          align = type.align
        else
          raise "alignment is unsupported for dynamic types"
        end
      else
        raise "alignement must be a power of 2" if align && (align - 1) & align != 0
      end
      @fields.push(Field::new(field, type, length, count, offset, sequence, condition, relative_offset, align))
      attr_accessor field
    end

    class << self
      alias register_field field
    end

    def self.define_scalar_constructor(klassname, name)
      eval <<EOF
    def self.#{name}(field, length: nil, count: nil, offset: nil, sequence: false, condition: nil, relative_offset: false, align: false)
      if align == true
        align = #{klassname}.align
      else
        raise "alignement must be a power of 2" if align && (align - 1) & align != 0
      end
      @fields.push(Field::new(field, #{klassname}, length, count, offset, sequence, condition, relative_offset, align))
      attr_accessor field
    end
EOF
    end

    SCALAR_TYPES.each { |klassname, name|
      define_scalar_constructor(klassname, name)
    }

    def self.string(field, length = nil, count: nil, offset: nil, sequence: false, condition: nil, relative_offset: false, align: false)
      if align == true
        align = Str.align
      else
        raise "alignement must be a power of 2" if align && (align - 1) & align != 0
      end
      @fields.push(Field::new(field, Str, length, count, offset, sequence, condition, relative_offset, align))
      attr_accessor field
    end

    class Enum
      class << self
        attr_reader :map
        attr_accessor :map_to
        attr_accessor :map_from
        attr_accessor :type

        def type_size=(sz)
          t = eval "Int#{sz}"
          raise "unsupported enum size #{sz}" unless t
          @type = t
          return sz
        end
        alias set_type_size type_size=

        def type_size
          @type.size
        end

        def map=(m)
          @map_to = m.invert
          @map_from = @map = m
        end
        alias set_map map=

        def inherited(subclass)
          subclass.instance_variable_set(:@type, Int32)
        end
      end

      def self.always_align
        @type.always_align
      end

      def self.align
        @type.align
      end

      def self.size(value = nil, previous_offset = 0, parent = nil, index = nil, length = nil)
        @type.size(value, previous_offset, parent, index, length)
      end

      def self.shape(value = nil, previous_offset = 0, parent = nil, index = nil, kind = DataShape, length = nil)
        @type.shape(value, previous_offset, parent, index, kind, length)
      end

      def self.load(input, input_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
        v = @type.load(input, input_big, parent, index, length)
        if length
          v.collect { |val|
             n = map_to[val]
             n = val unless n
             n
          }
        else
          n = map_to[v]
          n = v unless n
          n
        end
      end

      def self.convert(input, output, input_big = LibBin::default_big?, output_big = !input_big, parent = nil, index = nil, length = nil)
        v = @type.convert(input, output, input_big, output_big, parent, index, length)
        if length
          v.collect { |val|
             n = map_to[val]
             n = val unless n
             n
          }
        else
          n = map_to[v]
          n = v unless n
          n
        end
      end

      def self.dump(value, output, output_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
        if length
          v = length.times.collect { |i|
            val = map_from[value[i]]
            val = value[i] unless val
            val
          }
        else
          v = map_from[value]
          v = value unless v
        end
        @type.dump(v, output, output_big, parent, index, length)
      end
    end

    def self.enum(field, map, size: 32, length: nil, count: nil, offset: nil, sequence: false, condition: nil, relative_offset: false, align: false)
      klass = Class.new(Enum) do |c|
        c.type_size = size
        c.map = map
      end
      if klass.always_align
        al = klass.align
        if align.kind_of?(Integer)
          align = align >= al ? align : al
        else
          align = al
        end
      end
      if align == true
        align = klass.align
      else
        raise "alignement must be a power of 2" if align && (align - 1) & align != 0
      end
      @fields.push(Field::new(field, klass, length, count, offset, sequence, condition, relative_offset, align))
      attr_accessor field
    end

    class Bitfield
      class << self
        attr_reader :map
        attr_reader :signed
        attr_reader :type

        def set_type_size(sz, signed = false)
          tname = "#{signed ? "" : "U"}Int#{sz}"
          t = eval tname
          raise "unsupported bitfield type #{tname}" unless t
          @type = t
          @signed = signed
          return sz
        end

        def type=(type)
          @type = type
          @signed = type.name.split('::').last[0] != "U"
          type
        end
        alias set_type type=

        def map=(m)
          raise "map already set" if @map
          @map = m.each.collect { |k, v| [k, [v, k.to_sym, :"#{k}="]] }.to_h
          m.each_key { |k|
            attr_accessor k
          }
          m
        end
        alias set_map map=

        def inherited(subclass)
          subclass.instance_variable_set(:@type, UInt32)
        end
      end

      attr_reader :__remainder

      def __remainder=(v) # only keep relevant bits of the remainder
        if v != 0
          num_bits = self.class.type.size * 8
          num_used_bits = self.class.map.value.collect { |v, _, _| v }.select { |v| v > 0 }.sum(:+)
          if num_used_bits < num_bits
            v &= ((( 1 << (num_bits - num_used_bits)) - 1) << num_used_bits)
          else
            v = 0
          end
        end
        @__remainder = v
      end

      def initialize
        @__remainder = 0
      end

      def __set_value(val)
        base = 0
        self.class.map.each { |k, (v, _, s)|
          next if v <= 0
          tmp = (val >> base) & ((1 << v) - 1)
          val ^= tmp << base #remove bits from val
          tmp -= (1 << v) if self.class.signed && tmp >= (1 << (v - 1))
          send(s, tmp)
          base += v
        }
        @__remainder = val
      end

      def __get_value
        base = 0
        val = 0
        self.class.map.each { |k, (v, g, _)|
          next if v <= 0
          val |= ((send(g) & ((1 << v) - 1)) << base)
          base += v
        }
        val | @__remainder
      end

      def self.always_align
        @type.always_align
      end

      def self.align
        @type.align
      end

      def self.size(value = nil, previous_offset = 0, parent = nil, index = nil, length = nil)
        @type.size(value, previous_offset, parent, index, length)
      end

      def self.shape(value = nil, previous_offset = 0, parent = nil, index = nil, kind = DataShape, length = nil)
        @type.shape(value, previous_offset, parent, index, kind, length)
      end

      def self.load(input, input_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
        v = @type.load(input, input_big, parent, index, length)
        if length
          v.collect { |val|
             bf = self.new
             bf.__set_value(val)
             bf
          }
        else
          bf = self.new
          bf.__set_value(v)
          bf
        end
      end

      def self.convert(input, output, input_big = LibBin::default_big?, output_big = !input_big, parent = nil, index = nil, length = nil)
        v = @type.convert(input, output, input_big, output_big, parent, index, length)
        if length
          v.collect { |val|
             bf = self.new
             bf.__set_value(val)
             bf
          }
        else
          bf = self.new
          bf.__set_value(v)
          bf
        end
      end

      def self.dump(value, output, output_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
        v =
          if length
            length.times.collect { |i|
              value[i].__get_value
            }
          else
            value.__get_value
          end
        @type.dump(v, output, output_big, parent, index, length)
      end
    end

    def self.bitfield(field, map, size: 32, signed: false, length: nil, count: nil, offset: nil, sequence: false, condition: nil, relative_offset: false, align: false)
      klass = Class.new(Bitfield) do |c|
        c.set_type_size(size, signed)
        c.set_map(map)
      end
      if klass.always_align
        al = klass.align
        if align.kind_of?(Integer)
          align = align >= al ? align : al
        else
          align = al
        end
      end
      if align == true
        align = klass.align
      else
        raise "alignement must be a power of 2" if align && (align - 1) & align != 0
      end
      @fields.push(Field::new(field, klass, length, count, offset, sequence, condition, relative_offset, align))
      attr_accessor field
    end

  end

end
