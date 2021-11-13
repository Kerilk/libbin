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
    int8 :interpolation_type
    int16 :num_keys
    # This field is 4 byte aligned
    uint32 :offset, align: true
  end
  
  # Register the records field.
  # The required count and offset are contained in the header.
  # Here we showcase two different ways to express those constraints,
  # using a string representing the path, or a proc accessing the
  # class instance members.
  field :records, Record, count: 'header\num_records', offset: proc { header.offset_records }
  
end
```
