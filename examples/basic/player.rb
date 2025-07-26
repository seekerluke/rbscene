class Player < RBScene::GameObject
    texture "player.png"
    position x: 35, y: 112
    size width: 16, height: 16
    frame_rect Rect.new(0, 0, 16, 16)

    def setup
        @sound = Sound.load("jump.wav")
    end

    def update
        @sound.play if input.space_press
    end
end
