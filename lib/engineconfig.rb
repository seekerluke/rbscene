module RBScene
    class EngineConfig
        attr_accessor :window_title, :window_size, :start_scene

        def initialize
            @window_title = "Untitled"
            @window_size = [800, 600]
            @start_scene = nil # should maybe pick the first scene in the directory?
        end
    end
end
