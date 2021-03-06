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


#ifndef INCLUDED_MOVEII_BURST_SYNC_CC_H
#define INCLUDED_MOVEII_BURST_SYNC_CC_H

#include <moveii/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace moveii {

    /*!
     * \brief <+description of block+>
     * \ingroup moveii
     *
     */
    class MOVEII_API burst_sync_cc : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<burst_sync_cc> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of moveii::burst_sync_cc.
       *
       * To avoid accidental use of raw pointers, moveii::burst_sync_cc's
       * constructor is in a private implementation
       * class. moveii::burst_sync_cc::make is the public interface for
       * creating new instances.
       */
      static sptr make(bool MPSK, float framelen, std::string syncword, int synclen, int samples_per_symbol, float sample_rate, float freq_deviaton_max, float alpha, float gain, int ntaps, int N_scale);
    };

  } // namespace moveii
} // namespace gr

#endif /* INCLUDED_MOVEII_BURST_SYNC_CC_H */
