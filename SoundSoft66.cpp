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
	buffer_size = 32 * 1024;
	period_size =  2 * 1024;
	nfft = 2 * 1024; /* Soft66LC4 */

	cout << "SoundSoft66::SoundSoft66() begin.. \n"
				<< "  sound_device = " << sound_device
				<< ", channels = " << channels
				<< ", rate = " << rate
				<< ", buffer_size = " << buffer_size
				<< ", period_size = " << period_size << endl;

	samples            = new signed short[period_size * channels * 2];
	audio_signal       = new double      [period_size * channels * 2];
//	audio_signal_ffted = new double [nfft];
	in      = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft);
	out  = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft);
	plan = fftw_plan_dft_1d    (nfft, in     , out, FFTW_FORWARD, FFTW_MEASURE);

	asound_init();
	asound_go();
}

int SoundSoft66::asound_fftcopy() {
	for (int i = 0; i < nfft; i++) { /* I and Q reversed */
		in[i][1] = audio_signal[2 * i];
		in[i][0] = audio_signal[2 * i + 1];
	}
	return 0;
}

SoundSoft66::~SoundSoft66() {
	cout << "SoundSoft66::~SoundSoft66() begin.. \n";
}

