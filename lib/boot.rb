# frozen_string_literal: true

# This script is the entry point to your application.

require 'rbscene'

RBScene::Engine.init

# require gameobjects
gameobjects = Dir.glob(File.join(Dir.pwd, 'gameobjects', '**', '*.rb')).sort
if gameobjects.empty?
  warn 'Warning: No game objects found!'
else
  gameobjects.each { |file| require file }
end

# require scenes
scenes = Dir.glob(File.join(Dir.pwd, 'scenes', '**', '*.rb')).sort
if scenes.empty?
  warn 'Warning: No scenes found!'
else
  scenes.each { |file| require file }
end

# configure engine
config = File.expand_path('config.rb', Dir.pwd)
require config if File.exist?(config)
RBScene::Engine.update

# default inputs
RBScene::Input.define('up', [:up])
RBScene::Input.define('down', [:down])
RBScene::Input.define('left', [:left])
RBScene::Input.define('right', [:right])
RBScene::Input.define('space', [:space])

RBScene::Engine.run
