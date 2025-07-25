require "rbscene"

class Player < RBScene::GameObject
    sprite "player.png"
    position x: 50, y: 20
    angle 90

    def setup
        @sound = RBScene::Sound.load("jump.wav")
    end

    def update
        @sound.play if input.jump_press
    end
end
