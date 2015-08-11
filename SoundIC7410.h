/*
 * SoundIC7410.h
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#ifndef SOUNDIC7410_H_
#define SOUNDIC7410_H_

#include "AlsaParams.h"
#include "Sound.h"
#include <chrono>

class SoundIC7410: public Sound {
public:
	SoundIC7410(char *s);
	int get_index (int i, int j, int k) const override { return i; }
	int asound_fftcopy() override;
	virtual ~SoundIC7410();
private:

};

#endif /* SOUNDIC7410_H_ */
