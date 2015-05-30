/*
 * Sound.h
 *
 *  Created on: Apr 15, 2015
 *      Author: user1
 */

#ifndef SOUND_H_
#define SOUND_H_

#include <fftw3.h>
#include <asoundlib.h>
#include <string>
using namespace std;

class Sound {
public:
	Sound(char *, int);
	virtual ~ Sound();
	void asound_async_callback(snd_async_handler_t * ahandler); /* Note: pointer to a member function needs care. */
	bool asound_myread();
	void Sound_go();
private:
public:
	int  asound_set_hwparams   (snd_pcm_t * handle, snd_pcm_hw_params_t * hwparams);
	int  asound_set_swparams   (snd_pcm_t * handle, snd_pcm_sw_params_t * swparams);

	snd_pcm_uframes_t   buffer_size;
	snd_pcm_uframes_t   period_size;
	snd_pcm_sframes_t   avail;
	snd_pcm_sframes_t   frames_actually_read;
	snd_pcm_t           *handle;
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_sw_params_t *swparams;
	snd_async_handler_t *ahandler;

	char* sound_device;
	unsigned int rate;
	unsigned int channels;
	int nsamples;
	const int byte_per_sample = 2;	/* 16 bit format */
	const int resample        = 0;	/* disable resample */
	const int period_event    = 0;	/* produce poll event after each period */

//	signed short samples      [8192];
//	double       audio_signal [8192];
	signed short *samples;
	double       *audio_signal;

};

#endif				/* SOUND_H_ */
