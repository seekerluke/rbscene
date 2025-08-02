module RBScene
    class Rect
        # Rects are backed by raylib Rectangles
        # initialize is written in C and takes (x, y, width, height)
        # getters and setters for x, y, w, h are all written in C

        # edges

        def top
            self.y
        end

        def top=(value)
            self.y = value
        end

        def left
            self.x
        end

        def left=(value)
            self.x = value
        end

        def bottom
            self.y + self.h
        end

        def bottom=(value)
            self.y = value - self.h
        end

        def right
            self.x + self.w
        end

        def right=(value)
            self.x = value - self.w
        end

        # corners

        def topleft
            Vector2.new(left, top)
        end

        def topleft=(vec)
            self.left, self.top = vec
        end

        def bottomleft
            Vector2.new(left, bottom)
        end

        def bottomleft=(vec)
            self.left, self.bottom = vec
        end

        def topright
            Vector2.new(right, top)
        end

        def topright=(vec)
            self.right, self.top = vec
        end

        def bottomright
            Vector2.new(right, bottom)
        end

        def bottomright=(vec)
            self.right, self.bottom = vec
        end

        # midpoints

        def midtop
            Vector2.new(centerx, top)
        end

        def midtop=(vec)
            self.centerx, self.top = vec
        end

        def midbottom
            Vector2.new(centerx, bottom)
        end

        def midbottom=(vec)
            self.centerx, self.bottom = vec
        end

        def midleft
            Vector2.new(left, centery)
        end

        def midleft=(vec)
            self.left, self.centery = vec
        end

        def midright
            Vector2.new(right, centery)
        end

        def midright=(vec)
            self.right, self.centery = vec
        end

        # center

        def center
            Vector2.new(centerx, centery)
        end

        def center=(vec)
            self.centerx, self.centery = vec
        end

        def centerx
            self.x + self.w / 2.0
        end

        def centerx=(value)
            self.x = value - self.w / 2.0
        end

        def centery
            self.y + self.h / 2.0
        end

        def centery=(value)
            self.y = value - self.h / 2.0
        end

        # size

        def size
            Vector2.new(self.w, self.h)
        end

        def size=(vec)
            self.w, self.h = vec
        end

        def width
            self.w
        end

        def width=(val)
            self.w = val
        end

        def height
            self.h
        end

        def height=(val)
            self.h = val
        end

        # collision

        def collides?(other)
            return false if other.nil?

            self.x < other.x + other.w &&
            self.x + self.w > other.x &&
            self.y < other.y + other.h &&
            self.y + self.h > other.y
        end

        # helpers

        def to_s
            "Rect(x: #{self.x}, y: #{self.y}, w: #{self.w}, h: #{self.h})"
        end

        def to_a
            [self.x, self.y, self.w, self.h]
        end
    end
end