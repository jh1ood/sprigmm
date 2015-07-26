/*
 * Rig.cpp
 *
 */

#include "Rig.h"

int  RigParams::frequency_to_set =       0;
bool RigParams::frequency_to_go  =   false;
int  RigParams::ic7410_frequency = 7026111;
int  RigParams::soft66_frequency = 7020222;
int  RigParams::cw_pitch         =     600;
int  RigParams::operating_mode  {3};		/* CW=03, CW-REV=07, LSB=00, USB=01 */
int  RigParams::dsp_filter      {1};		/* FIL1=01, FIL2=02, FIL3=03 */


Rig::Rig() {
	// TODO Auto-generated constructor stub

}

Rig::~Rig() {
	// TODO Auto-generated destructor stub
}

