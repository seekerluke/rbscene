Gem::Specification.new do |spec|
  spec.name          = "rbscene"
  spec.version       = "0.0.1"
  spec.summary       = "An extremely minimal game engine with Ruby scripting."
  spec.description   = "A Ruby gem that provides a 2D game engine based on raylib."
  spec.authors       = ["Luke Lazzaro"]
  spec.email         = ["example@email.com"]
  spec.homepage      = "https://github.com/seekerluke/rbscene"
  spec.license       = "MIT"

  spec.files         = Dir["lib/**/*"] +
                       Dir["ext/**/*.{c,h,rb}"]

  spec.extensions    = ["ext/rbscene/extconf.rb"]

  spec.required_ruby_version = ">= 2.7.0"
end
