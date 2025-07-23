require "rbscene"

class Player
    def initialize
        @x = 100
        @y = 100
    end

    def update
        @x += 1
    end

    def draw
        RBScene::Bindings.draw_rectangle(@x, @y, 50, 50)
    end
end

RBScene::Engine.run([Player.new])
