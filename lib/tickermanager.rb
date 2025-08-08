# frozen_string_literal: true

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

    def stop(name, reset: false)
      @tickers[name][:running] = false

      return unless reset

      @tickers[name][:counter] = 0
      @tickers[name][:value] = 0
    end

    def running?(name)
      @tickers[name][:running]
    end

    def update(name)
      t = @tickers[name]
      return unless t[:running]

      t[:counter] += 1
      return unless t[:counter] > t[:rate]

      t[:value] += 1
      t[:value] = 0 if t[:value] >= t[:limit]
      t[:counter] = 0
    end
  end
end
