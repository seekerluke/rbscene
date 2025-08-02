module RBScene
    class Engine
        class Config
            attr_accessor :window_title, :window_size, :start_scene

            def initialize
                @window_title = "Untitled"
                @window_size = [800, 600]
                @start_scene = nil # should maybe pick the first scene in the directory?
            end
        end

        @config = Config.new

        class << self
            def configure
                yield @config if block_given?
                @current_scene = @config.start_scene.new
            end

            def scene
                @current_scene
            end

            def switch_scene(next_scene)
                @current_scene = next_scene.new
            end

            def window_size
                @config.window_size
            end

            def window_rect
                Rect.new(0, 0, *@config.window_size)
            end
        end
    end
end