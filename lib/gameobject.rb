module RBScene
  class GameObject
    include RBScene

    attr_accessor :scene

    def initialize(x: nil, y: nil, width: nil, height: nil, angle: nil, frame: nil, hflip: nil, vflip: nil,
                   origin_x: nil, origin_y: nil)
      # array of events per key
      @events = Hash.new { |h, k| h[k] = [] }

      @texture = self.class.default_texture
      if @texture
        # make_render_props is a private, internal C function that creates a blank render props object
        @render_props = make_render_props(self.class.default_texture)

        default_x, default_y = self.class.default_position
        @render_props.x = x || default_x
        @render_props.y = y || default_y

        default_width, default_height = self.class.default_size(@texture)
        @render_props.width = width || default_width
        @render_props.height = height || default_height

        @render_props.angle = angle || self.class.default_angle

        @render_props.frame = frame || self.class.default_frame(@texture)

        @render_props.hflip = hflip || self.class.default_hflip
        @render_props.vflip = vflip || self.class.default_vflip

        @render_props.origin_x = origin_x || self.class.default_origin_x
        @render_props.origin_y = origin_y || self.class.default_origin_y
      end

      # each object's ticker manager should be updated globally
      @ticker_manager = TickerManager.new

      setup
    end

    # method stub to be overridden
    def setup
    end

    # method stub to be overridden
    def update
    end

    def destroy
      scene.destroy(self)
    end

    def ticker
      @ticker_manager
    end

    def get_texture
      @texture
    end

    def set_texture
      raise 'no impl for set_texture'
    end

    def get_position
      [@render_props.x, @render_props.y]
    end

    def set_position(x: 0, y: 0)
      @render_props.x = x
      @render_props.y = y
    end

    def get_size
      [@render_props.width, @render_props.height]
    end

    def set_size(width: 0, height: 0)
      @render_props.width = width
      @render_props.height = height
    end

    def get_angle
      @render_props.angle
    end

    def set_angle(angle)
      @render_props.angle = angle
    end

    def set_frame(rect)
      @render_props.frame = rect
    end

    def get_frame
      @render_props.frame
    end

    def get_hflip
      @render_props.hflip
    end

    def set_hflip(val)
      @render_props.hflip = val
    end

    def get_vflip
      @render_props.vflip
    end

    def set_vflip(val)
      @render_props.vflip = val
    end

    def get_origin
      [@render_props.origin_x, @render_props.origin_y]
    end

    def set_origin(x: 0, y: 0)
      @render_props.origin_x = x
      @render_props.origin_y = y
    end

    # helpers

    def inspect
      "GameObject(x: #{@render_props.x}, y: #{@render_props.y}, width: #{@render_props.width}, height: #{@render_props.height}, angle: #{@render_props.angle}, frame: #{@render_props.frame}, hflip: #{@render_props.hflip}, vflip: #{@render_props.vflip})"
    end

    class << self
      def texture(path)
        @default_texture = Assets.load_texture(path)
      end

      def position(x: 0, y: 0)
        @default_position = [x, y]
      end

      def size(width: 0, height: 0)
        @default_size = [width, height]
      end

      def angle(angle)
        @default_angle = angle
      end

      def frame(rect)
        @default_frame = rect
      end

      def origin(x: 0, y: 0)
        # TODO: This is inconsistent with the other setters. Which do you prefer?
        @default_origin_x = x
        @default_origin_y = y
      end

      attr_reader :default_texture

      def default_position
        @default_position || [0, 0]
      end

      def default_size(texture)
        @default_size || [texture&.width || 0, texture&.height || 0]
      end

      def default_angle
        @default_angle || 0
      end

      def default_frame(texture)
        @default_frame || Rect.new(0, 0, *default_size(texture))
      end

      def default_hflip
        @default_hflip || false
      end

      def default_vflip
        @default_vflip || false
      end

      def default_origin_x
        @default_origin_x || 0
      end

      def default_origin_y
        @default_origin_y || 0
      end
    end
  end
end
