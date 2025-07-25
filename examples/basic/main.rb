require "rbscene"
require_relative "background"
require_relative "player"

# music = RBScene::Music.load("music.mp3")
# music.play

sound = RBScene::Sound.load("jump.wav")
sound.play

bg = Background.new
player = Player.new

RBScene::Engine.run
