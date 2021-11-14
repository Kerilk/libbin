# libbin
Read, write and convert Binary data in Ruby.

Detailed documentation can be found here:
https://kerilk.github.io/libbin/

## Philosophy

The goal of libbin is to provide a declarative way to describe binary files
formats. But, as this declarative approach is not always feasible or efficient,
it also provides more procedural approaches through a series of hooks that can
be used to override the behavior of the library.

## Example

In order to showcase usages of the library, a real life example is taken. This
examples reads motion files from recent PlatinumGames games (Nier Automata,
Bayonnetta2, ...). The format description can be found here:
https://github.com/Kerilk/bayonetta_tools/wiki/Motion-Formats-(mot-files)#nier-automata-and-transformers-devastation

```ruby
require 'libbin'

# The file is seen as big structure. Sub structures will be declared inside.
# contrary to what is done in the real implementation, the order of the
# declarations will be shown in the order they are the most interesting.
class MOT2File < LibBin::Structure

  # The file header.
  class Header < LibBin::Structure
    # First field if a 4 character identifier string that is expected to be "mot\x00"
    string :id, 4, expect: "mot\x00"
    uint32 :version
    uint16 :flags
    uint16 :frame_count
    uint32 :offset_records
    uint32 :num_records
    int8 :u_a, length: 4
    string :name, 16
  end

  # Register the header into our file structure
  field :header, Header

  # The animation records corresponding to the different animation tracks affected by
  # the animation
  class Record < LibBin::Structure
    int16 :bone_index
    int8 :animation_track
    int8 :format
    int16 :num_keys
    # field is aligned on it's size byte boundary
    uint32 :offset, align: true
    
    # The previous field is also a union. LibBin does not support these for now,
    # but getting around the problem is not particularly difficult:
    def value
      [offset].pack("L").unpack("F").first
    end
    
    def value=(v)
      self.offset = [v].pack("F").unpack("L").first
      v
    end
    
    # the size of this structure is known (Some structure's size can only be known
    # at runtime.
    def self.size(*args)
      12
    end
  end
  
  # Register the records field.
  # The required count and offset are contained in the header.
  # Here we showcase two different ways to express those constraints,
  # using a string representing the path, or a proc accessing the
  # class instance members.
  # There is an additional dummy record at he end of the list.
  field :records, Record, count: 'header\num_records + 1', offset: proc { header.offset_records }
  
  # The format defines many way to encode the animations tracks, with
  # incresing levels of compression, and many use interpolation with
  # cubic Hermit splines. Unless the animation track is constant
  # (format 0 (or -1 for dummy) in the record)
  
  # Format 1: 1 float value per key
  # value[frame] = keys[frame]
  class Format1 < LibBin::Structure
    # The number of keys is defined in the corresponding record.
    # Our rank in the repetition gives the index in the record vector.
    float :keys, count: '..\records[__index]\num_keys'
  end
  
  # Format 2: quantized values using a 16 bit integer mutiplier
  # value[frame] = p + key[frame] * dp
  class Format2 < LibBin::Structure
    float :p
    float :dp
    uint16 :keys, count: '..\records[__index]\num_keys'
  end
  
  # Format 3: quantized values using sepecial half floats.
  # value[frame] = p + key[frame] * dp
  class Format3 < LibBin::Structure
    pghalf :p
    pghalf :dp
    uint16 :keys, count: '..\records[__index]\num_keys'
  end
  
  # Format 4: Cubic hermit splines.
  class Format4 < LibBin::Structure
    class Key < LibBin::Structure
      uint16 :index
      uint16 :padding #Could use align on the next field as well
      float :p
      float :m0
      float :m1
    end
    field :keys, Key, count: '..\records[__index]\num_keys'
  end
  
  # Format 5: Cubic hermit splines with quantization
  class Format5 < LibBin::Structure
    class Key < LibBin::Structure
      uint16 :index
      uint16 :cp
      uint16 :cm0
      uint16 :cm1
    end
    float :p
    float :dp
    float :m0
    float :dm0
    float :m1
    float :dm1
    field :keys, Key, count: '..\records[__index]\num_keys'
  end
  
  # Format 6: Cubic hermit spline with quantization and
  # special half floats
  class Format6 < LibBin::Structure
    class Key < LibBin::Structure
      uint8 :index
      uint8 :cp
      uint8 :cm0
      uint8 :cm1
    end
    pghalf :p
    pghalf :dp
    pghalf :m0
    pghalf :dm0
    pghalf :m1
    pghalf :dm1
    register_field :keys, Key, count: '..\records[__index]\num_keys'
  end
  
  # Format 7: same as 6 with relative indices
  # Structure is exactly the same
  class Format7 < LibBin::Structure
    class Key < LibBin::Structure
      uint8 :index
      uint8 :cp
      uint8 :cm0
      uint8 :cm1
    end
    pghalf :p
    pghalf :dp
    pghalf :m0
    pghalf :dm0
    pghalf :m1
    pghalf :dm1
    register_field :keys, Key, count: '..\records[__index]\num_keys'
  end
  
  # Format 8: same as 6 but with a bigger index. This index is always store in big endian fashion
  class Format8 < LibBin::Structure
    class Key < LibBin::Structure
      uint16_be :index
      uint8 :cp
      uint8 :cm0
      uint8 :cm1
    end
        pghalf :p
    pghalf :dp
    pghalf :m0
    pghalf :dm0
    pghalf :m1
    pghalf :dm1
    register_field :keys, Key, count: '..\records[__index]\num_keys'
  end
  
  FORMATS = [nil, Format1, Format2, Format3, Format4, Format5, Format6, Format7, Format8]
  def format(i)
    FORMATS[i]
  end
  
  # register the tracks, they use relative offsets to the record position in the file
  # note that the type is dynamic and depends on the recod content, so we use the
  # sequence flag to tell libbin that the type and offset needs to be evaluated for
  # each repetition
  field :tracks, proc { format(records[__iterator].format) }, count: 'header\num_records',
        sequence: true, condition: proc { records[__iterator].format != 0 && records[__iterator].format != -1 },
        offset: proc { header.offset_records + Record.size * __iterator + records[__iterator].offset }
end
```
