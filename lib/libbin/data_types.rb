module LibBin

  module RangeRefinement
    refine Range do
      def +(other)
        Range::new(first <= other.first ? first : other.first,
                   last >= other.last ? last : other.last,
                   exclude_end?)
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
        @range = @members.values.flatten.compact.collect(&:range).reduce { |memo, obj| memo + obj }
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
        @range = args[0].values.flatten.compact.collect(&:range).reduce { |memo, obj| memo + obj }
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

    rl = lambda { |type, str|
      str.unpack(type.to_s).first
    }

    sl = lambda { |type, value|
      [value].pack(type.to_s)
    }

    l = lambda { |type|
      [rl.curry[type], sl.curry[type]]
    }

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

    DATA_SIZES = Hash::new { |h,k|
      if k.kind_of?(Symbol) && m = k.match(/a(\d+)/)
        m[1].to_i
      else
        nil
      end
    }
    DATA_SIZES.merge!( {
      :c => 1,
      :C => 1,
      :s => 2,
      :"s<" => 2,
      :"s>" => 2,
      :S => 2,
      :"S<" => 2,
      :"S>" => 2,
      :v => 2,
      :n => 2,
      :l => 4,
      :"l<" => 4,
      :"l>" => 4,
      :L => 4,
      :"L<" => 4,
      :"L>" => 4,
      :V => 4,
      :N => 4,
      :q => 8,
      :"q<" => 8,
      :"q>" => 8,
      :Q => 8,
      :"Q<" => 8,
      :"Q>" => 8,
      :F => 4,
      :e => 4,
      :g => 4,
      :D => 8,
      :E => 8,
      :G => 8,
      :"a*" => -1,
      :half => 2,
      :half_le => 2,
      :half_be => 2,
      :pghalf => 2,
      :pghalf_le => 2,
      :pghalf_be => 2
    } )
    DATA_ENDIAN = {
      true => Hash::new { |h,k|
        if k.kind_of?(Symbol) && m = k.match(/a(\d+)/)
          l[k]
        else
          nil
        end
      },
      false => Hash::new { |h,k|
        if k.kind_of?(Symbol) && m = k.match(/a(\d+)/)
          l[k]
        else
          nil
        end
      }
    }

    DATA_ENDIAN[true].merge!( {
      :c => l["c"],
      :C => l["C"],
      :s => l["s>"],
      :"s<" => l["s<"],
      :"s>" => l["s>"],
      :S => l["S>"],
      :"S<" => l["S<"],
      :"S>" => l["S>"],
      :v => l["v"],
      :n => l["n"],
      :l => l["l>"],
      :"l<" => l["l<"],
      :"l>" => l["l>"],
      :L => l["L>"],
      :"L<" => l["L<"],
      :"L>" => l["L>"],
      :V => l["V"],
      :N => l["N"],
      :q => l["q>"],
      :"q<" => l["q<"],
      :"q>" => l["q>"],
      :Q => l["Q>"],
      :"Q<" => l["Q<"],
      :"Q>" => l["Q>"],
      :F => l["g"],
      :e => l["e"],
      :g => l["g"],
      :D => l["G"],
      :E => l["E"],
      :G => l["G"],
      :"a*" => l["a*"],
      :half => [ lambda { |str| Flt::IEEE_binary16_BE::from_bytes(str).to(Float) },
                 lambda { |v| Flt::IEEE_binary16_BE::new(v).to_bytes } ],
      :half_le => [ lambda { |str| Flt::IEEE_binary16_LE::from_bytes(str).to(Float) },
                    lambda { |v| Flt::IEEE_binary16_LE::new(v).to_bytes } ],
      :half_be => [ lambda { |str| Flt::IEEE_binary16_BE::from_bytes(str).to(Float) },
                    lambda { |v| Flt::IEEE_binary16_BE::new(v).to_bytes } ],
      :pghalf => [ lambda { |str| Flt::IEEE_binary16_pg_BE::from_bytes(str).to(Float) },
                   lambda { |v| Flt::IEEE_binary16_pg_BE::new(v).to_bytes } ],
      :pghalf_le => [ lambda { |str| Flt::IEEE_binary16_pg_LE::from_bytes(str).to(Float) },
                      lambda { |v| Flt::IEEE_binary16_pg_LE::new(v).to_bytes } ],
      :pghalf_be => [ lambda { |str| Flt::IEEE_binary16_pg_BE::from_bytes(str).to(Float) },
                      lambda { |v| Flt::IEEE_binary16_pg_BE::new(v).to_bytes } ]
    } )
    DATA_ENDIAN[false].merge!( {
      :c => l["c"],
      :C => l["C"],
      :s => l["s<"],
      :"s<" => l["s<"],
      :"s>" => l["s>"],
      :S => l["S<"],
      :"S<" => l["S<"],
      :"S>" => l["S>"],
      :v => l["v"],
      :n => l["n"],
      :l => l["l<"],
      :"l<" => l["l<"],
      :"l>" => l["l>"],
      :L => l["L<"],
      :"L<" => l["L<"],
      :"L>" => l["L>"],
      :V => l["V"],
      :N => l["N"],
      :q => l["q<"],
      :"q<" => l["q<"],
      :"q>" => l["q>"],
      :Q => l["Q<"],
      :"Q<" => l["Q<"],
      :"Q>" => l["Q>"],
      :F => l["e"],
      :e => l["e"],
      :g => l["g"],
      :D => l["E"],
      :E => l["E"],
      :G => l["G"],
      :"a*" => l["a*"],
      :half => [ lambda { |str| Flt::IEEE_binary16::from_bytes(str).to(Float) },
                 lambda { |v| Flt::IEEE_binary16::new(v).to_bytes } ],
      :half_le => [ lambda { |str| Flt::IEEE_binary16_LE::from_bytes(str).to(Float) },
                    lambda { |v| Flt::IEEE_binary16_LE::new(v).to_bytes } ],
      :half_be => [ lambda { |str| Flt::IEEE_binary16_BE::from_bytes(str).to(Float) },
                    lambda { |v| Flt::IEEE_binary16_BE::new(v).to_bytes } ],
      :pghalf => [ lambda { |str| Flt::IEEE_binary16_pg::from_bytes(str).to(Float) },
                   lambda { |v| Flt::IEEE_binary16_pg::new(v).to_bytes } ],
      :pghalf_le => [ lambda { |str| Flt::IEEE_binary16_pg_LE::from_bytes(str).to(Float) },
                      lambda { |v| Flt::IEEE_binary16_pg_LE::new(v).to_bytes } ],
      :pghalf_be => [ lambda { |str| Flt::IEEE_binary16_pg_BE::from_bytes(str).to(Float) },
                      lambda { |v| Flt::IEEE_binary16_pg_BE::new(v).to_bytes } ]
    } )


    class Scalar

      def self.size(*args)
        @size
      end

      def self.shape(value, previous_offset = 0, _ = nil, _ = nil, kind = DataShape)
        kind::new(previous_offset, previous_offset + @size - 1)
      end

      def self.init(symbol)
        @symbol = symbol
        @size = DATA_SIZES[symbol]
        @rl_be, @sl_be = DATA_ENDIAN[true][symbol]
        @rl_le, @sl_be = DATA_ENDIAN[false][symbol]
      end

      def self.load(input, input_big = LibBin::default_big?, _ = nil, _ = nil)
        str = input.read(@size)
        input_big ? @rl_be[str] : @rl_le[str]
      end

      def self.dump(value, output, output_big = LibBin::default_big?, _ = nil, _ = nil)
        str = (output_big ? @sl_be[value] : @sl_le[value])
        output.write(str)
      end

      def self.convert(input, output, input_big = LibBin::default_big?, output_big = !LibBin::default_big, _ = nil, _ = nil)
        str = input.read(@size)
        value = (input_big ? @rl_be[str] : @rl_le[str])
        str = (output_big ? @sl_be[value] : @sl_le[value])
        output.write(str)
        value
      end

    end

    class Str < Scalar

      def self.load(input,  input_big = LibBin::default_big?, _ = nil, _ = nil)
        str = (@size < 0 ? input.readline("\x00") : input.read(@size))
        input_big ? @rl_be[str] : @rl_le[str]
      end

      def self.convert(input, output, input_big = LibBin::default_big?, output_big = !LibBin::default_big, _ = nil, _ = nil)
        str = (@size < 0 ? input.readline("\x00") : input.read(@size))
        value = (input_big ? @rl_be[str] : @rl_le[str])
        str = (output_big ? @sl_be[value] : @sl_le[value])
        output.write(str)
        value
      end

      def self.shape(value, previous_offset = 0, _ = nil, _ = nil, kind = DataShape)
        if @size < 0
          kind::new(previous_offset, previous_offset + value.size - 1)
        else
          kind::new(previous_offset, previous_offset + @size - 1)
        end
      end

    end

    def self.register_field(field, type, count: nil, offset: nil, sequence: false, condition: nil)
      if type.kind_of?(Symbol)
        if type[0] == 'a'
          c = Class::new(Str) do init(sym) end
          @fields.push([field, c, count, offset, sequence, condition])
        else
          @fields.push([field, const_get(SCALAR_TYPES[type][0]), count, offset, sequence, condition])
        end
      else
        @fields.push([field, type, count, offset, sequence, condition])
      end
      attr_accessor field
    end

    def self.create_scalar_type(symbol)
      klassname, name = SCALAR_TYPES[symbol]
      eval <<EOF
    class #{klassname} < Scalar
      init(#{symbol.inspect})
    end

    def self.#{name}(field, count: nil, offset: nil, sequence: false, condition: nil)
      @fields.push([field, #{klassname}, count, offset, sequence, condition])
      attr_accessor field
    end
EOF
    end

    create_scalar_type(:c)
    create_scalar_type(:C)
    create_scalar_type(:s)
    create_scalar_type(:"s<")
    create_scalar_type(:"s>")
    create_scalar_type(:S)
    create_scalar_type(:"S<")
    create_scalar_type(:"S>")
    create_scalar_type(:l)
    create_scalar_type(:"l<")
    create_scalar_type(:"l>")
    create_scalar_type(:L)
    create_scalar_type(:"L<")
    create_scalar_type(:"L>")
    create_scalar_type(:q)
    create_scalar_type(:"q<")
    create_scalar_type(:"q>")
    create_scalar_type(:Q)
    create_scalar_type(:"Q<")
    create_scalar_type(:"Q>")
    create_scalar_type(:F)
    create_scalar_type(:e)
    create_scalar_type(:g)
    create_scalar_type(:D)
    create_scalar_type(:E)
    create_scalar_type(:G)
    create_scalar_type(:half)
    create_scalar_type(:half_le)
    create_scalar_type(:half_be)
    create_scalar_type(:pghalf)
    create_scalar_type(:pghalf_le)
    create_scalar_type(:pghalf_be)

    def self.string( field, length = nil, count: nil, offset: nil, sequence: false, condition: nil)
      sym = (length ? :"a#{length}" : :"a*")
      c = Class::new(Str) do
        init(sym)
      end
      @fields.push([field, c, count, offset, sequence, condition])
      attr_accessor field
    end


  end

end
