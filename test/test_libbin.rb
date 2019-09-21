[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'libbin'
require 'stringio'

class LibBinTest < Minitest::Test
  SUFFIX = { true => "be", false => "le" }

  def test_half
    c = Class::new(LibBin::DataConverter) do
      register_field :a, :half, count: 4
      register_field :b, :pghalf, length: 4
    end

    [true, false].each { |big|
      File::open("binary/half_#{SUFFIX[big]}.bin") do |f|
        s = c::load(f, big)
        assert_equal( (1..4).to_a, s.a )
        assert_equal( (1..4).to_a, s.b )
        str = StringIO::new
        c::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        File::open("binary/half_#{SUFFIX[!big]}.bin") do |g|
          str = StringIO::new
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
      register_field :a, :half_le, length: 4
      register_field :b, :pghalf_le, count: 4
    end

    File::open("binary/half_le.bin") do |f|
      s = c::load(f, true)
      assert_equal( (1..4).to_a, s.a )
      assert_equal( (1..4).to_a, s.b )
      str = StringIO::new
      c::dump(s, str, true)
      f.rewind
      str.rewind
      assert_equal(f.read, str.read)
      f.rewind
      str = StringIO::new
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

    File::open("binary/half_be.bin") do |f|
      s = c::load(f, false)
      assert_equal( (1..4).to_a, s.a )
      assert_equal( (1..4).to_a, s.b )
      str = StringIO::new
      c::dump(s, str, false)
      f.rewind
      str.rewind
      assert_equal(f.read, str.read)
      f.rewind
      str = StringIO::new
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
      File::open("binary/simple_layout_#{SUFFIX[big]}.bin") do |f|
        s = c::load(f, big)
        assert_equal(1, s.a)
        assert_equal(0, s.b)
        assert_equal(2, s.c)
        assert_equal(3, s.d)
        assert_equal(4.0, s.e)
        str = StringIO::new
        c::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        File::open("binary/simple_layout_#{SUFFIX[!big]}.bin") do |g|
          str = StringIO::new
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
      File::open("binary/simple_layout_le.bin") do |f|
        s = c::load(f, big)
        assert_equal(1, s.a)
        assert_equal(0, s.b)
        assert_equal(2, s.c)
        assert_equal(3, s.d)
        assert_equal(4.0, s.e)
        str = StringIO::new
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
      File::open("binary/simple_layout_be.bin") do |f|
        s = c::load(f, big)
        assert_equal(1, s.a)
        assert_equal(0, s.b)
        assert_equal(2, s.c)
        assert_equal(3, s.d)
        assert_equal(4.0, s.e)
        str = StringIO::new
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
      File::open("binary/simple_layout_#{SUFFIX[big]}.bin") do |f|
        s = c::load(f, big)
        assert_equal(1, s.a)
        assert_equal(0, s.b)
        assert_equal(2, s.c)
        assert_equal(3, s.d)
        assert_equal(4.0, s.e)
        str = StringIO::new
        c::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        File::open("binary/simple_layout_#{SUFFIX[!big]}.bin") do |g|
          str = StringIO::new
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

    File::open("binary/simple_array.bin") do |f|
      s = c::load(f)
      assert_equal(15,  s.num)
      assert_equal(s.num, s.a.length)
      s.a.each_with_index { |e, i|
         assert_equal(i, e)
      }
      str = StringIO::new
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
      File::open("binary/simple_layout_#{SUFFIX[big]}.bin") do |f|
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
      File::open("binary/simple_layout_#{SUFFIX[big]}.bin") do |f|
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
      File::open("binary/sequence_layout_#{SUFFIX[big]}.bin") do |f|
        s = c::load(f, big)
        assert_equal((1..6).to_a, s.data)
        str = StringIO::new
        c::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        File::open("binary/sequence_layout_#{SUFFIX[!big]}.bin") do |g|
          str = StringIO::new
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
      File::open("binary/sequence_layout_#{SUFFIX[big]}.bin") do |f|
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
      File::open("binary/sequence_null_layout_#{SUFFIX[big]}.bin") do |f|
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

    File::open("binary/simple_array.bin") do |f|
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
      File::open("binary/offset_#{SUFFIX[big]}.bin") do |f|
        s = b::load(f, big)
        assert_equal(32, s.header.offset)
        assert_equal(4, s.header.count)
        (0..3).each { |i|
          assert_equal(i+1, s.data[i])
        }
        str = StringIO::new
        b::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        File::open("binary/offset_#{SUFFIX[!big]}.bin") do |g|
          str = StringIO::new
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
      File::open("binary/offset_#{SUFFIX[big]}.bin") do |f|
        s = b::load(f, big)
        assert_equal(32, s.header.offset)
        assert_equal(4, s.header.count)
        (0..3).each { |i|
          assert_equal(i+1, s.body.data[i])
        }
        str = StringIO::new
        b::dump(s, str, big)
        f.rewind
        str.rewind
        assert_equal(f.read, str.read)
        File::open("binary/offset_#{SUFFIX[!big]}.bin") do |g|
          str = StringIO::new
          f.rewind
          s = b::convert(f, str, big, !big)
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
      int32 :datum1, offset: '..\header.offset1'
      int16 :datum2, offset: '..\header.offset2'
    end
    s = Class::new(LibBin::DataConverter) do
      register_field :header, h
      register_field :body, b
    end
    [false, true].each { |big|
      File::open("binary/test_size_#{SUFFIX[big]}.bin") do |f|
        str = s::load(f, big)
        assert_equal(1, str.body.datum1)
        assert_equal(2, str.body.datum2)
        assert_equal(0x22, str.__size)
        shape = str.__shape
        assert_equal(0x22, shape.size)
        assert_equal(0x06, shape.header.size)
        assert_equal(0x00, shape.header.first)
        assert_equal(0x05, shape.header.last)
        assert_equal(0x12, shape.body.size)
        assert_equal(0x10, shape.body.first)
        assert_equal(0x21, shape.body.last)
      end
    }

    c = Class::new(LibBin::DataConverter) do
      int8 :num
      int8 :a, count: 'num'
    end

    File::open("binary/simple_array.bin") do |f|
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

    File::open("binary/test_string.bin") do |f|
      s = b::load(f)
      assert_equal("Hello", s.h)
      assert_equal("World!\x00", s.w)
      shape = s.__shape
      assert_equal(0x17, shape.size)
      assert_equal(0x05, shape.h.size)
      assert_equal(0x07, shape.w.size)
    end
  end

end
