[ '../lib', 'lib' ].each { |d| $:.unshift(d) if File::directory?(d) }
require 'minitest/autorun'
require 'libbin'
warn_level = $VERBOSE
$VERBOSE = nil
require 'float-formats'
$VERBOSE = warn_level

Flt::IEEE.binary :IEEE_binary16_pg, significand: 9, exponent: 6, bias: 47
Flt::IEEE.binary :IEEE_binary16_pg_BE, significand: 9, exponent: 6, bias: 47, endianness: :big_endian

class LibBinHalfTest < Minitest::Test

  def test_half
    nan = "\x00\x7e".force_encoding(Encoding::ASCII_8BIT)
    nans = "\x00\xfe".force_encoding(Encoding::ASCII_8BIT)
    256.times { |i|
      256.times { |j|
        str = eval("\"\\x%02x\\x%02x\"" % [j, i]).force_encoding(Encoding::ASCII_8BIT)
        val = LibBin::half_from_string(str, "S<")
        if val.nan? && (i & 0x80 == 0)
          assert_equal( nan, LibBin::half_to_string(val, "S<") )
          assert( Flt::IEEE_binary16.from_bytes(str).to(Float).nan? )
        elsif val.nan?
          assert_equal( nans, LibBin::half_to_string(val, "S<") )
          assert( Flt::IEEE_binary16.from_bytes(str).to(Float).nan? )
        else
          assert_equal( str, LibBin::half_to_string(val, "S<") )
          assert_equal( val, Flt::IEEE_binary16.from_bytes(str).to(Float) )
        end
      }
    }
  end

  def test_pghalf
    nan = "\x00\x7f".force_encoding(Encoding::ASCII_8BIT)
    nans = "\x00\xff".force_encoding(Encoding::ASCII_8BIT)
    256.times { |i|
      256.times { |j|
        str = eval("\"\\x%02x\\x%02x\"" % [j, i]).force_encoding(Encoding::ASCII_8BIT)
        val = LibBin::pghalf_from_string(str, "S<")
        if val.nan? && (i & 0x80 == 0)
          assert_equal( nan, LibBin::pghalf_to_string(val, "S<") )
          assert( Flt::IEEE_binary16_pg.from_bytes(str).to(Float).nan? )
        elsif val.nan?
          assert_equal( nans, LibBin::pghalf_to_string(val, "S<") )
          assert( Flt::IEEE_binary16_pg.from_bytes(str).to(Float).nan? )
        else
          assert_equal( str, LibBin::pghalf_to_string(val, "S<") )
          assert_equal( val, Flt::IEEE_binary16_pg.from_bytes(str).to(Float) )
        end
      }
    }
  end

  def test_half_big
    nan = "\x7e\x00".force_encoding(Encoding::ASCII_8BIT)
    nans = "\xfe\x00".force_encoding(Encoding::ASCII_8BIT)
    256.times { |i|
      256.times { |j|
        str = eval("\"\\x%02x\\x%02x\"" % [i, j]).force_encoding(Encoding::ASCII_8BIT)
        val = LibBin::half_from_string(str, "S>")
        if val.nan? && (i & 0x80 == 0)
          assert_equal( nan, LibBin::half_to_string(val, "S>") )
          assert( Flt::IEEE_binary16_BE.from_bytes(str).to(Float).nan? )
        elsif val.nan?
          assert_equal( nans, LibBin::half_to_string(val, "S>") )
          assert( Flt::IEEE_binary16_BE.from_bytes(str).to(Float).nan? )
        else
          assert_equal( str, LibBin::half_to_string(val, "S>") )
          assert_equal( val, Flt::IEEE_binary16_BE.from_bytes(str).to(Float) )
        end
      }
    }
  end

  def test_pghalf_big
    nan = "\x7f\x00".force_encoding(Encoding::ASCII_8BIT)
    nans = "\xff\x00".force_encoding(Encoding::ASCII_8BIT)
    256.times { |i|
      256.times { |j|
        str = eval("\"\\x%02x\\x%02x\"" % [i, j]).force_encoding(Encoding::ASCII_8BIT)
        val = LibBin::pghalf_from_string(str, "S>")
        if val.nan? && (i & 0x80 == 0)
          assert_equal( nan, LibBin::pghalf_to_string(val, "S>") )
          assert( Flt::IEEE_binary16_pg_BE.from_bytes(str).to(Float).nan? )
        elsif val.nan?
          assert_equal( nans, LibBin::pghalf_to_string(val, "S>") )
          assert( Flt::IEEE_binary16_pg_BE.from_bytes(str).to(Float).nan? )
        else
          assert_equal( str, LibBin::pghalf_to_string(val, "S>") )
          assert_equal( val, Flt::IEEE_binary16_pg_BE.from_bytes(str).to(Float) )
        end
      }
    }
  end

end
