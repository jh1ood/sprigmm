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
	buffer_size       =  32   * 1024;
	period_size       =   8   * 1024;
	nfft              =  16   * 1024;
	fft_forward_ratio = 0.25;
	timer_margin      = 2.0;

	bin_size    = (double) rate / (double) nfft;
	timer_value =  ( 1000.0 / ( (double)rate/(double)period_size) ) / timer_margin;

	waveform_x  = 1801; waveform_y  =   90;
	spectrum_x  = 1801;	spectrum_y  =   90;
	waterfall_x = 1801;	waterfall_y =   90;
	density_x   = 1801;	density_y   =  120;

	amax = 15.0; /* waterfall pseudo color */
	amin =  8.0; /* waterfall pseudo color */

	cout << "SoundIC7410::SoundIC7410()"
				<< "  sound_device = " << sound_device
				<< ", channels = "     << channels
				<< ", rate = "         << rate
				<< ", bin_size = "     << bin_size
				<< ", buffer_size = "  << buffer_size
				<< ", period_size = "  << period_size
				<< ", timer_value = "  << timer_value
				<< endl;

	samples      = new signed short[period_size * channels];
	audio_signal = new double      [buffer_size];
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

int SoundIC7410::asound_fftcopy() {
	/* copy into FFT input buffer */
	if(signal_end - signal_start >= nfft*channels) { /* this should always be true */
		auto p = signal_start;
		for (int i = 0; i < nfft; i++) {
			in[i][0] = *p++ * audio_window[i];
			in[i][1] = 0.0; /* no imaginary part */
		}
		return 0;
	} else { /* should never happen */
		cout << "SoundIC7410::asound_fftcopy((): error " << endl;
		return 1;
	}
}

SoundIC7410::~SoundIC7410() {
	cout << "SoundIC7410::~SoundIC7410() destructor.. \n";
}

