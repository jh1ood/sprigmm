/*
 * Usbsound.h
 *
 *  Created on: Jun 4, 2015
 *      Author: user1
 */

#ifndef USBSOUND_H_
#define USBSOUND_H_

#include <asoundlib.h>
#include <string>
#include <iostream>
using namespace std;

class Usbsound {
public:
	Usbsound();
	Usbsound(char* s);
	virtual ~Usbsound();
	void Usbsound_go();
	bool asound_myread();
//	virtual void output() { cout << "usbsound output" << endl; };
	int asound_set_hwparams(snd_pcm_t * handle, snd_pcm_hw_params_t * params);
	int asound_set_swparams(snd_pcm_t * handle, snd_pcm_sw_params_t * swparams);
public:
	unsigned int rate;
	signed short *samples;
	double       *audio_signal;

protected:
	int err;
	char* sound_device;
	unsigned int channels;
	int nsamples;

	snd_pcm_uframes_t   buffer_size;
	snd_pcm_uframes_t   period_size;
	snd_pcm_sframes_t   avail;
	snd_pcm_sframes_t   frames_actually_read;
	snd_pcm_t           *handle;
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_sw_params_t *swparams;
	snd_async_handler_t *ahandler;

	const int byte_per_sample = 2;	/* 16 bit format */
	const int resample        = 0;	/* disable resample */
	const int period_event    = 0;	/* produce poll event after each period */
private:
	string myid = "Usbsound::Usbsound(): ";
};

#endif /* USBSOUND_H_ */
