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
#include <fftw3.h>
#include <cstdio>
#include <cstdlib>
#include "hexstring_to_binary.h"
#include <math.h>

namespace gr {
  namespace moveii {

    class burst_sync_cc_impl : public burst_sync_cc
    {
     private:
      // Nothing to declare in this block.
      const bool d_MPSK;
      const unsigned int d_framelen_bits;
      const unsigned int d_synclen_bits;
      const float d_sample_rate;
      const float d_symbol_rate;
      const int d_sps;
      const float d_Fmax;
      const int d_steps;
      const int d_F_step;
      const float d_F_L;
      const float d_alpha;    //alpha value for the rrc filter
      const float d_gain;     //gain value for the rrc filter
      const int d_ntaps;      //number of taps for the rrc filter


      gr_complex *d_syncword; //buffer for syncword
      gr_complex *d_syncword_conj; //buffer for complex conjugated syncword
      gr_complex *d_tmp_fv;   //buffer for input samples
      gr_complex *d_tmp_fft;     //buffer for fft samples
      gr_complex *d_tmp_ifft;  //buffer for fft samples that can be overwritten
      fftwf_complex* d_tmp_fft_work;

      int blocksize;
      fftwf_plan plan_forward;
      fftwf_plan plan_backward;

      //variables for differential correlation
      float d_max_diff_corr;
      int d_sample_set[2];

      void fft_input_samples(const gr_complex *in);
      void ifft();
      void fft_center();
      void fft_freq_shift_coarse(gr_complex *in, int N);
      void maximum_search(gr_complex *in, int n, int N);
      gr_complex gamma_func(gr_complex *in, int n, int k);
      gr_complex diff_corr(gr_complex *in);
      void coarse_offset(const gr_complex *in, gr_complex *out);


     public:
      burst_sync_cc_impl(bool MPSK, float framelen, std::string syncword, int synclen, int samples_per_symbol, float sample_rate, float freq_deviaton_max, float alpha, float gain, int ntaps);
      ~burst_sync_cc_impl();

      // Where all the action really happens
      //void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace moveii
} // namespace gr

#endif /* INCLUDED_MOVEII_BURST_SYNC_CC_IMPL_H */
