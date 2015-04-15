/*
 * Sound.h
 *
 *  Created on: Apr 15, 2015
 *      Author: user1
 */

#ifndef SOUND_H_
#define SOUND_H_
#include <asoundlib.h>
#include <string>
using namespace std;

class Sound {
public:
	Sound(const char*, const char*, const char*);
	virtual ~Sound();
private:
	snd_pcm_sframes_t buffer_size;
	snd_pcm_sframes_t period_size;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_sw_params_t *swparams;
    string sound_device;
    unsigned int rate;
    unsigned int channels;
};

#endif /* SOUND_H_ */
