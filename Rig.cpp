/*
 * Rig.cpp
 *
 */

#include "Rig.h"
#include <iostream>
using namespace std;

int  RigParams::frequency_to_set =       0;
bool RigParams::frequency_to_go  =   false;
int  RigParams::ic7410_frequency = 7026000;
int  RigParams::soft66_frequency = 7020000;
int  RigParams::cw_pitch         =     600;
int  RigParams::operating_mode  {3};		/* CW=03, CW-REV=07, LSB=00, USB=01 */
int  RigParams::dsp_filter      {1};		/* FIL1=01, FIL2=02, FIL3=03 */

Rig::~Rig() {
	cout << "Rig::~Rig() destructor.." << endl;
}

