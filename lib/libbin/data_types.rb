module LibBin

  # Refinement to range to allow reducing range through the + operator
  module RangeRefinement
    refine Range do
      # Union of two ranges
      # @return [Range]
      def +(other)
        return other.dup unless min
        return self.dup unless other.min
        Range::new(min <= other.min ? min : other.min,
                   max >= other.max ? max : other.max)
      end
    end
  end

  # Classs that can be used to get the shape of a {Structure} or of a {Structure::Scalar}.
  class DataRange
    using RangeRefinement
    attr_reader :range

    # @overload initialize(min, max)
    #   Create a new shape starting and +min+ and ending at max.
    #   This shape has no members.
    #   @param min [Integer] start of the shape
    #   @param max [Integer] end of the shape
    #   @return [DataShape] a new DataShape
    # @overload initialize(members)
    #   Creates a new shape by reducing the shape of it's memebers.
    #   @param members [DataShape] the members composing the shape
    #   @return [DataShape] a new DataShape
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

    # Return the beginning of the shape
    # @return [Integer]
    def first
      @range.first
    end

    # Return the end of the shape
    # @return [Integer]
    def last
      @range.last
    end

    # Return the size of the shape
    # @return [Integer]
    def size
      @range.size
    end

  end

  # Default class used to get the shape of a {Structure} or of a {Structure::Scalar}.
  # It maintains a memory of it's members
  class DataShape # < DataRange
    using RangeRefinement
    attr_reader :members

    # Add method readers for members
    def method_missing(m, *arg, &block)
      return @members[m] if @members && @members[m]
      super
    end

    # @overload initialize(min, max)
    #   Create a new shape starting and +min+ and ending at max.
    #   This shape has no members.
    #   @param min [Integer] start of the shape
    #   @param max [Integer] end of the shape
    #   @return [DataShape] a new DataShape
    # @overload initialize(members)
    #   Creates a new shape by reducing the shape of it's memeber.
    #   Members can be accessed through {members}.
    #   @param members [DataShape] the members composing the shape
    #   @return [DataShape] a new DataShape
    def initialize(*args)
      if args.length == 1
        @members = args[0]
      end
      super
    end

  end

  # @abstract This class is used to describe a binary data structure
  # composed of different fields.
  class Structure

    # @!parse
    #   # Field class that is instantiated to describe a structure field.
    #   # Instances are immutable.
    #   class Field
    #     attr_reader :name
    #     attr_reader :type
    #     attr_reader :length
    #     attr_reader :count
    #     attr_reader :offset
    #     attr_reader :condition
    #     attr_reader :align
    #
    #     # @method initialize(name, type, length, count, offset, sequence, condition, relative_offset, align)
    #     # @param name [Symbol, String] the name of the field.
    #     # @param type [Class, String, Proc] the type of the field, as a Class, or
    #     #   as a String or Proc that will be evaluated in the context of the
    #     #   {Structure} instance.
    #     # @param length [Integer, String, Proc] if given, consider the field a
    #     #   vector of the type. The length is either a constant Integer of a
    #     #   String or Proc that will be evaluated in the context of the
    #     #   {Structure} instance.
    #     # @param count [Integer, String, Proc] if given, consider the field is
    #     #   repeated count times. The count is either a constant Integer of a
    #     #   String or Proc that will be evaluated in the context of the
    #     #   {Structure} instance.
    #     # @param offset [integer, String, Proc] if given, the absolute offset in
    #     #   the file, or the offset from the parent position, where the field can
    #     #   be found. See relative offset. The offset is either a constant
    #     #   Integer of a String or Proc that will be evaluated in the context
    #     #   of the {Structure} instance.
    #     # @param sequence [Boolean] if true, +type+, +length+, +offset+, and
    #     #   +condition+ are evaluated for each repetition.
    #     # @param condition [String, Proc] if given, the field, or repetition of the
    #     #   field can be conditionally present. The condition will be evaluated in
    #     #   the context of the {Structure} instance.
    #     # @param relative_offset [Boolean] consider the +offset+ relative to
    #     #   the field +parent+.
    #     # @param align [Integer] if given, align the field. If given as an
    #     #   Integer it must be a power of 2. Else the field is aligned to the
    #     #   field's type preferred alignment
    #     # @return [Field] new Field
    #
    #     # @method relative_offset?
    #     # Returns +true+ if the field offset should be relative to its parent.
    #     # @return [Boolean] field offset is relative
    #
    #     # @method sequence?
    #     # Returns +true+ if the field is a sequence
    #     # @return [Boolean] field is a sequence
    #   end

    # Define a new field in the structure
    # @param name [Symbol, String] the name of the field.
    # @param type [Class, String, Proc] the type of the field, as a Class, or
    #   as a String or Proc that will be evaluated in the context of the
    #   {Structure} instance.
    # @param length [Integer, String, Proc] if given, consider the field a
    #   vector of the type. The length is either a constant Integer of a
    #   String or Proc that will be evaluated in the context of the
    #   {Structure} instance.
    # @param count [Integer, String, Proc] if given, consider the field is
    #   repeated count times. The count is either a constant Integer of a
    #   String or Proc that will be evaluated in the context of the
    #   {Structure} instance.
    # @param offset [integer, String, Proc] if given, the absolute offset in
    #   the file, or the offset from the parent position, where the field can
    #   be found. See relative offset. The offset is either a constant
    #   Integer of a String or Proc that will be evaluated in the context
    #   of the {Structure} instance.
    # @param sequence [Boolean] if true, +type+, +length+, +offset+, and
    #   +condition+ are evaluated for each repetition.
    # @param condition [String, Proc] if given, the field, or repetition of the
    #   field can be conditionally present. The condition will be evaluated in
    #   the context of the {Structure} instance.
    # @param relative_offset [Boolean] consider the +offset+ relative to the
    #   field +parent+.
    # @param align [Boolean,Integer] if given, align the field. If given as an
    #   Integer it must be a power of 2. Else the field is aligned to the
    #   field's type preferred alignment.
    # @return self
    def self.field(name, type, length: nil, count: nil, offset: nil,
                   sequence: false, condition: nil, relative_offset: false,
                   align: false)
      if type.respond_to?(:always_align) && type.always_align
        al = type.align
        if align.kind_of?(Integer)
          align = align >= al ? align : al
        else
          align = al
        end
      end
      if align == true
        # Automatic alignment not supported for dynamic types
        if type.respond_to?(:align)
          align = type.align
        else
          raise "alignment is unsupported for dynamic types"
        end
      else
        raise "alignement must be a power of 2" if align && (align - 1) & align != 0
      end
      @fields.push(Field::new(name, type, length, count, offset, sequence,
                              condition, relative_offset, align))
      attr_accessor name
      self
    end

    class << self
      alias register_field field
    end

    # @!parse
    #   # @abstract Parent class for scalars
    #   class Scalar
    #   end

    # @!macro attach
    #   @!parse
    #     # $4
    #     class $1 < Scalar
    #       # @method align
    #       #   @scope class
    #       #   Returns the alignment of {$1}.
    #       #   @return [Integer] the byte boundary to align the type to.
    #       # @method shape(value = nil, offset = 0, parent = nil, index = nil, kind = DataShape, length = nil)
    #       #   @scope class
    #       #   Returns the shape of a field of type {$1}
    #       #   @param value [Object] ignored.
    #       #   @param offset [Integer] start of the shape.
    #       #   @param parent [Structure] ignored.
    #       #   @param index [Integer] ignored.
    #       #   @param kind [Class] shape class. Will be instantiated through
    #       #     new with the +offset+ and <tt>offset + sizeof($3) * length - 1</tt>.
    #       #   @param length [Integer] if given the length of the vector. Else
    #       #     the length is considered to be 1.
    #       #   @return [kind] a new instance of +kind+
    #       # @method size(value = nil, offset = 0, parent = nil, index = nil, length = nil)
    #       #   @scope class
    #       #   Returns the size of a field of type {$1}.
    #       #   @param value [Object] ignored.
    #       #   @param offset [Integer] ignored.
    #       #   @param parent [Structure] ignored.
    #       #   @param index [Integer] ignored.
    #       #   @param length [Integer] if given the length of the vector. Else
    #       #     the length is considered to be 1.
    #       #   @return [Integer] the type <tt>sizeof($3) * length</tt>.
    #       # @method load(input, input_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
    #       #   @scope class
    #       #   Load a field of type {$1} from +input+, and return it.
    #       #   @param input [IO] the stream to load the field from.
    #       #   @param input_big [Boolean] the endianness of +input+
    #       #   @param parent [Structure] ignored.
    #       #   @param index [Integer] ignored.
    #       #   @param length [Integer] if given the length of the vector. Else
    #       #     the length is considered to be 1.
    #       #   @return [Numeric, Array<Numeric>] the Ruby representation of the
    #       #     type, or the Array representation of the vector if +length+
    #       #     was specified
    #       # @method dump(value, output, output_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
    #       #   @scope class
    #       #   Dump a field of class {$1} to +output+.
    #       #   @param value [Numeric, Array<Numeric>] the Ruby representation
    #       #     of the type, or the Array representation of the vector if
    #       #     +length+ is specified.
    #       #   @param output [IO] the stream to dump the field to.
    #       #   @param output_big [Boolean] the endianness of +output+.
    #       #   @param parent [Structure] ignored.
    #       #   @param index [Integer] ignored.
    #       #   @param length [Integer] if given the length of the vector. Else
    #       #     the length is considered to be 1.
    #       #   @return [nil]
    #       # @method convert(input, output, input_big = LibBin::default_big?, output_big = !input_big, parent = nil, index = nil, length = nil)
    #       #   @scope class
    #       #   Convert a field of class {$1} by loading it from +input+ and
    #       #   dumping it to +output+. Returns the loaded field.
    #       #   @param input [IO] the stream to load the field from.
    #       #   @param output [IO] the stream to dump the field to.
    #       #   @param input_big [Boolean] the endianness of +input+
    #       #   @param output_big [Boolean] the endianness of +output+.
    #       #   @param parent [Structure] ignored.
    #       #   @param index [Integer] ignored.
    #       #   @param length [Integer] if given the length of the vector. Else
    #       #     the length is considered to be 1.
    #       #   @return [Numeric, Array<Numeric>] the Ruby representation of the
    #       #     type, or the Array representation of the vector if +length+
    #       #     was specified
    #     end
    #   @!method $2 method_signature(name, length: nil, count: nil, offset: nil, sequence: false, condition: nil, relative_offset: false, align: false)
    #     @!scope class
    #     Create a new field of type {$1} and name +name+. See {field} for options.
    #     @return self
    #
    def self.define_scalar_constructor(klassname, name, mapped_type, description)
      eval <<EOF
    def self.#{name}(name, length: nil, count: nil, offset: nil, sequence: false, condition: nil, relative_offset: false, align: false)
      if align == true
        align = #{klassname}.align
      else
        raise "alignement must be a power of 2" if align && (align - 1) & align != 0
      end
      @fields.push(Field::new(name, #{klassname}, length, count, offset, sequence, condition, relative_offset, align))
      attr_accessor name
      self
    end
EOF
    end
    private_class_method :define_scalar_constructor

    # @!group Scalars
    define_scalar_constructor "Int8", :int8, :int8_t, "A signed 8 bit integer"
    define_scalar_constructor "UInt8", :uint8, :uint8_t, "An unsigned 8 bit integer"
    define_scalar_constructor "Int16", :int16, :int16_t, "A signed 16 bit integer"
    define_scalar_constructor "Int16_LE", :int16_le, :int16_t, "A signed little endian 16 bit integer"
    define_scalar_constructor "Int16_BE", :int16_be, :int16_t, "A signed big endian 16 bit integer"
    define_scalar_constructor "UInt16", :uint16, :uint16_t, "An unsigned 16 bit integer"
    define_scalar_constructor "UInt16_LE", :uint16_le, :uint16_t, "An unsigned little endian 16 bit integer"
    define_scalar_constructor "UInt16_BE", :uint16_be, :uint16_t, "An unsigned big endian 16 bit integer"
    define_scalar_constructor "Int32", :int32, :int32_t, "A signed little endian 32 bit integer"
    define_scalar_constructor "Int32_LE", :int32_le, :int32_t, "A signed little endian 32 bit integer"
    define_scalar_constructor "Int32_BE", :int32_be, :int32_t, "A signed big endian 32 bit integer"
    define_scalar_constructor "UInt32", :uint32, :uint32_t, "An unsigned 32 bit integer"
    define_scalar_constructor "UInt32_LE", :uint32_le, :uint32_t, "An unsigned little endian 32 bit integer"
    define_scalar_constructor "UInt32_BE", :uint32_be, :uint32_t, "An unsigned big endian 32 bit integer"
    define_scalar_constructor "Int64", :int64, :int64_t, "A signed little endian 64 bit integer"
    define_scalar_constructor "Int64_LE", :int64_le, :int64_t, "A signed little endian 64 bit integer"
    define_scalar_constructor "Int64_BE", :int64_be, :int64_t, "A signed big endian 64 bit integer"
    define_scalar_constructor "UInt64", :uint64, :uint64_t, "An unsigned 64 bit integer"
    define_scalar_constructor "UInt64_LE", :uint64_le, :uint64_t, "An unsigned little endian 64 bit integer"
    define_scalar_constructor "UInt64_BE", :uint64_be, :uint64_t, "An unsigned big endian 64 bit integer"
    define_scalar_constructor "Flt", :float, :float, "A single precision floating point scalar"
    define_scalar_constructor "Flt_LE", :float_le, :float, "A single precision little endian floating point scalar"
    define_scalar_constructor "Flt_BE", :float_be, :float, "A single precision big endian floating point scalar"
    define_scalar_constructor "Double", :double, :double, "A double precision floating point scalar"
    define_scalar_constructor "Double_LE", :double_le, :double, "A double precision little endian floating point scalar"
    define_scalar_constructor "Double_BE", :double_be, :double, "A double precision big endian floating point scalar"
    define_scalar_constructor "Half", :half, :half, "A half precision floating point scalar"
    define_scalar_constructor "Half_LE", :half_le, :half, "A half precision little endian floating point scalar"
    define_scalar_constructor "Half_BE", :half_be, :half, "A half precision big endian floating point scalar"
    define_scalar_constructor "PGHalf", :pghalf, :pghalf, "A half precision floating point scalar as used by PlatinumGames in certain formats"
    define_scalar_constructor "PGHalf_LE", :pghalf_le, :pghalf, "A half precision little endian floating point scalar as used by PlatinumGames in certain formats"
    define_scalar_constructor "PGHalf_BE", :pghalf_be, :pghalf, "A half precision big endian floating point scalar as used by PlatinumGames in certain formats"

    # @!parse
    #   # A string type, can be NULL terminated or have an arbitrary length.
    #   class Str < Scalar
    #   end

    # Create a new field of type {Str} and name +name+. See {field} for options.
    # @return self
    def self.string(field, length = nil, count: nil, offset: nil, sequence: false, condition: nil, relative_offset: false, align: false)
      if align == true
        align = Str.align
      else
        raise "alignement must be a power of 2" if align && (align - 1) & align != 0
      end
      @fields.push(Field::new(field, Str, length, count, offset, sequence, condition, relative_offset, align))
      attr_accessor field
      self
    end

    # @abstract A parent class representing an enumeration that is loading integers as symbols.
    # Useer should inherit this base class.
    class Enum
      class << self
        #get the enum map
        attr_reader :map
        # Get the underlying type
        attr_accessor :type

        # Specify the size of the underlying type
        def type_size=(sz)
          t = eval "Int#{sz}"
          raise "unsupported enum size #{sz}" unless t
          @type = t
          return sz
        end
        alias set_type_size type_size=

        # Get the underlying type size in bits
        def type_size
          @type.size
        end

        # Set the enum map
        # @param m [Hash{Symbol=>Integer}] enum values
        def map=(m)
          @map_to = m.invert
          @map_from = @map = m
        end
        alias set_map map=

        # @!visibility private
        def inherited(subclass)
          subclass.instance_variable_set(:@type, Int32)
        end
      end

      # Returns the underlying type +always_align+ property
      def self.always_align
        @type.always_align
      end

      # Returns the underlying type +align+ property
      def self.align
        @type.align
      end

      # Forwards the call to the underlying type
      def self.size(value = nil, previous_offset = 0, parent = nil, index = nil, length = nil)
        @type.size(value, previous_offset, parent, index, length)
      end

      # Forwards the call to the underlying type
      def self.shape(value = nil, previous_offset = 0, parent = nil, index = nil, kind = DataShape, length = nil)
        @type.shape(value, previous_offset, parent, index, kind, length)
      end

      # Load a field of type {Enum} from +input+, and return it.
      # @param input [IO] the stream to load the field from.
      # @param input_big [Boolean] the endianness of +input+
      # @param parent [Structure] ignored.
      # @param index [Integer] ignored.
      # @param length [Integer] if given the length of the vector. Else
      #   the length is considered to be 1.
      # @return [Symbol,Integer,Array<Symbol,Integer>] the Ruby
      #   representation of the type, or the Array representation of the
      #   vector if +length+ was specified
      def self.load(input, input_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
        v = @type.load(input, input_big, parent, index, length)
        if length
          v.collect { |val|
             n = @map_to[val]
             n = val unless n
             n
          }
        else
          n = @map_to[v]
          n = v unless n
          n
        end
      end

      # Convert a field of class {Enum} by loading it from +input+ and
      # dumping it to +output+. Returns the loaded field.
      # @param input [IO] the stream to load the field from.
      # @param output [IO] the stream to dump the field to.
      # @param input_big [Boolean] the endianness of +input+
      # @param output_big [Boolean] the endianness of +output+.
      # @param parent [Structure] ignored.
      # @param index [Integer] ignored.
      # @param length [Integer] if given the length of the vector. Else
      #   the length is considered to be 1.
      # @return [Symbol,Integer,Array<Symbol,Integer>] the Ruby representation
      #   of the type, or the Array representation of the vector if +length+
      #   was specified
      def self.convert(input, output, input_big = LibBin::default_big?, output_big = !input_big, parent = nil, index = nil, length = nil)
        v = @type.convert(input, output, input_big, output_big, parent, index, length)
        if length
          v.collect { |val|
             n = @map_to[val]
             n = val unless n
             n
          }
        else
          n = @map_to[v]
          n = v unless n
          n
        end
      end

      # Dump a field of class {Enum} to +output+.
      # @param value [Numeric, Array<Numeric>] the Ruby representation
      #   of the type, or the Array representation of the vector if
      #   +length+ is specified.
      # @param output [IO] the stream to dump the field to.
      # @param output_big [Boolean] the endianness of +output+.
      # @param parent [Structure] ignored.
      # @param index [Integer] ignored.
      # @param length [Integer] if given the length of the vector. Else
      #   the length is considered to be 1.
      # @return [nil]
      def self.dump(value, output, output_big = LibBin::default_big?, parent = nil, index = nil, length = nil)
        if length
          v = length.times.collect { |i|
            val = @map_from[value[i]]
            val = value[i] unless val
            val
          }
        else
          v = @map_from[value]
          v = value unless v
        end
        @type.dump(v, output, output_big, parent, index, length)
      end
    end

    # Create a new field of a type inheriting from {Enum} and name +name+.
    # See {field} for more options
    # @param name [Symbol,String] name of the field.
    # @param map [Hash{Symbol => Integer}] enum values.
    # @param size [Integer] size in bits of the underlying integer
    # @return self
    def self.enum(name, map, size: 32, length: nil, count: nil, offset: nil, sequence: false, condition: nil, relative_offset: false, align: false)
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
      @fields.push(Field::new(name, klass, length, count, offset, sequence, condition, relative_offset, align))
      attr_accessor name
      self
    end

    # @abstract A parent class that represent a bitfield that is
    # loading integers as an instance of itself.
    # User should inherit this base class.
    class Bitfield
      class << self
        # Bitfield's field names and number of bits
        attr_reader :map
        # Signedness of the underlying type
        attr_reader :signed
        # Underlying type
        attr_reader :type

        # Set the size and signedness of the underlying type
        def set_type_size(sz, signed = false)
          tname = "#{signed ? "" : "U"}Int#{sz}"
          t = eval tname
          raise "unsupported bitfield type #{tname}" unless t
          @type = t
          @signed = signed
          return sz
        end

        # Set the underlying type
        def type=(type)
          @type = type
          @signed = type.name.split('::').last[0] != "U"
          type
        end
        alias set_type type=

        # Set the bitfield's field names and number of bits
        # Creates accessor to fields for instance of this class
        def map=(m)
          raise "map already set" if @map
          @map = m.each.collect { |k, v| [k, [v, k.to_sym, :"#{k}="]] }.to_h
          m.each_key { |k|
            attr_accessor k
          }
          m
        end
        alias set_map map=

        # @!visibility private
        def inherited(subclass)
          subclass.instance_variable_set(:@type, UInt32)
        end
      end

      # Unused bits of underlying value
      attr_accessor :__remainder

      # set only the unused bits of the underlying value
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

      # @!visibility private
      def initialize
        @__remainder = 0
      end

      # @!visibility private
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

      # @!visibility private
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

      # Returns the underlying type +always_align+ property
      def self.always_align
        @type.always_align
      end

      # Returns the underlying type +align+ property
      def self.align
        @type.align
      end

      # Forwards the call to the underlying type
      def self.size(value = nil, previous_offset = 0, parent = nil, index = nil, length = nil)
        @type.size(value, previous_offset, parent, index, length)
      end

      # Forwards the call to the underlying type
      def self.shape(value = nil, previous_offset = 0, parent = nil, index = nil, kind = DataShape, length = nil)
        @type.shape(value, previous_offset, parent, index, kind, length)
      end

      # Load a {Bitfield} from +input+, and return it.
      # @param input [IO] the stream to load the field from.
      # @param input_big [Boolean] the endianness of +input+
      # @param parent [Structure] ignored.
      # @param index [Integer] ignored.
      # @param length [Integer] if given the length of the vector. Else
      #   the length is considered to be 1.
      # @return [Bitfield,Array<Bitfield>] an instance of self, or an
      #   Array if length was specified
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

      # Convert a {Bitfield} by loading it from +input+ and
      # dumping it to +output+. Returns the loaded field.
      # @param input [IO] the stream to load the field from.
      # @param output [IO] the stream to dump the field to.
      # @param input_big [Boolean] the endianness of +input+
      # @param output_big [Boolean] the endianness of +output+.
      # @param parent [Structure] ignored.
      # @param index [Integer] ignored.
      # @param length [Integer] if given the length of the vector. Else
      #   the length is considered to be 1.
      # @return [Bitfield,Array<Bitfield>] an instance of self, or an
      #   Array if length was specified
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

      # Dump a {Bitfield} to +output+.
      # @param value [Bitfield, Array<Bitfield>] the Ruby representation
      #   of the type, or the Array representation of the vector if
      #   +length+ is specified.
      # @param output [IO] the stream to dump the field to.
      # @param output_big [Boolean] the endianness of +output+.
      # @param parent [Structure] ignored.
      # @param index [Integer] ignored.
      # @param length [Integer] if given the length of the vector. Else
      #   the length is considered to be 1.
      # @return [nil]
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

    # Create a new field of a type inheriting from {Bitfield} and name +name+.
    # See {field} for more options
    # @param name [Symbol,String] name of the field.
    # @param map [Hash{Symbol => Integer}] bitfield field names and number of bits.
    # @param size [Integer] size in bits of the underlying integer
    # @param signed [Boolean] signedness of the underlying type
    # @return self
    def self.bitfield(name, map, size: 32, signed: false, length: nil, count: nil, offset: nil, sequence: false, condition: nil, relative_offset: false, align: false)
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
      @fields.push(Field::new(name, klass, length, count, offset, sequence, condition, relative_offset, align))
      attr_accessor name
      self
    end

  end

end
