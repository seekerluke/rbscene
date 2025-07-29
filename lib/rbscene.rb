require_relative "vector2"
require_relative "rect"
require_relative "texture"
require_relative "tickermanager"
require_relative "scene"
require_relative "gameobject"
require_relative "input"

# loads the C extension last, extension init function depends on previous classes existing on RBScene
require "rbscene/rbscene"
