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

    def self.register_field(field, type, count: nil, offset: nil, sequence: false, condition: nil)
      @fields.push([field, type, count, offset, sequence, condition])
      attr_accessor field
    end

    def self.int8( field, count: nil, offset: nil, sequence: false, condition: nil)
      register_field(field, :c, count: count, offset: offset, sequence: sequence, condition: condition)
    end

    def self.uint8( field, count: nil, offset: nil, sequence: false, condition: nil)
      register_field(field, :C, count: count, offset: offset, sequence: sequence, condition: condition)
    end

    def self.int16( field, count: nil, offset: nil, sequence: false, condition: nil)
      register_field(field, :s, count: count, offset: offset, sequence: sequence, condition: condition)
    end

    def self.uint16( field, count: nil, offset: nil, sequence: false, condition: nil)
      register_field(field, :S, count: count, offset: offset, sequence: sequence, condition: condition)
    end

    def self.int32( field, count: nil, offset: nil, sequence: false, condition: nil)
      register_field(field, :l, count: count, offset: offset, sequence: sequence, condition: condition)
    end

    def self.uint32( field, count: nil, offset: nil, sequence: false, condition: nil)
      register_field(field, :L, count: count, offset: offset, sequence: sequence, condition: condition)
    end

    def self.int64( field, count: nil, offset: nil, sequence: false, condition: nil)
      register_field(field, :q, count: count, offset: offset, sequence: sequence, condition: condition)
    end

    def self.uint64( field, count: nil, offset: nil, sequence: false, condition: nil)
      register_field(field, :Q, count: count, offset: offset, sequence: sequence, condition: condition)
    end

    def self.float( field, count: nil, offset: nil, sequence: false, condition: nil)
      register_field(field, :F, count: count, offset: offset, sequence: sequence, condition: condition)
    end

    def self.double( field, count: nil, offset: nil, sequence: false, condition: nil)
      register_field(field, :D, count: count, offset: offset, sequence: sequence, condition: condition)
    end

    def self.half( field, count: nil, offset: nil, sequence: false, condition: nil)
      register_field(field, :half, count: count, offset: offset, sequence: sequence, condition: condition)
    end

    def self.pghalf( field, count: nil, offset: nil, sequence: false, condition: nil)
      register_field(field, :pghalf, count: count, offset: offset, sequence: sequence, condition: condition)
    end

    def self.string( field, length = nil, count: nil, offset: nil, sequence: false, condition: nil)
      if length
        register_field(field, :"a#{length}", count: count, offset: offset, sequence: sequence, condition: condition)
      else
        register_field(field, :"a*", count: count, offset: offset, sequence: sequence, condition: condition)
      end
    end


  end

end
