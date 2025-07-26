require "rbscene"
require_relative "background"
require_relative "player"

RBScene::Music.load("music.mp3")
RBScene::Music.play

bg = Background.new
player = Player.new

RBScene::Engine.run
