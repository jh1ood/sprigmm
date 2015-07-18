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

SoundIC7410::SoundIC7410(char* s, char* t) {
	sound_device = s;
	tty_device   = t;
	channels = 1;
	rate = 32000;
	buffer_size = 32 * 1024;
	period_size =  8 * 1024;
	nfft = 4 * 1024;
	bin_size = (double) rate / (double) nfft;


	cout << "SoundIC7410::SoundIC7410() begin.. \n"
				<< "  sound_device = " << sound_device
				<< ", channels = " << channels
				<< ", rate = " << rate
				<< ", bin_size = " << bin_size
				<< ", buffer_size = " << buffer_size
				<< ", period_size = " << period_size << endl;

	samples      = new signed short[period_size * channels * 2];
	audio_signal = new double      [period_size * channels * 2];
	in_real      = new double[nfft];
	out          = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * ( nfft/2 + 1 )*2 );
	plan         = fftw_plan_dft_r2c_1d(nfft, in_real, out, FFTW_MEASURE);

	rig_init_serial(tty_device); /* currently for IC-7410 only */
	asound_init();
}

int SoundIC7410::get_frequency  () {

	/* freq response in char[8]-char[5] */

	static unsigned char command1[2] = { 0x03, 0xfd };
	char string[256];
	unsigned char buf[255];
	int res;

	send_command(command1);
	res = myread(buf);
	if (res != 11) {
		cout << "frequency response is wrong!" << endl;
	}
	sprintf(string, "%02x%02x%02x%02x", buf[8], buf[7], buf[6], buf[5]);
	ic7410_frequency = atoi(string);
	cout << "res = " << res << ", frequency = " << ic7410_frequency << endl;

	return ic7410_frequency;
}

//void set_ic7410_frequency(int i)  {
//	ic7410_frequency = i;
//}

int SoundIC7410::asound_fftcopy() {
	for (int i = 0; i < nfft; i++) {
		in_real[i] = audio_signal[i];
	}
	return 0;
}

void SoundIC7410::set_frequnecy(int i) {
	cout << "SoundIC7410::set_frequency " << i << endl;
}

SoundIC7410::~SoundIC7410() {
	cout << "SoundIC7410::~SoundIC7410() begin.. \n";
}

