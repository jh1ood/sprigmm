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
	SoundSoft66(char* s, char* t);
	int  get_frequency  ()      override { return soft66_frequency; }
	int  get_other_frequency () override { return ic7410_frequency; }
	void set_frequnecy(int) override;
	int get_channels   () const override { return  channels; }
	int get_nfft       () const override { return  nfft; }
	int get_spectrum_x () const override { return  1920; }
	int get_spectrum_y () const override { return    50; }
	int get_waterfall_x() const override { return  1920; }
	int get_waterfall_y() const override { return   300; }
	int get_timervalue () const override { return   100; }
	int get_smeter     ()       override { return     0; }
	int get_index (int i, int j, int k) const override { return (2*j-(k/2)+i)%j; }
	int asound_fftcopy() override;
	virtual ~SoundSoft66();
};

#endif /* SOUNDSOFT66_H_ */
