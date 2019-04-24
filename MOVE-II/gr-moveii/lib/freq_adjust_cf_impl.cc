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
#include "freq_adjust_cf_impl.h"
#include <iostream>
using namespace std;

namespace gr {
  namespace moveii {

    freq_adjust_cf::sptr
    freq_adjust_cf::make(bool MPSK, float framelen, std::string syncword, int synclen, int samples_per_symbol, float sample_rate)
    {
      return gnuradio::get_initial_sptr
        (new freq_adjust_cf_impl(MPSK, framelen, syncword, synclen, samples_per_symbol, sample_rate));
    }

    /*
     * The private constructor
     */
    freq_adjust_cf_impl::freq_adjust_cf_impl()
      : gr::block("freq_adjust_cf",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(float)))

        d_framelen_bits(round(framelen*8)),
        d_MPSK(MPSK),  //true if BPSK
        d_synclen_bits(synclen),  //syncword length in bits
        d_sample_rate(sample_rate), //sample rate
        d_sps(samples_per_symbol), //oversampling rate
        d_FL(), // parameters for the frequency sweep
        d_Fmax(),
        d_Fstep()

    {

      //Copy ASM to private memory
      const unsigned int synclen_byte = std::ceil((d_synclen_bits)/8.0f);
      boost::scoped_array<unsigned char> tmp(new unsigned char[synclen_byte]);
      hexstring_to_binary(&syncword, tmp.get());

      d_tmp_fv = (gr_complex*) volk_malloc(d_framelen_bits * sizeof(gr_complex), volk_get_alignment()); //aligned buffer for complex samples TODO framelen is right?
      d_tmp_fc = (gr_complex*) volk_malloc(d_framelen_bits * sizeof(gr_complex), volk_get_alignment()); //aligned buffer for complex conjugated samples
      d_tmp_f = (gr_complex*) volk_malloc(sizeof(gr_complex), volk_get_alignment()); //aligned buffer for correlation result

      d_syncword = (gr_complex*) volk_malloc(d_synclen_bits * sizeof(gr_complex), volk_get_alignment());


      //get syncword out of tmp
      for(unsigned int i=0; i<d_synclen_bits; i++)  {
        const uint8_t byte = tmp[i/8];

        const bool bit ((byte >> (7-(i%8))) & 0x01) == 0x01;
        d_syncword[i] = (bit ? gr_complex(-1.0, 0.0) : gr_complex(1.0, 0.0));
      }

      //initialise variables
      //if not

    }

    /*
     * virtual destructor.
     */
    freq_adjust_cf_impl::~freq_adjust_cf_impl()
    {

      //volk_free() of all buffers TODO
    }

    void
    freq_adjust_cf_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
    }

    gr_complex freq_adjust_cf_impl::diff_corr(const gr_complex *in, const gr_complex *syncword, const int sps){

      //take the conjugate of the input samples
      volk_32fc_conjugate_32fc(d_tmp_fc, in, d_synclen_bits+1); //TODO needed samples = d_synclen_bits+1

      for (int l = 0; l<d_synclen_bits; l++){
        y_sum = 0;
        y_sum += (&d_tmp_fc[l*d_sps] * syncword[l]) * (&in[(l+1)*d_sps] * syncword[l+1]); //TODO update for qpsk
      }

      return y_sum;
    }

    gr_complex freq_adjust_cf_impl::freq_shift(const gr_complex *in, int i){

      const double pi = std::acos(-1);
      const std::complex<double> j(0, 1);
      shift_in = *in * std::exp(-j*pi*(-d_Fmax+i*d_Fstep))

    }


    gr_complex freq_adjust_cf_impl::coarse_freq_estimate(const gr_complex *in, const float alpha, const float FL, const int N) //not done
    {
      gr_complex coarse_branch_abs = (0.0,0.0); //into general_work?
      for (int i = 0; i <= N ; i++) {

        coarse_branch = /*matchedfilter of */freq_shift(in,i) * diff_corr();

      }
    }


    int
    freq_adjust_cf_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      float *out = (float *) output_items[0];

      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (noutput_items);

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace moveii */
} /* namespace gr */
