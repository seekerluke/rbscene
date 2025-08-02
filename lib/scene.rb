module RBScene
    class Scene
        def initialize
            @objects = []
            setup
        end

        def create(type, x: 0, y: 0)
            gobj = type.new(x: x, y: y)
            gobj.scene = self
            @objects.push(gobj)
            gobj
        end

        def get(type)
            @objects.find { |obj| obj.is_a?(type) }
        end

        def get_all(type)
            @objects.select { |obj| obj.is_a?(type) }
        end

        def destroy(obj)
            @objects.delete(obj)
        end

        # method stub to be overridden
        def setup
        end
    end
end
