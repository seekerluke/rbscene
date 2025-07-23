# clean all build artifacts, including the C extension stuff
rm *.gem
cd ext/rbscene
rm *.o
rm -rf *.bundle*
rm *.log
rm Makefile
