/*
 * SoundSof66.cpp
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#include "SoundSoft66.h"

SoundSoft66::SoundSoft66() {
}

SoundSoft66::SoundSoft66(char* s) {
	sound_device = s;
	channels = 2;
	rate = 48000;
	buffer_size = 32 * 1024;
	period_size =  2 * 1024;
	cout << "SoundSoft66::SoundSoft66() begin.. \n"
				<< "  sound_device = " << sound_device
				<< ", channels = " << channels
				<< ", rate = " << rate
				<< ", buffer_size = " << buffer_size
				<< ", period_size = " << period_size << endl;
	asound_init();
	asound_go();
}

SoundSoft66::~SoundSoft66() {
	// TODO Auto-generated destructor stub
}

