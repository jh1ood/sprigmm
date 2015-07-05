/*
 * SoundIC7410.cpp
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#include "SoundIC7410.h"
#include <iostream>
using namespace std;

SoundIC7410::SoundIC7410() {
}

SoundIC7410::SoundIC7410(char* s) {
	sound_device = s;
	channels = 1;
	rate = 32000;
	buffer_size = 32 * 1024;
	period_size =  8 * 1024;
	cout << "SoundIC7410::SoundIC7410() begin.. \n"
				<< "  sound_device = " << sound_device
				<< ", channels = " << channels
				<< ", rate = " << rate
				<< ", buffer_size = " << buffer_size
				<< ", period_size = " << period_size << endl;

	asound_init();
	asound_go();
}

SoundIC7410::~SoundIC7410() {
	// TODO Auto-generated destructor stub
}

