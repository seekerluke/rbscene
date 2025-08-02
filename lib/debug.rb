module RBScene
  class Debug
    @rects = []

    class << self
      def add_rect(rect)
        @rects.push(rect)
      end

      def remove_rect(rect)
        @rects.delete(rect)
      end
    end
  end
end
