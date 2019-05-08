#!/usr/bin/sh
export VOLK_GENERIC=1
export GR_DONT_LOAD_PREFS=1
export srcdir=/home/max/MOVEII-sync/MOVE-II/gr-moveii/python
export PATH=/home/max/MOVEII-sync/MOVE-II/gr-moveii/build/python:$PATH
export LD_LIBRARY_PATH=/home/max/MOVEII-sync/MOVE-II/gr-moveii/build/lib:$LD_LIBRARY_PATH
export PYTHONPATH=/home/max/MOVEII-sync/MOVE-II/gr-moveii/build/swig:$PYTHONPATH
/usr/bin/python2 /home/max/MOVEII-sync/MOVE-II/gr-moveii/python/qa_freq_adjust_cf.py 
