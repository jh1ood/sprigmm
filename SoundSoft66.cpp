/*
 * SoundSof66.cpp
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#include "SoundSoft66.h"

SoundSoft66::SoundSoft66(char* s) {
	sound_device = s;
	channels     = 2;
	rate         =  48 * 1000; /* it is 1000, not 1024 */
	buffer_size  =  16 * 1024; /* this is the maximum value */
	period_size  =   4 * 1024;
	nfft         =   2 * 1024;
	fft_forward_ratio = 0.5; /* (0.0, 1.0], 0.5 is half overlap, 1.0 is no overlap */
	timer_margin      = 1.0;

	bin_size    = (double) rate / (double) nfft;
	timer_value =  ( 1000.0 / ( (double)rate/(double)period_size) ) / timer_margin;
	timer_value2 = timer_value / 0.3;

	waveform_x  = 1801; waveform_y  =   40;
	spectrum_x  = 1801;	spectrum_y  =   80;
	waterfall_x = 1801;	waterfall_y =  480;
	amax = 14.0; /* waterfall pseudo color */
	amin =  7.0; /* waterfall pseudo color */

	cout << "SoundSoft66::SoundSoft66()"
				<< "  sound_device = " << sound_device
				<< ", channels = "     << channels
				<< ", rate = "         << rate
				<< ", bin_size = " << bin_size
				<< ", buffer_size = " << buffer_size
				<< ", period_size = " << period_size
				<< ", timer_value = " << timer_value
				<< ", audio_signal_buffer_size = " << audio_signal_buffer_size
				<< endl;

	samples      = new signed short[period_size * channels];
	audio_signal = new double [audio_signal_buffer_size]; /* just the buffer_size is not enough */
	signal_start = audio_signal;
	signal_end   = audio_signal;

	audio_window = new double[nfft];
	for(int i=0;i<nfft;i++) {
		audio_window[i] = 0.5* ( 1.0 - cos(2.0*3.14159265*(double)i / (double) (nfft - 1))); /* Hanning */
	}

	in   = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft);
	out  = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft);
	plan = fftw_plan_dft_1d    (nfft, in     , out, FFTW_FORWARD, FFTW_MEASURE);

	asound_init();
}

SoundSoft66::~SoundSoft66() {
	cout << "SoundSoft66::~SoundSoft66() destructor.. \n";
}

