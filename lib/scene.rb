module RBScene
    class Scene
        def initialize
            @objects = []
        end

        def add(obj)
            @objects.append obj
        end
    end

    # Global on RBScene... maybe there's a better way to store these? RBScene will expand quickly.
    @current_scene = Scene.new

    class << self
        def scene
            @current_scene
        end

        def switch_scene(next_scene)
            @current_scene = next_scene
        end
    end
end
