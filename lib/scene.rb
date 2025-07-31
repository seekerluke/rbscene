module RBScene
    class Scene
        def initialize
            @objects = []
            setup
        end

        def create(type, x: 0, y: 0)
            gobj = type.new
            gobj.set_position(x: x, y: y)
            @objects.push(gobj)
        end

        # method stub to be overridden
        def setup
        end
    end
end
