require "singleton"

module RBScene
    class Input
        include Singleton

        def initialize
            @inputs = {}
        end

        def define(name, codes)
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
        end
    end
end
