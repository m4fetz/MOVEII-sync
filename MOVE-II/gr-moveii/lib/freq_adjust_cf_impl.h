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

#ifndef INCLUDED_MOVEII_FREQ_ADJUST_CF_IMPL_H
#define INCLUDED_MOVEII_FREQ_ADJUST_CF_IMPL_H

#include <moveii/freq_adjust_cf.h>
#include <volk/volk.h>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include "hexstring_to_binary.h"

namespace gr {
  namespace moveii {

    class freq_adjust_cf_impl : public freq_adjust_cf
    {
     private:
       const bool d_MPSK; // parameter
       const unsigned int d_framelen_bits;
       gr_complex *d_syncword;
       const int d_sps;
       // d_blabla --> creates a option for the block
     public:
      freq_adjust_cf_impl();
      ~freq_adjust_cf_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace moveii
} // namespace gr

#endif /* INCLUDED_MOVEII_FREQ_ADJUST_CF_IMPL_H */
