require "rbscene/rbscene" # loads the C extension

module RBEngine
    class GameObject
        def initialize
            # array of events per key
            @events = Hash.new { |h, k| h[k] = [] }
        end

        def on(type_sym, &block)
            @events[type_sym] << block
        end

        def handle_event(type_sym, *args)
            @events[type_sym].each { |handler| handler.call(*args) }
        end
    end
end
