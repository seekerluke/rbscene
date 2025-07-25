module RBScene
    class GameObject
        @shared_sprite = nil

        def initialize
            # array of events per key
            @events = Hash.new { |h, k| h[k] = [] }
            @sprite = self.class.shared_sprite # can be nil

            # add to scene on creation
            RBScene.scene.add(self)
        end

        def on(type_sym, &block)
            @events[type_sym] << block
        end

        def handle_event(type_sym, *args)
            @events[type_sym].each { |handler| handler.call(*args) }
        end

        class << self
            def sprite(path)
                @shared_sprite = RBScene::Texture.load(path)
            end

            def shared_sprite
                @shared_sprite
            end
        end
    end
end
