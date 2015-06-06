/*
 * Usbsoundiq.h
 *
 *  Created on: Jun 4, 2015
 *      Author: user1
 */

#ifndef USBSOUNDIQ_H_
#define USBSOUNDIQ_H_

#include <iostream>
#include "Usbsound.h"
using namespace std;

class Usbsound_iq: public Usbsound {
public:
	Usbsound_iq();
	Usbsound_iq(char *s);
	virtual ~Usbsound_iq();
//	virtual void output() { cout << "usbsound_iq output" << endl; };
private:
	string myid = "Usbsound_iq::Usbsound_iq(): ";
};

#endif /* USBSOUNDIQ_H_ */
