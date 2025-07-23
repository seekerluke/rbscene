require "rbscene"

class Player < RBEngine::GameObject
    def initialize(texture, music)
        @texture = texture
        @music = music
        super() # annoying
    end

    def update
        @music.update
    end

    def draw
        @texture.draw 0, 0
    end
end

texture = RBScene::Texture.load("scene.png")

music = RBScene::Music.load("music.mp3")
music.play

sound = RBScene::Sound.load("jump.wav")
sound.play

player = Player.new texture, music

player.on :keypress do |key|
    sound.play if key == :space
    puts "enter" if key == :enter
end

player.on :keydown do |key|
    puts "DOWN" if key == :down
end

RBScene::Engine.run([player])
