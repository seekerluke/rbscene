module RBScene
    class TickerManager
        def initialize
            @tickers = {}
        end
        
        def define(name, rate: 60, limit: 3)
            @tickers[name] = {
                running: false,
                rate: rate,
                limit: limit,
                value: 0,
                counter: 0
            }
            define_singleton_method(name) { @tickers[name][:value] }
        end

        def undefine(name)
            @tickers.delete(name)
            undef_method(name)
        end

        def start(name)
            @tickers[name][:running] = true
        end

        def stop(name)
            @tickers[name][:running] = false
        end

        def running?(name)
            @tickers[name][:running]
        end

        def update(name)
            t = @tickers[name]
            return if !t[:running]

            t[:counter] += 1
            if t[:counter] > t[:rate]
                t[:value] += 1
                t[:value] = 0 if t[:value] >= t[:limit]
                t[:counter] = 0
            end
        end
    end
end