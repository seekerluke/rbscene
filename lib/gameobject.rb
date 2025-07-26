module RBScene
    class GameObject
        def initialize
            # array of events per key
            @events = Hash.new { |h, k| h[k] = [] }

            # private C function used to store render properties for faster rendering
            @render_props = make_render_props(self.class.default_texture) if self.class.default_texture

            # change these to set render_props with the internal accessor functions
            @texture = self.class.default_texture
            @x, @y = self.class.default_position
            @width = @texture.width || 0
            @height = @texture.height || 0
            @angle = self.class.default_angle

            RBScene.scene.add(self)
            setup
        end

        # method stub to be overridden
        def setup
        end

        # method stub to be overridden
        def update
        end

        def input
            RBScene::Input.instance
        end

        def on(type_sym, &block)
            @events[type_sym] << block
        end

        def handle_event(type_sym, *args)
            @events[type_sym].each { |handler| handler.call(*args) }
        end

        class << self
            def texture(path)
                @default_texture = RBScene::Texture.load(path)
            end

            def position(x: 0, y: 0)
                @default_position = [x, y]
            end

            def angle(angle)
                @default_angle = angle
            end

            def default_texture
                @default_texture # can be nil
            end

            def default_position
                @default_position || [0, 0]
            end

            def default_angle
                @default_angle || 0
            end
        end
    end
end
