module RBScene
    class Vector2
        attr_accessor :x, :y

        def initialize(x = 0, y = 0)
            @x, @y = x, y
        end

        # operators

        def +(other)
            Vector2.new(@x + other.x, @y + other.y)
        end

        def -(other)
            Vector2.new(@x - other.x, @y - other.y)
        end

        def *(scalar)
            Vector2.new(@x * scalar, @y * scalar)
        end

        def /(scalar)
            Vector2.new(@x / scalar.to_f, @y / scalar.to_f)
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