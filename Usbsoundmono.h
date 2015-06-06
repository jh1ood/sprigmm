/*
 * Usbsoundmono.h
 *
 *  Created on: Jun 4, 2015
 *      Author: user1
 */

#ifndef USBSOUNDMONO_H_
#define USBSOUNDMONO_H_

#include <iostream>
#include "Usbsound.h"
using namespace std;

class Usbsound_mono: public Usbsound {
public:
	Usbsound_mono();
	Usbsound_mono(char *s);
	virtual ~Usbsound_mono();
//	virtual void output() { cout << "usbsound_mono output" << endl; };
private:
	string myid = "Usbsound_mono::Usbsound_mono(): ";
};

#endif /* USBSOUNDMONO_H_ */
