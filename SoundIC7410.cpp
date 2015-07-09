/*
 * SoundIC7410.cpp
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#include "SoundIC7410.h"
#include <iostream>
using namespace std;

SoundIC7410::SoundIC7410() {
}

SoundIC7410::SoundIC7410(char* s) {
	sound_device = s;
	channels = 1;
	rate = 32000;
	buffer_size = 32 * 1024;
	period_size =  8 * 1024;
	nfft = 4 * 1024; /* IC-7410 */

	cout << "SoundIC7410::SoundIC7410() begin.. \n"
				<< "  sound_device = " << sound_device
				<< ", channels = " << channels
				<< ", rate = " << rate
				<< ", buffer_size = " << buffer_size
				<< ", period_size = " << period_size << endl;

	samples            = new signed short[period_size * channels * 2];
	audio_signal       = new double      [period_size * channels * 2];
	audio_signal_ffted = new double [nfft];
	in_real = new double[nfft];
	out  = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * ( nfft/2 + 1 )*2 );
	plan = fftw_plan_dft_r2c_1d(nfft, in_real, out, FFTW_MEASURE);


	asound_init();
	asound_go();
}

int SoundIC7410::asound_fftcopy() {
	for (int i = 0; i < nfft; i++) {
		in_real[i] = audio_signal[i];
		if(i < 5 || i> nfft-6) cout << "i = " << i << ", nfft = " << nfft << ", in_real = " << in_real[i] << endl;
	}
	return 0;
}

SoundIC7410::~SoundIC7410() {
	cout << "SoundIC7410::~SoundIC7410() begin.. \n";
}

