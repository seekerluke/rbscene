# frozen_string_literal: true

module RBScene
  class Scene
    include RBScene

    attr_accessor :camera

    def initialize
      @objects = []
      @ui_objects = []
      @camera = Camera.new

      # stop if empty string is specified
      # continue playing previous music if nil
      path = self.class.music_path
      if path.is_a?(String) && !path.empty?
        Assets.load_music(path).play
      elsif path == ''
        RBScene::Music.stop
      end

      setup
    end

    def create(type, x: 0, y: 0, ui: false, **kwargs)
      gobj = type.new(x: x, y: y, **kwargs)
      gobj.scene = self

      if ui
        @ui_objects.push(gobj)
      else
        @objects.push(gobj)
      end

      gobj
    end

    # TODO: these get methods need to take ui objects into account
    def get(type)
      @objects.find { |obj| obj.is_a?(type) }
    end

    def get_all(type)
      @objects.select { |obj| obj.is_a?(type) }
    end

    def destroy(obj)
      @objects.delete(obj)
      @ui_objects.delete(obj)
    end

    def switch(scene_type)
      RBScene::Engine.switch_scene(scene_type)
    end

    # method stub to be overridden
    def setup; end

    # method stub to be overridden
    def update; end

    class << self
      attr_reader :music_path

      def music(path)
        @music_path = path
      end
    end
  end
end
