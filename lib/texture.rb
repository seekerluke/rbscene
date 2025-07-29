module RBScene
    class Texture
        # more methods defined in C

        def split(width:, height:)
            cols = (self.width / width).to_i
            rows = (self.height / height).to_i
            slices = []

            rows.times do |row|
                cols.times do |col|
                    x = col * width
                    y = row * height
                    slices << Rect.new(x, y, width, height)
                end
            end

            slices
        end
    end
end