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
    burst_sync_cc_impl::burst_sync_cc_impl()
      : gr::sync_block("burst_sync_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),

      d_framelen_bits(round(framelen*8)),
      d_MPSK(MPSK),  //true if BPSK
      d_synclen_bits(synclen),  //syncword length in bits
      d_sample_rate(sample_rate), //sample rate
      d_sps(samples_per_symbol), //oversampling rate
      d_Fmax(freq_deviaton_max) //max frequency offset at receiver


    {
      //copy ASM to private memory
      const unsigned int synclen_byte = std::ceil((d_synclen_bits)/8.0f);
      boost::scoped_array<unsigned char> tmp(new unsigned char[synclen_byte]);
      hexstring_to_binary(&syncword, tmp.get());

      //Buffers
      d_tmp_fv = (gr_complex*) volk_malloc(d_framelen_bits * sizeof(gr_complex), volk_get_alignment()); //aligned Buffer for complex samples
      d_tmp_fc = (gr_complex*) volk_malloc(d_framelen_bits * sizeof(gr_complex), volk_get_alignment()); //aligned Buffer for fft samples
      d_syncword = (gr_complex*) volk_malloc(d_synclen_bits * sizeof(gr_complex), volk_get_alignment()); // aligned buffer for syncword

      //get syncword out of tmp
      for(unsigned int i=0; i<d_synclen_bits; i++)  {
        const uint8_t byte = tmp[i/8];

        const bool bit ((byte >> (7-(i%8))) & 0x01) == 0x01;
        d_syncword[i] = (bit ? gr_complex(-1.0, 0.0) : gr_complex(1.0, 0.0));
      }

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


    void
    burst_sync_cc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      ninput_items_required[0] = noutput_items;
    }

    void
    burst_sync_cc_impl::fft_input_samples(const gr_complex *in, gr_complex *out,const int N) {
      //compute the
      //needs the length of the input array --> N = ninput_items
      fftw_plan plan = fftw_plan_dft_1d(N, in, out, FFT_FORWARD, FFT_ESTIMATE);
      fftw_execute(plan);
      fftw_destroy_plan(plan);
      fftw_cleanup();
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
