module RBScene
  class Vector2
    attr_accessor :x, :y

    def initialize(x = 0, y = 0)
      @x = x
      @y = y
    end

    # operators

    def +(other)
      Vector2.new(@x + other.x, @y + other.y)
    end

    def -(other)
      Vector2.new(@x - other.x, @y - other.y)
    end

    def *(other)
      Vector2.new(@x * other, @y * other)
    end

    def /(other)
      Vector2.new(@x / other.to_f, @y / other.to_f)
    end

    def ==(other)
      other.is_a?(Vector2) && @x == other.x && @y == other.y
    end

    # helpers

    def to_s
      "Vector2(x: #{@x}, y: #{@y})"
    end

    def to_a
      [@x, @y]
    end
  end
end
