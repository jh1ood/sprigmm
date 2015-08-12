/*
 * SoundSof66.cpp
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#include "SoundSoft66.h"

SoundSoft66::SoundSoft66(char* s) {
	sound_device = s;
	channels = 2;
	rate = 48000;
	buffer_size = 128 * 1024;
	period_size =   8 * 1024;
	nfft        =   2 * 1024;
	bin_size = (double) rate / (double) nfft;
	waveform_x  = 1801; waveform_y  =  40;
	spectrum_x  = 1801;	spectrum_y  =  90;
	waterfall_x = 1801;	waterfall_y =  90;
	density_x   = 1801;	density_y   =  90;
	timer_value =  ( 1000.0 / ( (double)rate/(double)period_size) ) / 1.5;
	cout << "SoundSoft66::SoundSoft66(): timer_value = " << timer_value << endl;
	amax = 14.0;
	amin =  7.0;

	cout << "SoundSoft66::SoundSoft66() begin.. \n"
				<< "  sound_device = " << sound_device
				<< ", channels = " << channels
				<< ", rate = " << rate
				<< ", bin_size = " << bin_size
				<< ", buffer_size = " << buffer_size
				<< ", period_size = " << period_size << endl;

	samples            = new signed short[period_size * channels];
//	audio_signal       = new double      [period_size * channels];
	audio_signal = new double [2*buffer_size];
	signal_start = audio_signal;
	signal_end   = audio_signal;
cout << "XXX " << signal_end - signal_start << " " << signal_start - audio_signal << " " << signal_end - audio_signal << endl;

	audio_window       = new double      [period_size];
	for(int i=0;i<(int) period_size;i++) {
		audio_window[i] = 0.5* ( 1.0 - cos(2.0*3.14159265*(double)i / (double) (period_size - 1))); /* Hanning */
	}
	in   = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft);
	out  = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft);
	plan = fftw_plan_dft_1d    (nfft, in     , out, FFTW_FORWARD, FFTW_MEASURE);

	asound_init();
}

int SoundSoft66::asound_fftcopy() {
//	for (int i = 0; i < nfft; i++) { /* I and Q reversed */
//		in[i][1] = audio_signal[2 * i    ] * audio_window[i];
//		in[i][0] = audio_signal[2 * i + 1] * audio_window[i];
//	}
//	return 0;

cout << "entered fftcopy \n";
	if(signal_end - signal_start >= nfft) { /* this should always be true */
		cout << "signal_end - signal_start = " << signal_end - signal_start << endl;
		auto p = signal_start;
		for (int i = 0; i < nfft; i++) {
//			in_real[i] = *p++ * audio_window[i];
			in[i][1] = *p++ * audio_window[i];
			in[i][0] = *p++ * audio_window[i];
		}
	}

	/* forward nfft/2 samples */
	signal_start += channels * (int) (nfft/2.345);
	cout << "signal_start offset = " << signal_start - audio_signal << endl;

//	cout << "AAA " << signal_end - signal_start << " " << signal_start - audio_signal << " " << signal_end - audio_signal << endl;
	return signal_end - signal_start;

}

SoundSoft66::~SoundSoft66() {
	cout << "SoundSoft66::~SoundSoft66() destructor.. \n";
}

