/*
 * SoundIC7410.cpp
 */

#include "SoundIC7410.h"
#include <iostream>
using namespace std;

SoundIC7410::SoundIC7410(char* s) {
	sound_device      =            s;
	channels          =            1;
	rate              =  32   * 1000; /* it is 1000, not 1024 */
	buffer_size       = 128   * 1024;
	period_size       =   4   * 1024;
	nfft              =   8   * 1024;
	fft_forward_ratio = 0.5; /* (0.0, 1.0], 0.5 is half overlap, 1.0 is no overlap */
	timer_margin      = 1.0;

	bin_size    = (double) rate / (double) nfft;
	timer_value =  ( 1000.0 / ( (double)rate/(double)period_size) ) / timer_margin;
	timer_value2 = timer_value / 1.1;

	waveform_x  = 1801; waveform_y  =   40;
	spectrum_x  = 1801;	spectrum_y  =   40;
	waterfall_x = 1801;	waterfall_y =  100;

	amax = 14.0; /* waterfall pseudo color */
	amin =  8.0; /* waterfall pseudo color */

	cout << "SoundIC7410::SoundIC7410()"
				<< "  sound_device = " << sound_device
				<< ", channels = "     << channels
				<< ", rate = "         << rate
				<< ", bin_size = "     << bin_size
				<< ", buffer_size = "  << buffer_size
				<< ", period_size = "  << period_size
				<< ", timer_value = "  << timer_value
				<< ", audio_signal_buffer_size = " << audio_signal_buffer_size
				<< endl;

	samples      = new signed short[period_size * channels];
	audio_signal = new double      [audio_signal_buffer_size]; /* just the buffer_size is not enough */
	signal_start = audio_signal;
	signal_end   = audio_signal;

	audio_window = new double[nfft];
	for(int i=0;i<nfft;i++) {
		audio_window[i] = 0.5* ( 1.0 - cos(2.0*3.14159265*(double)i / (double) (nfft - 1))); /* Hanning */
	}

	in   = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft); /* imaginary part is always zero */
	out  = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft);
	plan = fftw_plan_dft_1d(nfft, in, out, FFTW_FORWARD, FFTW_MEASURE);

	asound_init();
}

SoundIC7410::~SoundIC7410() {
	cout << "SoundIC7410::~SoundIC7410() destructor.. \n";
}

