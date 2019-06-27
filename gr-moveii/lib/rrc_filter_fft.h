#ifndef RRC_FILTER_FFT_H
#define RRC_FILTER_FFT_H

#define FFTW_CREATE_PATIENCE FFTW_MEASURE

#include <fftw3.h>

typedef unsigned int num_t;

class rrc_filter_fft {

private:

  fftwf_plan p_forward;

  float *buffer;
  fftwf_complex *filter_fft;
  fftwf_complex *buffer_complex;


  const num_t FILTER_LEN;
  const float ROLLOFF;
  const num_t OSF1;

  float filter_tap(num_t i, num_t k, const num_t);

public:

  rrc_filter_fft(const float ROLLOFF, const num_t NUM_TAPS, const num_t OSF1,/* const num_t OSF2,*/ const num_t N_SCALE);

  ~rrc_filter_fft();

  void filter(gr_complex *input);


};

#endif
