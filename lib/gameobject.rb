module RBScene
    class GameObject
        @default_sprite = nil

        def initialize
            # array of events per key
            @events = Hash.new { |h, k| h[k] = [] }

            @sprite = self.class.default_sprite
            @x, @y = self.class.default_position
            @width = @sprite.width || 0
            @height = @sprite.height || 0
            @angle = self.class.default_angle

            RBScene.scene.add(self)
        end

        # method stub to be overridden
        def update
        end

        def on(type_sym, &block)
            @events[type_sym] << block
        end

        def handle_event(type_sym, *args)
            @events[type_sym].each { |handler| handler.call(*args) }
        end

        class << self
            def sprite(path)
                @default_sprite = RBScene::Texture.load(path)
            end

            def position(x: 0, y: 0)
                @default_position = [x, y]
            end

            def angle(angle)
                @default_angle = angle
            end

            def default_sprite
                @default_sprite # can be nil
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
