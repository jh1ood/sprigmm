/*
 * SoundSoft66.h
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#ifndef SOUNDSOFT66_H_
#define SOUNDSOFT66_H_

#include "AlsaParams.h"
#include "Sound.h"
#include <iostream>
using namespace std;

class SoundSoft66: public Sound {
public:
	SoundSoft66(char* s);
	int get_index (int i, int j, int k) const override { return (2*j-(k/2)+i)%j; }
	virtual ~SoundSoft66();
};

#endif /* SOUNDSOFT66_H_ */
