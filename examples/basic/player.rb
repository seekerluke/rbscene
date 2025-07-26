class Player < RBScene::GameObject
    texture "player.png"
    position x: 50, y: 20
    angle 90

    def setup
        @sound = RBScene::Sound.load("jump.wav")
    end

    def update
        @sound.play if input.space_press
    end
end
