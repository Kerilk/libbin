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

    def self.register_field(field, type, length: nil, count: nil, offset: nil, sequence: false, condition: nil, relative_offset: false, align: false)
      if type.kind_of?(Symbol)
        if type[0] == 'a'
          real_type = Str
        else
          real_type = const_get(SCALAR_TYPES[type][0])
        end
      else
        real_type = type
      end
      if real_type.respond_to?(:always_align) && real_type.always_align
        al = real_type.align
        if align.kind_of?(Integer)
          align = align >= al ? align : al
        else
          align = al
        end
      end
      if align == true
        if real_type.respond_to?(:align) # Automatic alignment not supported for dynamic types
          align = real_type.align
        else
          raise "alignment is unsupported for dynamic types"
        end
      else
        raise "alignement must be a power of 2" if align && (align - 1) & align != 0
      end
      @fields.push(Field::new(field, real_type, length, count, offset, sequence, condition, relative_offset, align))
      attr_accessor field
    end

    class << self
      alias field register_field
    end

    def self.create_scalar_accessor(symbol)
      klassname, name = SCALAR_TYPES[symbol]
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
