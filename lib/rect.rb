# frozen_string_literal: true

module RBScene
  class Rect
    # Rects are backed by raylib Rectangles
    # initialize is written in C and takes (x, y, width, height)
    # getters and setters for x, y, w, h are all written in C

    # edges

    def top
      y
    end

    def top=(value)
      self.y = value
    end

    def left
      x
    end

    def left=(value)
      self.x = value
    end

    def bottom
      y + h
    end

    def bottom=(value)
      self.y = value - h
    end

    def right
      x + w
    end

    def right=(value)
      self.x = value - w
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
      x + w / 2.0
    end

    def centerx=(value)
      self.x = value - w / 2.0
    end

    def centery
      y + h / 2.0
    end

    def centery=(value)
      self.y = value - h / 2.0
    end

    # size

    def size
      Vector2.new(w, h)
    end

    def size=(vec)
      self.w, self.h = vec
    end

    def width
      w
    end

    def width=(val)
      self.w = val
    end

    def height
      h
    end

    def height=(val)
      self.h = val
    end

    # collision

    def collides?(other)
      return false if other.nil?

      x < other.x + other.w &&
        x + w > other.x &&
        y < other.y + other.h &&
        y + h > other.y
    end

    # helpers

    def to_s
      "Rect(x: #{x}, y: #{y}, w: #{w}, h: #{h})"
    end

    def to_a
      [x, y, w, h]
    end
  end
end
