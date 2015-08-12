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
	buffer_size  =  64 * 1024; /* 64, 8, 2 is a good combination?? */
	period_size  =   8 * 1024; /* 64, 8, 2 is a good combination?? */
	nfft         =   2 * 1024; /* 64, 8, 2 is a good combination?? */
	fft_forward_ratio = 0.5; /* half overlap */
	timer_margin      = 2.0;
	bin_size = (double) rate / (double) nfft;
	waveform_x  = 1801; waveform_y  =  40;
	spectrum_x  = 1801;	spectrum_y  =  90;
	waterfall_x = 1801;	waterfall_y =  90;
	density_x   = 1801;	density_y   =  90;
	timer_value =  ( 1000.0 / ( (double)rate/(double)period_size) ) / timer_margin;
	amax = 14.0; /* waterfall pseudo color */
	amin =  7.0; /* waterfall pseudo color */

	cout << "SoundSoft66::SoundSoft66()"
				<< "  sound_device = " << sound_device
				<< ", channels = " << channels
				<< ", rate = " << rate
				<< ", bin_size = " << bin_size
				<< ", buffer_size = " << buffer_size
				<< ", period_size = " << period_size
				<< ", timer_value = " << timer_value
				<< endl;

	samples      = new signed short[period_size * channels];
	audio_signal = new double [buffer_size];
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

int SoundSoft66::asound_fftcopy() {
	/* copy into FFT input buffer */
	if(signal_end - signal_start >= nfft*channels) { /* this should always be true */
		auto p = signal_start;
		for (int i = 0; i < nfft; i++) {
			in[i][0] = *p++ * audio_window[i]; /* invert I and Q signals */
			in[i][1] = *p++ * audio_window[i]; /* invert I and Q signals */
		}
		return 0;
	} else { /* should never happen */
		cout << "SoundSoft66::asound_fftcopy((): error " << endl;
		return 1;
	}
}

SoundSoft66::~SoundSoft66() {
	cout << "SoundSoft66::~SoundSoft66() destructor.. \n";
}

