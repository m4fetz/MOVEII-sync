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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "burst_sync_cc_impl.h"
#include <iostream>
#include <complex> // include complex before fftw3.h to make sure that std::complex<float> is compatible with fftwf_comples
#include <fftw3.h>
using namespace std;

namespace gr {
  namespace moveii {

    burst_sync_cc::sptr
    burst_sync_cc::make(bool MPSK, float framelen, std::string syncword, int synclen, int samples_per_symbol, float sample_rate, float freq_deviaton_max)
    {
      return gnuradio::get_initial_sptr
        (new burst_sync_cc_impl(MPSK, framelen, syncword, synclen, samples_per_symbol, sample_rate, freq_deviaton_max));
    }

    /*
     * The private constructor
     */
    burst_sync_cc_impl::burst_sync_cc_impl(bool MPSK, float framelen, std::string syncword, int synclen, int samples_per_symbol, float sample_rate, float freq_deviaton_max)
      : gr::sync_block("burst_sync_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),

      d_framelen_bits(round(framelen*8)),
      d_MPSK(MPSK),  //true if BPSK
      d_synclen_bits(synclen),  //syncword length in bits
      d_sample_rate(sample_rate), //sample rate
      d_sps(samples_per_symbol), //oversampling rate
      d_Fmax(freq_deviaton_max), //max frequency offset at receiver
      d_steps(static_cast<int>(4.0 * (1.0/d_sample_rate) * d_Fmax)), //steps for the coarse frequency estimate
      d_F_L(d_sample_rate/4.0)  //coarse frequency range

    {
      //copy ASM to private memory
      const unsigned int synclen_byte = std::ceil((d_synclen_bits)/8.0f);
      boost::scoped_array<unsigned char> tmp(new unsigned char[synclen_byte]);
      hexstring_to_binary(&syncword, tmp.get());



      //Buffers
      d_tmp_fv = (gr_complex*) volk_malloc(d_framelen_bits * sizeof(gr_complex), volk_get_alignment()); //aligned Buffer for complex samples
      d_tmp_fc = (gr_complex*) volk_malloc(d_framelen_bits * sizeof(gr_complex), volk_get_alignment()); //aligned Buffer for fft samples
      d_tmp_fs = (gr_complex*) volk_malloc(d_framelen_bits * sizeof(gr_complex), volk_get_alignment()); //aligned Buffer for freq shifted fft samples
      d_syncword = (gr_complex*) volk_malloc(d_synclen_bits * sizeof(gr_complex), volk_get_alignment()); // aligned buffer for syncword
      //d_max_diff_corr =     //buffer for the max result of the diff correlation
      //compute the frequencies in advance

      //get syncword out of tmp
      for(unsigned int i=0; i<d_synclen_bits; i++)  {
        const uint8_t byte = tmp[i/8];

        const bool bit = ((byte >> (7-(i%8))) & 0x01) == 0x01;
        d_syncword[i] = (bit ? gr_complex(-1.0, 0.0) : gr_complex(1.0, 0.0));
      }

      //fftw plan here
      //fftwf_plan plan = fftwf_plan_dft_1d(N,const_cast<fftwf_complex*>(reinterprets_cast<const fftwf_complex*>(in)), reinterpret_cast<fftwf_complex*>(out), FFTW_FORWARD, FFTW_ESTIMATE);
    }

    /*
     * Our virtual destructor.
     */
    burst_sync_cc_impl::~burst_sync_cc_impl()
    {
      volk_free(d_syncword);
      volk_free(d_tmp_fv);
      volk_free(d_tmp_fc);
      //volk_free() of all buffers
    }

    /*
      void burst_sync_cc_impl::init_freq_param() {
      //compute F_L and #ofsteps in advance
      int steps = static_cast<int>(4.0 * (1.0/d_sample_rate) * d_Fmax);
      float F_L = d_sample_rate/4.0;
      cout << "steps" << steps;
    }
    */

    void
    burst_sync_cc_impl::fft_input_samples(const gr_complex *in, gr_complex *out,const int N) {

      //needs the length of the required samples
      /* into  constructor: */
      fftwf_plan plan = fftwf_plan_dft_1d(N,const_cast<fftwf_complex*>(reinterpret_cast<const fftwf_complex*>(in)), reinterpret_cast<fftwf_complex*>(out), FFTW_FORWARD, FFTW_ESTIMATE);
      fftwf_execute(plan);
      fftwf_destroy_plan(plan);
      fftwf_cleanup();
    }



    void burst_sync_cc_impl::fft_freq_shift_coarse() {

      //take fft_samples and shift/rotate with incremental steps
      for (int N = 0; N < d_steps; N++) {
        /* code */
      }

    }

    gr_complex burst_sync_cc_impl::diff_corr(){
      /*compute the differential correlation*/
      return 0;
      
    }

    int burst_sync_cc_impl::maximum_search() {

      /* take abs of diff_corr result and compare it to previous one
      afterwards store the freq branch corresponding to the greater result*/
      return 0;
    }

    int
    burst_sync_cc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      // Do <+signal processing+>


      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace moveii */
} /* namespace gr */
