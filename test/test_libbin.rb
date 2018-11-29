[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'libbin'

class LibBinTest < Minitest::Test

  def test_simple_layout
    c = Class::new(LibBin::DataConverter) do
      int8 :a
      int8 :b
      int16 :c
      int32 :d
      float :e
    end

    File::open("binary/simple_layout_le.bin") do |f|
      s = c::load(f, false)
      assert_equal(1, s.a)
      assert_equal(0, s.b)
      assert_equal(2, s.c)
      assert_equal(3, s.d)
      assert_equal(4.0, s.e)
    end

    File::open("binary/simple_layout_be.bin") do |f|
      s = c::load(f, true)
      assert_equal(1, s.a)
      assert_equal(0, s.b)
      assert_equal(2, s.c)
      assert_equal(3, s.d)
      assert_equal(4.0, s.e)
    end
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
    end
  end

  def test_offset
    c = Class::new(LibBin::DataConverter) do
      int8 :b, offset: 1
      int32 :d, offset: 4
    end

    File::open("binary/simple_layout_le.bin") do |f|
      s = c::load(f, false)
      assert_equal(0, s.b)
      assert_equal(3, s.d)
    end
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
      int32 :data, count: 'header.count', offset: 'header.offset'
    end
    File::open("binary/offset_le.bin") do |f|
      s = b::load(f, false)
      assert_equal(32, s.header.offset)
      assert_equal(4, s.header.count)
      (0..3).each { |i|
        assert_equal(i+1, s.data[i])
      }
    end
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
    File::open("binary/offset_le.bin") do |f|
      s = b::load(f, false)
      assert_equal(32, s.header.offset)
      assert_equal(4, s.header.count)
      (0..3).each { |i|
        assert_equal(i+1, s.body.data[i])
      }
    end
  end

end
