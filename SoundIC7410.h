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
	SoundIC7410(char *s);
	int get_channels   () const override { return channels; }
	int get_nfft       () const override { return     nfft; }
	int get_spectrum_x () const override { return      512; }
	int get_spectrum_y () const override { return       50; }
	int get_waterfall_x() const override { return      512; }
	int get_waterfall_y() const override { return      150; }
	int get_index(int i, int, int) const override { return i; }
	int asound_fftcopy() override;
	virtual ~SoundIC7410();
};

#endif /* SOUNDIC7410_H_ */
