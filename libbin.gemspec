Gem::Specification.new do |s|
  s.name = 'libbin'
  s.version = "1.0.2"
  s.author = "Brice Videau"
  s.email = "brice.videau@imag.fr"
  s.homepage = "https://github.com/kerilk/libbin"
  s.summary = "Library for loading and converting binary files"
  s.description = "Read, write and convert Binary data in Ruby."
  s.files =  Dir[ 'libbin.gemspec', 'LICENSE', 'lib/**/*.rb', 'ext/libbin/extconf.rb', 'ext/libbin/*.c', 'ext/libbin/*.h' ]
  s.extensions << 'ext/libbin/extconf.rb'
  s.has_rdoc = false
  s.license = 'BSD-2-Clause'
  s.required_ruby_version = '>= 2.0.0'
  s.add_dependency 'float-formats', '~> 0.3', '>=0.3.0'
end
