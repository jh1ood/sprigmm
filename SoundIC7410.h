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

class SoundIC7410: public Sound {
public:
	SoundIC7410();
	SoundIC7410(char *s);
	int get_channels() const override { return 1; }
	virtual ~SoundIC7410();
};

#endif /* SOUNDIC7410_H_ */
