/*
 * Sound.h
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#ifndef SOUND_H_
#define SOUND_H_

#include "AlsaParams.h"
#include <asoundlib.h>


class Sound : public AlsaParams {
public:
	Sound();
	virtual int get_channels() const = 0;
	int asound_init();
	int asound_go();
	int asound_read();
	int asound_set_hwparams   (snd_pcm_t * handle, snd_pcm_hw_params_t * hwparams);
	int asound_set_swparams   (snd_pcm_t * handle, snd_pcm_sw_params_t * swparams);
	virtual ~Sound();
private:
	int count = 0;
};

#endif /* SOUND_H_ */
