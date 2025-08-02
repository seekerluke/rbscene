# frozen_string_literal: true

require 'fileutils'

module RBScene
  class CLI
    def self.start(argv)
      command = argv.shift

      case command
      when 'dev'
        run_game
      when 'new'
        new_project
      else
        warn "Unknown command: #{command}"
      end
    end

    def self.run_game
      Dir.chdir(Dir.pwd) do
        require 'boot'
      end
    end

    def self.new_project
      abort "Error: Directory '#{Dir.pwd}' is not empty." unless Dir.empty?(Dir.pwd)

      template_path = File.expand_path('../templates/basic', __dir__)

      abort "Error: Template directory not found at #{template_path}" unless Dir.exist?(template_path)

      puts "Creating new project in #{Dir.pwd}..."
      FileUtils.cp_r("#{template_path}/.", Dir.pwd)

      puts 'Your project has been created. Have fun!'
    end
  end
end
