# frozen_string_literal: true

module RBScene
  class CLI
    def self.start(argv)
      command = argv.shift

      case command
      when 'dev'
        run_game
      else
        warn "Unknown command: #{command}"
      end
    end

    def self.run_game
      puts Dir.pwd
      Dir.chdir(Dir.pwd) do
        require 'boot'
      end
    end
  end
end
