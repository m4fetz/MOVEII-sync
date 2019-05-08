/* -*- c++ -*- */
/*
 * Copyright 2019 gr-moveii author.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_MOVEII_BURST_SYNC_CC_IMPL_H
#define INCLUDED_MOVEII_BURST_SYNC_CC_IMPL_H

#include <moveii/burst_sync_cc.h>
#include <volk/volk.h>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include "hexstring_to_binary.h"

namespace gr {
  namespace moveii {

    class burst_sync_cc_impl : public burst_sync_cc
    {
     private:
      // Nothing to declare in this block.
      const bool d_MPSK;
      const unsigned int d_framelen_bits;
      const int d_sps;
      const float d_Fmax;
      gr_complex *d_syncword;
      gr_complex *d_tmp_fv;   //input samples
      gr_complex *d_tmp_fc;   //fft samples

     public:
      burst_sync_cc_impl(bool MPSK, float framelen, std::string syncword, int synclen, int samples_per_symbol, float sample_rate, float freq_deviaton_max);
      ~burst_sync_cc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int work(int noutput_items,
         gr_vector_int &ninput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace moveii
} // namespace gr

#endif /* INCLUDED_MOVEII_BURST_SYNC_CC_IMPL_H */
