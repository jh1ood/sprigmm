/*
 * Usbsoundiq.cpp
 *
 *  Created on: Jun 4, 2015
 *      Author: user1
 */

#include "Usbsoundiq.h"

Usbsound_iq::Usbsound_iq(char* s) : Usbsound(s) {
	cout << myid << "constructor begin.." << endl;

	channels    = 2;
	rate        = 48000;
	buffer_size = 96 * 1024 * 10;
	period_size =  2 * 1024;

	cout << myid << "sound_device = " << sound_device
			<< ", rate = " << rate
			<< ", buffer_size = " << buffer_size
			<< ", period_size = " << period_size << endl;

	samples            = new signed short[period_size * channels * 2];
	audio_signal       = new double      [period_size * channels * 2];

	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);
	cout << myid << "hw and sw params alloc done" << endl;

	if ((err = snd_pcm_open(&handle, sound_device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		cout << myid << "snd_pcm_open() error. " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if ((err = asound_set_hwparams(handle, hwparams)) < 0) {
		cout << myid << "setting of hwparams failed. " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if ((err = asound_set_swparams(handle, swparams)) < 0) {
		cout << myid << "setting of swparams failed. " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if ((err = snd_pcm_prepare(handle)) < 0) {
		cout << myid << "snd_pcm_prepare error: " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if (snd_pcm_state(handle) == SND_PCM_STATE_PREPARED) {
		if ((err = snd_pcm_start(handle)) < 0) {
			cout << myid << "pcm_start error: " << snd_strerror(err) << endl;
			exit(EXIT_FAILURE);
		}
	}

	cout << myid << "constructor end.." << endl;
}

Usbsound_iq::~Usbsound_iq() {
	cout << "usbsound_iq destructor" << endl;
}

