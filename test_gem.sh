# build C extensions and install the gem
cd ext/rbscene
ruby extconf.rb
make
cd ../..
gem build rbscene.gemspec
gem install rbscene-0.0.1.gem
