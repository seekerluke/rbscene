class Player < RBScene::GameObject
    texture "player.png"
    position x: 50, y: 20

    def setup
        set_size width: 32, height: 32
        set_frame_rect Rect.new(0, 0, 32, 32)
        @sound = Sound.load("jump.wav")
    end

    def update
        @sound.play if input.space_press
    end
end
