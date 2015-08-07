/*
 * SoundIC7410.cpp
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#include "SoundIC7410.h"
#include <iostream>
using namespace std;

SoundIC7410::SoundIC7410(char* s) {
	sound_device = s;
	channels = 1;
	rate = 32000;
	buffer_size = 128 * 1024;
	period_size =   8 * 1024;
	nfft        =   8 * 1024;
	bin_size = (double) rate / (double) nfft;
	waveform_x  = 1801; waveform_y  =   90;
	spectrum_x  =  520;	spectrum_y  =   90;
	waterfall_x =  520;	waterfall_y =   90;
	density_x   =  520;	density_y   =  120;
	timervalue =  ( 1000.0 / ( (double)rate/(double)period_size) ) / 1.5;
	cout << "SoundIC7410::SoundIC7410(): timervalue = " << timervalue << endl;
	timervalue = 50; /* nominal 170 */
	amax = 16.0;
	amin =  5.0;

	cout << "SoundIC7410::SoundIC7410() begin.. \n"
				<< "  sound_device = " << sound_device
				<< ", channels = " << channels
				<< ", rate = " << rate
				<< ", bin_size = " << bin_size
				<< ", buffer_size = " << buffer_size
				<< ", period_size = " << period_size << endl;

	samples      = new signed short[period_size * channels];
	audio_signal = new double      [period_size * channels];
	audio_window = new double      [period_size];
	for(int i=0;i<(int) period_size;i++) {
		audio_window[i] = 0.5* ( 1.0 - cos(2.0*3.14159265*(double)i / (double) (period_size - 1))); /* Hanning */
		cout << "aaa" << audio_window[i] << endl;
	}
	in_real      = new double[nfft];
	out          = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * ( nfft/2 + 1 )*2 );
	plan         = fftw_plan_dft_r2c_1d(nfft, in_real, out, FFTW_MEASURE);

	asound_init();
}

int SoundIC7410::asound_fftcopy() {
	for (int i = 0; i < nfft; i++) {
		in_real[i] = audio_signal[i] * audio_window[i];
//		audio_signal[i] *= audio_window[i];
	}
	return 0;
}

SoundIC7410::~SoundIC7410() {
	cout << "SoundIC7410::~SoundIC7410() destructor.. \n";
}

