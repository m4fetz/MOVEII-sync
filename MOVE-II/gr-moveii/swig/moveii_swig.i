/* -*- c++ -*- */

#define MOVEII_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "moveii_swig_doc.i"

%{
#include "moveii/freq_adjust_cf.h"
%}


%include "moveii/freq_adjust_cf.h"
GR_SWIG_BLOCK_MAGIC2(moveii, freq_adjust_cf);
