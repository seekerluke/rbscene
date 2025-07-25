require_relative "scene"
require_relative "gameobject"

# loads the C extension last, extension init function depends on previous classes existing on RBScene
require "rbscene/rbscene"
