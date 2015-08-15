/*
 * RigParams.h
 *
 *  Created on: Jul 20, 2015
 *      Author: user1
 */

#ifndef RIGPARAMS_H_
#define RIGPARAMS_H_
#include <chrono>

struct RigParams {
public:
	int         frequency {7020000};
	int fd             {-1};
	int s_meter         {0};
	int nch             {0};

	static int  frequency_to_set;
	static bool frequency_to_go;
	static int  ic7410_frequency;
	static int  soft66_frequency;
	static int  cw_pitch;
	static int  operating_mode;	/* CW=03, CW-REV=07, LSB=00, USB=01 */
	static int  dsp_filter;		/* FIL1=01, FIL2=02, FIL3=03 */
//	std::chrono::system_clock::time_point start;
	std::chrono::system_clock::time_point current;
	std::chrono::system_clock::time_point current_b4;

};

#endif /* RIGPARAMS_H_ */
