#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Copyright 2019 gr-moveii author.
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this software; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
#

from gnuradio import gr, gr_unittest
from gnuradio import blocks
import moveii_swig as moveii

def getBasePath():

    return os.path.realpath(os.path.dirname(__file__)+'/../')

class qa_burst_sync_cc (gr_unittest.TestCase):

    def setUp (self):
        self.tb = gr.top_block ()

    def tearDown (self):
        self.tb = None

    def runSyn_coarse_fsource(self, MPSK, framelen, syncword, synclen, samples_per_symbol, sample_rate, freq_deviaton_max, alpha, gain, ntaps):

        fsource = getBasePath()+'/testing/'

        #Blocks
        self.file_source = blocks.file_source(gr.sizeof_gr_complex*1, fsource, False)
        self.burst_sync = blocks.burst_sync_cc(MPSK, framelen, syncword, synclen, samples_per_symbol, sample_rate, freq_deviaton_max, alpha, gain, ntaps)
        dst = blocks.vector_sink_b();
        #Connections
        self.tb.connect((self.file_source, 0),(self.burst_sync, 0))
        self.tb.connect(self.burst_sync_cc, dst)
        #Create Flowgraph
        self.tb.run ()
        #check data
        result_data = dst.data()

    def test_001_t (self):
        # set up fg
        self.tb.run ()
        # check data


if __name__ == '__main__':
    gr_unittest.run(qa_burst_sync_cc, "qa_burst_sync_cc.xml")
