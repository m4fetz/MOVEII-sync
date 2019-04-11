#!/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/home/max/gnuradio/gr-moveii/lib
export PATH=/home/max/gnuradio/gr-moveii/build/lib:$PATH
export LD_LIBRARY_PATH=/home/max/gnuradio/gr-moveii/build/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$PYTHONPATH
test-moveii 
