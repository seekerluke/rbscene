require 'mkmf'

dir_config('raylib', '/opt/homebrew/include/', '/opt/homebrew/lib/')
abort('raylib library not found') unless have_library('raylib')
abort('raylib header not found') unless have_header('raylib.h')

create_makefile('rbscene/rbscene')
