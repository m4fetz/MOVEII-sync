#include "rrc_filter_fft.h"
#include <cmath>
#include <cstdio>
#include <complex.h>
#include <cstring> // provides memcpy

#define PI 3.14159265f

float my_abs(const float a){
  return (a>=0.0f) ? a : -a;
}

rrc_filter_fft::rrc_filter_fft(const float ROLLOFF, const num_t NUM_TAPS, const num_t OSF1, /* const num_t OSF2 ,*/ const num_t N_SCALE)
    : OSF1(OSF1),                       //oversampling factor of the input before FFT
      //OSF2(OSF2),                       //desired oversampling factor of output after FFT
      //OSF_TOTAL(OSF1*OSF2),             //actual oversampling factor at output
      ROLLOFF(ROLLOFF),                 //d_alpha
      FILTER_LEN(2*NUM_TAPS*OSF1+1),
      OVERLAP(FILTER_LEN-1),
      N_forward(N_SCALE*OVERLAP),       //total length of filter including the overlap
      N_forward_half(N_SCALE*OVERLAP/2)
      //N_backwards_half(N_forward*OSF2/2),
      //BLOCK_LEN_IN_SYM((N_SCALE-1)*2*NUM_TAPS),
      //BLOCK_LEN_OUT(BLOCK_LEN_IN_SYM*OSF_TOTAL)
{
      //two buffers for different data types
      this->buffer = fftwf_alloc_real(N_forward);
      this->filter_fft = fftwf_alloc_complex(N_forward_half);

      this->buffer_complex = fftwf_alloc_complex(N_forward);
      this->buffer_processed = fftwf_alloc_complex(N_forward);

      //create plan
      this->p_forward  = fftwf_plan_dft_r2c_1d(N_forward, this->buffer, this->filter_fft,  FFTW_CREATE_PATIENCE);

      //Compute frequency response of the RRC filter
      const float gain = 1.0/(sqrt((double) this->N_forward*OSF1)*4.0*sqrt(2));

      for(num_t i = 0; i < 2*NUM_TAPS; i++) {
            for(num_t k = 0; k < this->OSF1; k++) {
                this->buffer[i*OSF1+k] = gain*this->filter_tap(i, k, NUM_TAPS);
            }
      }
      this->buffer[2*NUM_TAPS*OSF1] = gain*this->filter_tap(2*NUM_TAPS,0, NUM_TAPS);

      for(num_t i=2*NUM_TAPS*OSF1+1; i<N_forward; i++) {  //zero padding for the overlap-save method Overlap = N_forward -FILTER_LEN
          this->buffer[i] = 0.0f;
      }

      fftwf_execute(this->p_forward);

      /*
      done in burst_sync_cc_impl
      // Init overlap part of input buffer with zeros
      for(num_t i=0; i<N_forward; i++) {
          this->buffer_in[i] = 0.0f;
      }
      */
}

rrc_filter_fft::~rrc_filter_fft() {
  fftwf_destroy_plan(p_forward);

  fftwf_free(this->buffer);
  fftwf_free(this->filter_fft);
  fftwf_free(this->buffer_complex);

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
    //every other t
    const float t = (float)i - (float)N + (((float)k) / OSF1);

    const float nom = sin(PI*t*(1.0f-ROLLOFF)) + 4.0f*ROLLOFF*t*cos(PI*t*(1.0f+ROLLOFF));
    const float denom = PI*t*(1.0f - ( (4*ROLLOFF*t)*(4*ROLLOFF*t) ));

    return (nom/denom);
}

void rrc_filter_fft::filter(fftwf_complex *input){
    num_t i;
    float tmp_real, tmp_imag;

    /*
    for(i=0; i<BLOCK_LEN_IN_SYM; i++) {
        this->buffer_in[OVERLAP+OSF1*i] = input[i];
    }
    */
    for (i=0; i < N_forward_half; i++) {

      //fftwf_complex is defined as float[2] array
      tmp_real =  input[i][0] * this->filter_fft[i][0]
                - input[i][1] * this->filter_fft[i][1];
      tmp_imag =  input[i][0] * this->filter_fft[i][1]
                + input[i][1] * this->filter_fft[i][0];

      this->buffer_complex[i][0] = tmp_real;
      this->buffer_complex[i][1] = tmp_imag;
    }

    //std::memcpy(&input, this->filter_fft, N_forward_half*sizeof(fftwf_complex));
}
const fftwf_complex* rrc_filter_fft::get_output_buffer() const{
  return this->buffer_processed;
}
