module RBScene
  class Input
    @inputs = {}

    class << self
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

        # TODO: need to change how methods are defined, not a singleton anymore
      end

      def undefine(name)
        @inputs.delete(name)
        singleton_class.undef_method(name)
        singleton_class.undef_method("#{name}_press")
        singleton_class.undef_method("#{name}_release")
      end
    end
  end
end
