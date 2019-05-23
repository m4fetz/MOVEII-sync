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
#include <gnuradio/filter/firdes.h>
using namespace std;

namespace gr {
  namespace moveii {

    burst_sync_cc::sptr
    burst_sync_cc::make(bool MPSK, float framelen, std::string syncword, int synclen, int samples_per_symbol, float sample_rate, float freq_deviaton_max, float alpha, float gain, int ntaps)
    {
      return gnuradio::get_initial_sptr
        (new burst_sync_cc_impl(MPSK, framelen, syncword, synclen, samples_per_symbol, sample_rate, freq_deviaton_max, alpha, gain, ntaps ));
    }

    /*
     * The private constructor
     */
    burst_sync_cc_impl::burst_sync_cc_impl(bool MPSK, float framelen, std::string syncword, int synclen, int samples_per_symbol, float sample_rate, float freq_deviaton_max, float alpha, float gain, int ntaps)
      : gr::sync_block("burst_sync_cc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex))),

      d_framelen_bits(round(framelen*8)),
      d_MPSK(MPSK),  //true if BPSK
      d_synclen_bits(synclen),  //syncword length in bits
      d_sample_rate(sample_rate), //sample rate
      d_sps(samples_per_symbol), //oversampling rate
      d_symbol_rate(d_sample_rate/d_sps), //symbol rate
      d_Fmax(freq_deviaton_max), //max frequency offset at receiver
      d_steps(static_cast<int>(4.0 * (1.0/d_sample_rate) * d_Fmax)), //steps for the coarse frequency estimate
      d_F_L(d_sample_rate/4.0),  //coarse frequency range

      d_alpha(alpha), //alpha of rrc_filter
      d_gain(gain),   //gain of rrc_filter
      d_ntaps(ntaps)  //number of taps for rrc_filter

    {
      //copy ASM to private memory
      const unsigned int synclen_byte = std::ceil((d_synclen_bits)/8.0f);
      boost::scoped_array<unsigned char> tmp(new unsigned char[synclen_byte]);
      hexstring_to_binary(&syncword, tmp.get());

      //initialise the root raised cosine filter
      //static std::vector<float> rrc_filter;
      //static std::vector<float> gr::filter::firdes::root_raised_cosine(double d_gain, double d_sample_rate, double d_symbol_rate, double	d_alpha, int ntaps);



      //Buffers
      d_tmp_fv = (gr_complex*) volk_malloc(d_framelen_bits * sizeof(gr_complex), volk_get_alignment()); //aligned Buffer for complex samples
      d_tmp_fft_in = (gr_complex*) volk_malloc(d_framelen_bits * sizeof(gr_complex), volk_get_alignment()); //aligned input Buffer for fft samples
      d_tmp_fft_out = (gr_complex*) volk_malloc(d_framelen_bits * sizeof(gr_complex), volk_get_alignment()); //aligned output Buffer for fft samples
      d_tmp_ifft_in = (gr_complex*) volk_malloc(d_framelen_bits * sizeof(gr_complex), volk_get_alignment()); //aligned input Buffer for ifft samples
      d_tmp_ifft_out = (gr_complex*) volk_malloc(d_framelen_bits * sizeof(gr_complex), volk_get_alignment());//aligned output Buffer for ifft samples

      d_syncword = (gr_complex*) volk_malloc(d_synclen_bits * sizeof(gr_complex), volk_get_alignment()); // aligned buffer for syncword
      d_syncword_conj = (gr_complex*) volk_malloc(d_synclen_bits * sizeof(gr_complex), volk_get_alignment()); //aligned buffer for complex conjugated syncword


      //initialise the fftw fftwf_plan
      int blocksize;           //blocksize of the fft
      fftwf_plan plan_forward = fftwf_plan_dft_1d(blocksize, const_cast<fftwf_complex*>(reinterpret_cast<const fftwf_complex*>(d_tmp_fft_in)), reinterpret_cast<fftwf_complex*>(d_tmp_fft_out), FFTW_FORWARD, FFTW_ESTIMATE);
      fftwf_plan plan_backward = fftwf_plan_dft_1d(blocksize, const_cast<fftwf_complex*>(reinterpret_cast<const fftwf_complex*>(d_tmp_ifft_in)), reinterpret_cast<fftwf_complex*>(d_tmp_ifft_out), FFTW_BACKWARD, FFTW_ESTIMATE);

      //get syncword out of tmp
      for(unsigned int i=0; i<d_synclen_bits; i++)  {
        const uint8_t byte = tmp[i/8];

        const bool bit = ((byte >> (7-(i%8))) & 0x01) == 0x01;
        d_syncword[i] = (bit ? gr_complex(-1.0, 0.0) : gr_complex(1.0, 0.0));
      }

      //compute the complex conjugate of d_syncword
      volk_32fc_conjugate_32fc(d_syncword_conj, d_syncword, d_synclen_bits);

      //variables for the differential correlation
      float d_max_diff_corr = 0;      //buffer for the max result of the diff correlation
      int d_sample_set[2];            //array of the branch and sample time
    }

    /*
     * Our virtual destructor.
     */
    burst_sync_cc_impl::~burst_sync_cc_impl()
    {
      volk_free(d_syncword);
      volk_free(d_tmp_fv);
      volk_free(d_tmp_fft_out);
      volk_free(d_tmp_fft_in);
      volk_free(d_tmp_ifft_out);
      volk_free(d_tmp_ifft_in);
      //volk_free() of all buffers
    }


    void
    burst_sync_cc_impl::fft_input_samples(const gr_complex *in, gr_complex *out, fftwf_plan plan, bool forward) {

      switch (forward) {
        case true:
          //copy into buffer
          for (size_t i = 0; i < d_framelen_bits; i++) {
            d_tmp_fft_in[i] = in[i];
          }

          fftwf_execute(plan);

          for (size_t i = 0; i < d_framelen_bits; i++) {
            out[i] = d_tmp_fft_out[i];
          }
          break;

        case false:
          //copy into buffer
          for (size_t i = 0; i < d_framelen_bits; i++) {
            d_tmp_ifft_in[i] = in[i];
          }

          fftwf_execute(plan);

          for (size_t i = 0; i < d_framelen_bits; i++) {
            out[i] = d_tmp_ifft_out[i];
            out[i] /= d_framelen_bits;  //scaling
          }

          break;
        }
    }



    void burst_sync_cc_impl::fft_freq_shift_coarse() {

      //take fft_samples and add the phase increment
      for (size_t i = 0; i < d_framelen_bits; i++) {
        /*add factor of d_F_L to every sample */
      }

    }

    gr_complex burst_sync_cc_impl::gamma_func(const gr_complex *in, int n, int k){
      //gamma(n,k)=x(nT_s) * conj(p_r(kT))
      gr_complex result = in[n] * d_syncword_conj[k];
      return result;
    }

    gr_complex burst_sync_cc_impl::diff_corr(const gr_complex *in){
      /*compute the differential correlation*/
      gr_complex diff_corr = 0;

      for (size_t l = 0; l < d_synclen_bits; l++) {
        diff_corr += conj(gamma_func(in, l*d_sps, l)) * gamma_func(in, (l+1)*d_sps, (l+1));
      }
      return diff_corr;
    }

    void burst_sync_cc_impl::maximum_search(const gr_complex *in, int n, int N) {
      /* 'n' is the current sample set and 'N' the current branch, which will be stored in d_sample_set,
      then take abs of diff_corr result and compare it to previous one*/
      float current_corr = abs(diff_corr(in));

      if (d_max_diff_corr < current_corr) {

        d_max_diff_corr = current_corr;
        d_sample_set[0] = N;
        d_sample_set[1] = n;
      }
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
