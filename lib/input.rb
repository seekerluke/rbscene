require "singleton"

module RBScene
    class Input
        include Singleton

        def initialize
            @inputs = {}

            # default inputs
            define("up", [:up])
            define("down", [:down])
            define("left", [:left])
            define("right", [:right])
            define("space", [:space])
            define("space", [:z])
        end

        def define(name, codes)
            # overwrite the exisiting input if name already exists
            undefine(name) if @inputs[name]

            # eg. converts [:up, :space] to {up: [false, false, false], space: [false, false, false]}
            # this hash format is required by C processing
            # the bools correspond to [key_down, key_pressed, key_released]
            new_hash = codes.each_with_index.to_h { |sym| [sym, [false, false, false]] }
            @inputs[name] = new_hash
            
            # dynamic methods for checking state
            define_singleton_method(name) { @inputs[name].values.any? { |key_bools| key_bools[0] } }
            define_singleton_method("#{name}_press") { @inputs[name].values.any? { |key_bools| key_bools[1] } }
            define_singleton_method("#{name}_release") { @inputs[name].values.any? { |key_bools| key_bools[2] } }
        end

        def undefine(name)
            @inputs.delete(name)
            self.singleton_class.undef_method(name)
            self.singleton_class.undef_method("#{name}_press")
            self.singleton_class.undef_method("#{name}_release")
        end
    end
end
