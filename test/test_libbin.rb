[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'libbin'
require 'stringio'

def bin_path(filename)
  File.join(__dir__, "binary", filename)
end

def open_bin(filename, &block)
  File::open(bin_path(filename), "rb", &block)
end

def new_stringio()
  StringIO::new("", "r+b")
end

class LibBinTest < Minitest::Test
  SUFFIX = { true => "be", false => "le" }

  def test_half
    c = Class::new(LibBin::DataConverter) do
      register_field :a, :half, count: 4
      register_field :b, :pghalf, length: 4
    end

    [true, false].each { |big|
      open_bin("half_#{SUFFIX[big]}.bin") do |f|
        s = c::load(f, big)
        assert_equal( (1..4).to_a, s.a )
        assert_equal( (1..4).to_a, s.b )
        shape = s.__shape
        assert_equal( 0x08, shape.b.size)
        assert_equal( 0x08, shape.b.first)
        assert_equal( 0x0f, shape.b.last)
        str = new_stringio
        c::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        open_bin("half_#{SUFFIX[!big]}.bin") do |g|
          str = new_stringio
          f.rewind
          s = c::convert(f, str, big, !big)
          str.rewind
          assert_equal(g.read, str.read)
        end
      end
    }
  end

  def test_half_le
    c = Class::new(LibBin::DataConverter) do
      field :a, :half_le, length: 4
      field :b, :pghalf_le, count: 4
    end

    open_bin("half_le.bin") do |f|
      s = c::load(f, true)
      assert_equal( (1..4).to_a, s.a )
      assert_equal( (1..4).to_a, s.b )
      str = new_stringio
      shape = s.__shape
      assert_equal( 0x08, shape.b.size)
      assert_equal( 0x08, shape.b.first)
      assert_equal( 0x0f, shape.b.last)
      c::dump(s, str, true)
      f.rewind
      str.rewind
      assert_equal(f.read, str.read)
      f.rewind
      str = new_stringio
      c::convert(f, str, true, true)
      f.rewind
      str.rewind
      assert_equal(f.read, str.read)
     end
  end

  def test_half_be
    c = Class::new(LibBin::DataConverter) do
      register_field :a, :half_be, count: 4
      register_field :b, :pghalf_be, count: 4
    end

    open_bin("half_be.bin") do |f|
      s = c::load(f, false)
      assert_equal( (1..4).to_a, s.a )
      assert_equal( (1..4).to_a, s.b )
      str = new_stringio
      c::dump(s, str, false)
      f.rewind
      str.rewind
      assert_equal(f.read, str.read)
      f.rewind
      str = new_stringio
      c::convert(f, str, false, false)
      f.rewind
      str.rewind
      assert_equal(f.read, str.read)
    end
  end

  def test_register_field
    c = Class::new(LibBin::DataConverter) do
      register_field :a, :c
      register_field :b, LibBin::DataConverter::Int8
      register_field :c, :s
      register_field :d, :l
      register_field :e, :F
    end

    [true, false].each { |big|
      open_bin("simple_layout_#{SUFFIX[big]}.bin") do |f|
        s = c::load(f, big)
        assert_equal(1, s.a)
        assert_equal(0, s.b)
        assert_equal(2, s.c)
        assert_equal(3, s.d)
        assert_equal(4.0, s.e)
        str = new_stringio
        c::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        open_bin("simple_layout_#{SUFFIX[!big]}.bin") do |g|
          str = new_stringio
          f.rewind
          s = c::convert(f, str, big, !big)
          str.rewind
          assert_equal(g.read, str.read)
        end
      end
    }
  end

  def test_forced_byte_order
    c = Class::new(LibBin::DataConverter) do
      int8 :a
      int8 :b
      int16_le :c
      int32_le :d
      float_le :e
    end

    [true, false].each { |big|
      open_bin("simple_layout_le.bin") do |f|
        s = c::load(f, big)
        assert_equal(1, s.a)
        assert_equal(0, s.b)
        assert_equal(2, s.c)
        assert_equal(3, s.d)
        assert_equal(4.0, s.e)
        str = new_stringio
        c::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
      end
    }

    c = Class::new(LibBin::DataConverter) do
      int8 :a
      int8 :b
      int16_be :c
      int32_be :d
      float_be :e
    end

    [true, false].each { |big|
      open_bin("simple_layout_be.bin") do |f|
        s = c::load(f, big)
        assert_equal(1, s.a)
        assert_equal(0, s.b)
        assert_equal(2, s.c)
        assert_equal(3, s.d)
        assert_equal(4.0, s.e)
        str = new_stringio
        c::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
      end
    }
  end

  def test_simple_layout
    c = Class::new(LibBin::DataConverter) do
      int8 :a
      int8 :b
      int16 :c
      int32 :d
      float :e
    end

    [true, false].each { |big|
      open_bin("simple_layout_#{SUFFIX[big]}.bin") do |f|
        s = c::load(f, big)
        assert_equal(1, s.a)
        assert_equal(0, s.b)
        assert_equal(2, s.c)
        assert_equal(3, s.d)
        assert_equal(4.0, s.e)
        str = new_stringio
        c::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        open_bin("simple_layout_#{SUFFIX[!big]}.bin") do |g|
          str = new_stringio
          f.rewind
          s = c::convert(f, str, big, !big)
          str.rewind
          assert_equal(g.read, str.read)
        end
      end
    }
  end

  def test_array
    c = Class::new(LibBin::DataConverter) do
      int8 :num
      int8 :a, count: 'num'
    end

    open_bin("simple_array.bin") do |f|
      s = c::load(f)
      assert_equal(15,  s.num)
      assert_equal(s.num, s.a.length)
      s.a.each_with_index { |e, i|
         assert_equal(i, e)
      }
      str = new_stringio
      c::dump(s, str)
      f.rewind
      str.rewind
      assert_equal(f.read, str.read)
    end
  end

  def test_offset
    c = Class::new(LibBin::DataConverter) do
      int8 :b, offset: 1
      int32 :d, offset: 4
    end

    [true, false].each { |big|
      open_bin("simple_layout_#{SUFFIX[big]}.bin") do |f|
        s = c::load(f, big)
        assert_equal(0, s.b)
        assert_equal(3, s.d)
      end
    }
  end

  def test_offset_relative
    c = Class::new(LibBin::DataConverter) do
      int8 :b, offset: 1, relative_offset: true
      int32 :d, offset: 4, relative_offset: true
    end

    [true, false].each { |big|
      open_bin("simple_layout_#{SUFFIX[big]}.bin") do |f|
        s = c::load(f, big)
        assert_equal(0, s.b)
        assert_equal(3, s.d)
      end
    }
  end

  def test_sequence
    c = Class::new(LibBin::DataConverter) do
      int16 :offsets, count: 6
      int16 :data, count: 6, sequence: true, offset: 'offsets[__iterator]'
    end

    [true, false].each { |big|
      open_bin("sequence_layout_#{SUFFIX[big]}.bin") do |f|
        s = c::load(f, big)
        assert_equal((1..6).to_a, s.data)
        str = new_stringio
        c::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        open_bin("sequence_layout_#{SUFFIX[!big]}.bin") do |g|
          str = new_stringio
          f.rewind
          s = c::convert(f, str, big, !big)
          str.rewind
          assert_equal(g.read, str.read)
        end
      end
    }
  end

  def test_sequence_condition
    c = Class::new(LibBin::DataConverter) do
      int16 :offsets, count: 6
      int16 :data, count: 6, sequence: true,
            offset: 'offsets[__iterator]',
            condition: '__iterator % 2 == 0'
    end

    [true, false].each { |big|
      open_bin("sequence_layout_#{SUFFIX[big]}.bin") do |f|
        s = c::load(f, big)
        assert_equal([1, nil, 3, nil, 5, nil], s.data)
      end
    }
  end

  def test_sequence_null
    c = Class::new(LibBin::DataConverter) do
      int16 :offsets, count: 6
      int16 :data, count: 6, sequence: true, offset: 'offsets[__iterator]'
    end

    [true, false].each { |big|
      open_bin("sequence_null_layout_#{SUFFIX[big]}.bin") do |f|
        s = c::load(f, big)
        assert_equal([1, 2, nil, 4, 5, 6], s.data)
      end
    }
  end

  def test_datatypes
    b = Class::new(LibBin::DataConverter) do
      int8 :a
      int8 :b
    end

    c = Class::new(LibBin::DataConverter) do
      register_field :bs, b, count: 5
    end

    open_bin("simple_array.bin") do |f|
      s = c::load(f)
      assert_equal(15,  s.bs[0].a)
      assert_equal(0,  s.bs[0].b)
      (1..4).each { |i|
        assert_equal(2*i - 1, s.bs[i].a)
        assert_equal(2*i    , s.bs[i].b)
      }
    end
  end

  def test_local_reference
    h = Class::new(LibBin::DataConverter) do
      int32 :offset
      int32 :count
    end
    b = Class::new(LibBin::DataConverter) do
      register_field :header, h
      int32 :data, length: 'header.count', offset: 'header.offset'
    end

    [true, false].each { |big|
      open_bin("offset_#{SUFFIX[big]}.bin") do |f|
        s = b::load(f, big)
        assert_equal(32, s.header.offset)
        assert_equal(4, s.header.count)
        (0..3).each { |i|
          assert_equal(i+1, s.data[i])
        }
        str = new_stringio
        b::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        open_bin("offset_#{SUFFIX[!big]}.bin") do |g|
          str = new_stringio
          f.rewind
          s = b::convert(f, str, big, !big)
          str.rewind
          assert_equal(g.read, str.read)
        end
      end
    }
  end

  def test_remote_reference
    h = Class::new(LibBin::DataConverter) do
      int32 :offset
      int32 :count
    end
    d = Class::new(LibBin::DataConverter) do
      int32 :data, count: '..\header.count', offset: '..\header.offset'
    end
    b = Class::new(LibBin::DataConverter) do
      register_field :header, h
      register_field :body, d
    end
    [true, false].each { |big|
      open_bin("offset_#{SUFFIX[big]}.bin") do |f|
        s = b::load(f, big)
        assert_equal(32, s.header.offset)
        assert_equal(4, s.header.count)
        (0..3).each { |i|
          assert_equal(i+1, s.body.data[i])
        }
        str = new_stringio
        b::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        open_bin("offset_#{SUFFIX[!big]}.bin") do |g|
          str = new_stringio
          f.rewind
          s = b::convert(f, str, big, !big)
          str.rewind
          assert_equal(g.read, str.read)
        end
      end
    }
  end

  def test_class_count
    h = Class::new(LibBin::DataConverter) do
      register_field :f, :half
    end
    pgh = Class::new(LibBin::DataConverter) do
      register_field :f, :pghalf
    end
    c = Class::new(LibBin::DataConverter) do
      register_field :a, h, count: 4
      register_field :b, pgh, length: lambda { 4 }
    end

    [true, false].each { |big|
      open_bin("half_#{SUFFIX[big]}.bin") do |f|
        s = c::load(f, big)
        assert_equal( (1..4).to_a, s.a.collect(&:f) )
        assert_equal( (1..4).to_a, s.b.collect(&:f) )
        str = new_stringio
        c::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        open_bin("half_#{SUFFIX[!big]}.bin") do |g|
          str = new_stringio
          f.rewind
          s = c::convert(f, str, big, !big)
          str.rewind
          assert_equal(g.read, str.read)
        end
      end
    }
  end

  def test_size
    h = Class::new(LibBin::DataConverter) do
      int32 :offset1
      int16 :offset2
    end
    b = Class::new(LibBin::DataConverter) do
      int32 :datum1, offset: '..\header\offset1'
      int16 :datum2, offset: proc { __parent.header.offset2 }
    end
    s = Class::new(LibBin::DataConverter) do
      register_field :header, h
      register_field :body, b
    end
    [false, true].each { |big|
      open_bin("test_size_#{SUFFIX[big]}.bin") do |f|
        str = s::load(f, big)
        assert_equal(1, str.body.datum1)
        assert_equal(2, str.body.datum2)
        assert_equal(0x22, str.__size)
        shape = str.__shape
        assert_equal(0x22, shape.size)
        assert_equal(0x00, shape.first)
        assert_equal(0x21, shape.last)
        assert_equal(0x06, shape.header.size)
        assert_equal(0x00, shape.header.first)
        assert_equal(0x05, shape.header.last)
        assert_equal(0x04, shape.header.offset1.size)
        assert_equal(0x00, shape.header.offset1.first)
        assert_equal(0x03, shape.header.offset1.last)
        assert_equal(0x12, shape.body.size)
        assert_equal(0x10, shape.body.first)
        assert_equal(0x21, shape.body.last)
      end
    }

    c = Class::new(LibBin::DataConverter) do
      int8 :num
      int8 :a, count: 'num'
    end

    open_bin("simple_array.bin") do |f|
      s = c::load(f)
      assert_equal(0x10, s.__size)
      shape = s.__shape
      assert_equal(0x10, shape.size)
      assert_equal(0x0f, shape.a.size)
    end

  end

  def test_strings
    b = Class::new(LibBin::DataConverter) do
      string :h, 5
      string :w, offset: 0x10
    end

    open_bin("test_string.bin") do |f|
      s = b::load(f)
      assert_equal("Hello", s.h)
      assert_equal("World!\x00", s.w)
      shape = s.__shape
      assert_equal(0x17, shape.size)
      assert_equal(0x05, shape.h.size)
      assert_equal(0x07, shape.w.size)
      assert_equal(0x10, shape.w.first)
    end
  end

  def test_strings2
    b = Class::new(LibBin::DataConverter) do
      string :h, 32
      float :f
    end

    open_bin("test_string2.bin") do |f|
      s = b::load(f)
      assert_equal("Hello"+"\x00"*27, s.h)
      assert_equal(14.5, s.f)
      str = new_stringio
      b::dump(s, str)
      f.rewind
      str.rewind
      assert_equal(f.read, str.read)
      s.h = "Hello"
      str = new_stringio
      b::dump(s, str)
      f.rewind
      str.rewind
      assert_equal(f.read, str.read)
    end
  end

  def test_exception
    h = Class::new(LibBin::DataConverter) do
      int32 :offset1
      int16 :offset2
    end
    b = Class::new(LibBin::DataConverter) do
      int32 :datum1, offset: '..\header.offset1'
      int16 :datum2, offset: '..\header.offset2'
      int32 :error, length: 4
    end
    s = Class::new(LibBin::DataConverter) do
      register_field :header, h
      register_field :body, b
    end
    begin
      LibBin.__output = nil
      [false, true].each { |big|
        open_bin("test_size_#{SUFFIX[big]}.bin") do |f|
          struct = nil
          assert_raises {
            struct = s::load(f, big)
          }
          str = new_stringio
          f.rewind
          assert_raises {
            struct = s::convert(f, str, big, !big)
          }
        end
      }
    ensure
      LibBin.__output = $stderr
    end
  end

  def test_exception2
    h = Class::new(LibBin::DataConverter) do
      int32 :offset1
      int16 :offset2
    end
    b = Class::new(LibBin::DataConverter) do
      int32 :datum1, offset: '..\header.offset1'
      int16 :datum2, offset: '..\header.offset2'
    end
    s = Class::new(LibBin::DataConverter) do
      register_field :header, h
      register_field :body, b
    end
    begin
      LibBin.__output = nil
      [false, true].each { |big|
        open_bin("test_size_#{SUFFIX[big]}.bin") do |f|
          struct = s::load(f, big)
          struct.body.datum2 = nil
          str = new_stringio
          assert_raises {
            s::dump(struct, str)
          }
        end
      }
    ensure
      LibBin.__output = $stderr
    end
  end

  def test_bone_index_translate_table
    bitt = Class::new(LibBin::DataConverter) do
      int16 :first_level, length: 16
      int16 :second_level, length: 16, sequence: true, count: 16, condition: "first_level[__iterator] != -1"
      int16 :third_level, length: 16, sequence: true, count: "second_level.length * 16",
            condition: "second_level[__iterator/16] && second_level[__iterator/16][__iterator%16] != -1"
    end

    [false, true].each { |big|
      open_bin("bone_index_translate_table_#{SUFFIX[big]}.bin") do |f|
        t = bitt.load(f, big)
        assert_equal(0x220, bitt.size(t))
        str = new_stringio
        bitt.dump(t, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        open_bin("bone_index_translate_table_#{SUFFIX[!big]}.bin") do |g|
          str = new_stringio
          f.rewind
          t = bitt.convert(f, str, big, !big)
          str.rewind
          assert_equal(g.read, str.read)
        end
      end
    }
  end

  def test_bone_index_translate_table2
    bitt = Class::new(LibBin::DataConverter) do
      int16 :offsets, length: 16

      def __size(position = 0, parent = nil, index = nil)
        sz = super()
        if @second_levels
          @second_levels.each { |e|
            sz += e.__size(position, parent, index)
          }
        end
        if @third_levels
          @third_levels.each { |e|
            sz += e.__size(position, parent, index)
          }
        end
        sz
      end

      def __convert(input, output, input_big, output_big, parent, index, level = 1)
        __set_convert_type(input, output, input_big, output_big, parent, index)
        __convert_fields
        if level == 1
          @second_levels = []
          @offsets.each { |o|
            if o != -1
              t = self.class::new
              t.__convert(input, output, input_big, output_big, self, nil, level+1)
              @second_levels.push t
            end
          }
          @third_levels = []
          @second_levels.each { |l|
            l.offsets.each { |o|
              if o != -1
                t = self.class::new
                t.__convert(input, output, input_big, output_big, self, nil, level+2)
                @third_levels.push t
              end
            }
          }
        else
          @second_levels = nil
          @third_levels = nil
        end
        __unset_convert_type
        self
      end

      def __load(input, input_big, parent, index, level = 1)
        __set_load_type(input, input_big, parent, index)
        __load_fields
        if level == 1
          @second_levels = []
          @offsets.each { |o|
            if o != -1
              t = self.class::new
              t.__load(input, input_big, self, nil, level+1)
              @second_levels.push t
            end
          }
          @third_levels = []
          @second_levels.each { |l|
            l.offsets.each { |o|
              if o != -1
                t = self.class::new
                t.__load(input, input_big, self, nil, level+2)
                @third_levels.push t
              end
            }
          }
        else
          @second_levels = nil
          @third_levels = nil
        end
        __unset_load_type
        self
      end

      def __dump(output, output_big, parent, index, level = 1)
        __set_dump_type(output, output_big, parent, index)
        __dump_fields
        if @second_levels
          @second_levels.each { |e|
            e.__dump(output, output_big, self, nil, level+1)
          }
        end
        if @third_levels
          @third_levels.each { |e|
            e.__dump(output, output_big, self, nil, level+2)
          }
        end
        __unset_dump_type
      end

    end

    [false, true].each { |big|
      open_bin("bone_index_translate_table_#{SUFFIX[big]}.bin") do |f|
        t = bitt.load(f, big)
        str = new_stringio
        bitt.dump(t, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        open_bin("bone_index_translate_table_#{SUFFIX[!big]}.bin") do |g|
          str = new_stringio
          f.rewind
          t = bitt.convert(f, str, big, !big)
          str.rewind
          assert_equal(g.read, str.read)
        end
      end
    }
  end

end
