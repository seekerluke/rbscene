module RBScene
  class Scene
    def initialize
      @objects = []

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

    def create(type, x: 0, y: 0)
      gobj = type.new(x: x, y: y)
      gobj.scene = self
      @objects.push(gobj)
      gobj
    end

    def get(type)
      @objects.find { |obj| obj.is_a?(type) }
    end

    def get_all(type)
      @objects.select { |obj| obj.is_a?(type) }
    end

    def destroy(obj)
      @objects.delete(obj)
    end

    def switch(scene_type)
      RBScene::Engine.switch_scene(scene_type)
    end

    # method stub to be overridden
    def setup
    end

    class << self
      attr_reader :music_path

      def music(path)
        @music_path = path
      end
    end
  end
end
