module RBScene
    class Rect
        attr_accessor :x, :y, :w, :h

        # Rects are backed by raylib Rectangles
        # initialize is written in C and takes (x, y, width, height)
        # instance variables are set, attr_accessors work

        # edges

        def top
            @y
        end

        def top=(value)
            @y = value
        end

        def left
            @x
        end

        def left=(value)
            @x = value
        end

        def bottom
            @y + @h
        end

        def bottom=(value)
            @y = value - @h
        end

        def right
            @x + @w
        end

        def right=(value)
            @x = value - @w
        end

        # corners

        def topleft
            Vector2.new(left, top)
        end

        def topleft=(vec)
            self.left = vec.x
            self.top = vec.y
        end

        def bottomleft
            Vector2.new(left, bottom)
        end

        def bottomleft=(vec)
            self.left = vec.x
            self.bottom = vec.y
        end

        def topright
            Vector2.new(right, top)
        end

        def topright=(vec)
            self.right = vec.x
            self.top = vec.y
        end

        def bottomright
            Vector2.new(right, bottom)
        end

        def bottomright=(vec)
            self.right = vec.x
            self.bottom = vec.y
        end

        # midpoints

        def midtop
            Vector2.new(centerx, top)
        end

        def midtop=(vec)
            self.centerx = vec.x
            self.top = vec.y
        end

        def midbottom
            Vector2.new(centerx, bottom)
        end

        def midbottom=(vec)
            self.centerx = vec.x
            self.bottom = vec.y
        end

        def midleft
            Vector2.new(left, centery)
        end

        def midleft=(vec)
            self.left = vec.x
            self.centery = vec.y
        end

        def midright
            Vector2.new(right, centery)
        end

        def midright=(vec)
            self.right = vec.x
            self.centery = vec.y
        end

        # center

        def center
            Vector2.new(centerx, centery)
        end

        def center=(vec)
            self.centerx = vec.x
            self.centery = vec.y
        end

        def centerx
            @x + @w / 2.0
        end

        def centerx=(value)
            @x = value - @w / 2.0
        end

        def centery
            @y + @h / 2.0
        end

        def centery=(value)
            @y = value - @h / 2.0
        end

        # size

        def size
            Vector2.new(@w, @h)
        end

        def size=(vec)
            @w = vec.x
            @h = vec.y
        end

        def width
            @w
        end

        def width=(val)
            @w = val
        end

        def height
            @h
        end

        def height=(val)
            @h = val
        end

        # helpers

        def to_s
            "Rect(x: #{@x}, y: #{@y}, w: #{@w}, h: #{@h})"
        end

        def to_a
            [@x, @y, @w, @h]
        end
    end
end