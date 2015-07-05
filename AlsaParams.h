/*
 * AlsaParams.h
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#ifndef ALSAPARAMS_H_
#define ALSAPARAMS_H_

#include <asoundlib.h>

struct AlsaParams { // Advanced Linux Sound Architecture
	snd_pcm_uframes_t   buffer_size = 0;
	snd_pcm_uframes_t   period_size = 0;
	snd_pcm_sframes_t   avail = 0;
	snd_pcm_sframes_t   frames_actually_read = 0;
	snd_pcm_t           *handle = nullptr;
	snd_pcm_hw_params_t *hwparams = nullptr;
	snd_pcm_sw_params_t *swparams = nullptr;
	snd_async_handler_t *ahandler = nullptr;
	const int byte_per_sample = 2;	/* 16 bit format */
	const int resample        = 0;	/* disable resample */
	const int period_event    = 0;	/* produce poll event after each period */
	char* sound_device = nullptr;
	signed short *samples = nullptr;
	double       *audio_signal = nullptr;
	unsigned int channels;
	unsigned int rate;

	// ...
};

#endif /* ALSAPARAMS_H_ */
