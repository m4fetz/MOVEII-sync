#include "rrc_filter_fft.h"
#include <cmath>
#include <cstdio>
#include <complex.h>

#define PI 3.14159265f

float my_abs(const float a){
  return (a>=0.0f) ? a : -a;
}

rrc_filter_fft::rrc_filter_fft(const float ROLLOFF, const num_t NUM_TAPS, const num_t OSF1, const num_t OSF2, const num_t N_SCALE)
    : OSF1(OSF1),                       //oversampling factor of the input
      OSF2(OSF2),                       //desired oversampling factor of output
      OSF_TOTAL(OSF1*OSF2),             //actual oversampling factor at output
      ROLLOFF(ROLLOFF),                 //d_alpha
      FILTER_LEN(2*NUM_TAPS*OSF1+1),
      OVERLAP(FILTER_LEN-1),
      N_forward(N_SCALE*OVERLAP),       //total length of filter including the overlap
      N_forward_half(N_SCALE*OVERLAP/2),
      N_backwards_half(N_forward*OSF2/2),
      BLOCK_LEN_IN_SYM((N_SCALE-1)*2*NUM_TAPS),
      BLOCK_LEN_OUT(BLOCK_LEN_IN_SYM*OSF_TOTAL)
{
      this->buffer_real_in = fftwf_alloc_real(N_forward);
      this->buffer_real_out = fftwf_alloc_real(2*N_backwards_half);
      this->buffer_complex = fftwf_alloc_complex(N_backwards_half+1);
      this->filter = fftwf_alloc_complex(N_forward_half);


      this->p_forward  = fftwf_plan_dft_r2c_1d(N_forward, this->buffer_real_in, this->buffer_complex,  FFTW_CREATE_PATIENCE);
      // not needed here
      this->p_backward = fftwf_plan_dft_c2r_1d(2*N_backwards_half, this->buffer_complex, this->buffer_real_out, FFTW_CREATE_PATIENCE);

      //Compute frequency response of the RRC filter
      const float gain = 1.0/(sqrt((double) this->N_forward*OSF_TOTAL*OSF1)*4.0*sqrt(2));

      for(num_t i = 0; i < 2*NUM_TAPS; i++) {
            for(num_t k = 0; k < this->OSF1; k++) {
                this->buffer_real_in[i*OSF1+k] = gain*this->filter_tap(i, k, NUM_TAPS);
            }
      }
      this->buffer_real_in[2*NUM_TAPS*OSF1] = gain*this->filter_tap(2*NUM_TAPS,0, NUM_TAPS);
      
      for(num_t i=2*NUM_TAPS*OSF1+1; i<N_forward; i++) {
          this->buffer_real_in[i] = 0.0f;
      }


}

float rrc_filter_fft::filter_tap(const num_t i, const num_t k, const num_t N) {
    const float k_f = (float)k;
    const float i_f = (float)i;
    if(i==N && k==0) {
        // t=0
        return (1.0f-ROLLOFF+4.0f*ROLLOFF/PI);
    } else if(my_abs(my_abs( i_f - (float)N + k_f/OSF1 ) - (1.0f / (4.0f*ROLLOFF))) < 1e-4) {
        // t = +/- T/4beta
        return (ROLLOFF/sqrt(2.0f)*( (1.0f+2.0f/PI)*sin(PI/(4.0f*ROLLOFF)) + (1.0f-2.0f/PI)*cos(PI/(4.0f*ROLLOFF))));
    }
    const float t = (float)i - (float)N + (((float)k) / OSF1);

    /*
    if(my_abs(t) <= 1e-5) {
        return (1.0f-ROLLOFF+4.0f*ROLLOFF/PI);
    }

    if(my_abs( my_abs(4.0f*ROLLOFF*t) - 1.0f) <= 1e-5) {
        return ROLLOFF/sqrt(2.0f)*( (1.0f+2.0f/PI)*sin(PI/(4.0f*ROLLOFF)) + (1.0f-2.0f/PI)*cos(PI/(4.0f*ROLLOFF)));
    }
    */

    const float nom = sin(PI*t*(1.0f-ROLLOFF)) + 4.0f*ROLLOFF*t*cos(PI*t*(1.0f+ROLLOFF));
    const float denom = PI*t*(1.0f - ( (4*ROLLOFF*t)*(4*ROLLOFF*t) ));

    return (nom/denom);
}
