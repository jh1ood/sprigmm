/*
 * Sound.cpp
 *
 *  Created on: Apr 15, 2015
 *      Author: user1
 */

#include "Sound.h"
#include <iostream>
#include <string>
using namespace std;

Sound::Sound(const char* s, const char* r, const char* c)  {
	sound_device = s;
	rate         = atoi(r);
	channels     = atoi(c);
	cout << "Sound::Sound: sound_device = " << sound_device << ", rate = " << rate << ", channels = " << channels << endl;
}

Sound::~Sound() {
	// TODO Auto-generated destructor stub
}

