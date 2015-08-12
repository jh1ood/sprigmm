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
	start = chrono::system_clock::now();
	sound_device = s;
	channels = 1;
	rate = 32000;
	buffer_size =  32   * 1024;
	period_size =   8.123   * 1024;
	nfft        =   4   * 1024;
	bin_size = (double) rate / (double) nfft;
	waveform_x  = 1801; waveform_y  =   90;
	spectrum_x  =  600;	spectrum_y  =   90;
	waterfall_x =  600;	waterfall_y =   90;
	density_x   =  600;	density_y   =  120;
	timer_value =  ( 1000.0 / ( (double)rate/(double)period_size) ) / 2.0;
	cout << "SoundIC7410::SoundIC7410(): timer_value = " << timer_value << endl;
	amax = 15.0;
	amin =  8.0;

	cout << "SoundIC7410::SoundIC7410() begin.. \n"

				<< "  sound_device = " << sound_device
				<< ", channels = " << channels
				<< ", rate = " << rate
				<< ", bin_size = " << bin_size
				<< ", buffer_size = " << buffer_size
				<< ", period_size = " << period_size << endl;

	samples      = new signed short[period_size * channels];

	audio_signal = new double [2*buffer_size];
	signal_start = audio_signal;
	signal_end   = audio_signal;

	audio_window = new double[nfft];
	for(int i=0;i<nfft;i++) {
		audio_window[i] = 0.5* ( 1.0 - cos(2.0*3.14159265*(double)i / (double) (nfft - 1))); /* Hanning */
	}

	in_real      = new double[nfft];
//	in_real      = new double[2*buffer_size];
	out          = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * ( nfft/2 + 1 )*2 );
	plan         = fftw_plan_dft_r2c_1d(nfft, in_real, out, FFTW_MEASURE);

	asound_init();
}

int SoundIC7410::asound_fftcopy() {
	auto end = chrono::system_clock::now();
	auto diff = end - start;
	cout << "SoundIC7410::asound_fftcopy(): elapsed time = " << chrono::duration_cast<std::chrono::milliseconds>(diff).count() << " msec." << endl;

	/* copy audio_signal into fft input buffer */

	cout << "entered asound_fftcopy(): audio_signal = " << audio_signal << ", signal_start = " << signal_start << ", signal_end = " << signal_end << ", diff = "
			<< signal_end - signal_start << endl;

	if(signal_end - signal_start >= nfft) { /* this should always be true */
		cout << "signal_end - signal_start = " << signal_end - signal_start << endl;
		auto p = signal_start;
		for (int i = 0; i < nfft; i++) {
			in_real[i] = *p++ * audio_window[i];
		}
	}

	/* forward nfft/2 samples */
	signal_start += (int) (nfft/2.345);
	cout << "signal_start offset = " << signal_start - audio_signal << endl;

//	cout << "AAA " << signal_end - signal_start << " " << signal_start - audio_signal << " " << signal_end - audio_signal << endl;
	return signal_end - signal_start;
}

SoundIC7410::~SoundIC7410() {
	cout << "SoundIC7410::~SoundIC7410() destructor.. \n";
}

