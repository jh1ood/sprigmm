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
#include <termios.h>
#include <fcntl.h>
#include <vector>
using namespace std;

class Sound : public AlsaParams {
public:
	Sound();
	virtual ~Sound();

	virtual int  get_index(int, int, int) const = 0;
	virtual int  asound_fftcopy () = 0;

	int asound_init();
	int asound_read();
	int asound_set_hwparams();
	int asound_set_swparams();

};

#endif /* SOUND_H_ */
