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
	buffer_size = 32 * 1024;
	period_size =  8 * 1024;
	nfft        =  8 * 1024;
	bin_size = (double) rate / (double) nfft;
	waveform_x   = 510; waveform_y  =   90;
	spectrum_x  =  510;	spectrum_y  =   90;
	waterfall_x =  510;	waterfall_y =   90;
	density_x   =  480;	density_y   =  120;
	timervalue =  ( 1000.0 / ( (double)rate/(double)period_size) ) / 1.1;
	timervalue = 90;
	amax = 16.0;
	amin =  7.0;

	cout << "SoundIC7410::SoundIC7410() begin.. \n"
				<< "  sound_device = " << sound_device
				<< ", channels = " << channels
				<< ", rate = " << rate
				<< ", bin_size = " << bin_size
				<< ", buffer_size = " << buffer_size
				<< ", period_size = " << period_size << endl;

	samples      = new signed short[period_size * channels];
	audio_signal = new double      [period_size * channels];
	in_real      = new double[nfft];
	out          = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * ( nfft/2 + 1 )*2 );
	plan         = fftw_plan_dft_r2c_1d(nfft, in_real, out, FFTW_MEASURE);

	asound_init();
}

int SoundIC7410::asound_fftcopy() {
	for (int i = 0; i < nfft; i++) {
		in_real[i] = audio_signal[i];
	}
	return 0;
}

//int SoundIC7410::get_index(const int i, const int, const int) const {
//		return i;
//}

SoundIC7410::~SoundIC7410() {
	cout << "SoundIC7410::~SoundIC7410() destructor.. \n";
}

