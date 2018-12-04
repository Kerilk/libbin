module LibBin

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
      :S => 2,
      :l => 4,
      :L => 4,
      :q => 8,
      :Q => 8,
      :F => 4,
      :D => 8,
      :"a*" => -1,
      :half => 2,
      :pghalf => 2
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
      :S => l["S>"],
      :l => l["l>"],
      :L => l["L>"],
      :q => l["q>"],
      :Q => l["Q>"],
      :F => l["g"],
      :D => l["G"],
      :"a*" => l["a*"],
      :half => [ lambda { |str| Flt::IEEE_binary16_BE::from_bytes(str).to(Float) },
                 lambda { |v| Flt::IEEE_binary16_BE::new(v).to_bytes } ],
      :pghalf => [ lambda { |str| Flt::IEEE_binary16_pg_BE::from_bytes(str).to(Float) },
                   lambda { |v| Flt::IEEE_binary16_pg_BE::new(v).to_bytes } ]
    } )
    DATA_ENDIAN[false].merge!( {
      :c => l["c"],
      :C => l["C"],
      :s => l["s<"],
      :S => l["S<"],
      :l => l["l<"],
      :L => l["L<"],
      :q => l["q<"],
      :Q => l["Q<"],
      :F => l["e"],
      :D => l["E"],
      :"a*" => l["a*"],
      :half => [ lambda { |str| Flt::IEEE_binary16::from_bytes(str).to(Float) },
                 lambda { |v| Flt::IEEE_binary16::new(v).to_bytes } ],
      :pghalf => [ lambda { |str| Flt::IEEE_binary16_pg::from_bytes(str).to(Float) },
                   lambda { |v| Flt::IEEE_binary16_pg::new(v).to_bytes } ]
    } )


    class Scalar

      def self.size(*args)
        @size
      end

      def self.init(symbol)
        @symbol = symbol
        @size = DATA_SIZES[symbol]
        @rl_be, @sl_be = DATA_ENDIAN[true][symbol]
        @rl_le, @sl_be = DATA_ENDIAN[false][symbol]
      end

      def self.load(input,  input_big = LibBin::default_big?, _ = nil, _ = nil)
        str = (@size < 0 ? input.readline("\x00") : input.read(@size))
        input_big ? @rl_be[str] : @rl_le[str]
      end

      def self.dump(value, output, output_big = LibBin::default_big?, _ = nil, _ = nil)
        str = (output_big ? @sl_be[value] : @sl_le[value])
        output.write(str)
      end

      def self.convert(input, output, input_big = LibBin::default_big?, output_big = !LibBin::default_big, _ = nil, _ = nil)
        str = (@size < 0 ? input.readline("\x00") : input.read(@size))
        value = (input_big ? @rl_be[str] : @rl_le[str])
        str = (output_big ? @sl_be[value] : @sl_le[value])
        output.write(str)
        value
      end

    end

    def self.register_field(field, type, count: nil, offset: nil, sequence: false, condition: nil)
      @fields.push([field, type, count, offset, sequence, condition])
      attr_accessor field
    end

    def self.create_scalar_type(name, symbol)
      eval <<EOF
    class #{name} < Scalar
      init(#{symbol.inspect})
    end

    def self.#{name.downcase}(field, count: nil, offset: nil, sequence: false, condition: nil)
      register_field(field, #{name}, count: count, offset: offset, sequence: sequence, condition: condition)
    end
EOF
    end

    create_scalar_type(:Int8, :c)
    create_scalar_type(:UInt8, :C)
    create_scalar_type(:Int16, :s)
    create_scalar_type(:UInt16, :S)
    create_scalar_type(:Int32, :l)
    create_scalar_type(:UInt32, :L)
    create_scalar_type(:Int64, :q)
    create_scalar_type(:UInt64, :Q)
    create_scalar_type(:Float, :F)
    create_scalar_type(:Double, :D)
    create_scalar_type(:Half, :half)
    create_scalar_type(:PGHalf, :pghalf)

    class Str < Scalar
    end

    def self.string( field, length = nil, count: nil, offset: nil, sequence: false, condition: nil)
      sym = (length ? :"a#{length}" : :"a*")
      c = Class::new(Str) do
        init(sym)
      end
      register_field(field, c, count: count, offset: offset, sequence: sequence, condition: condition)
    end


  end

end
