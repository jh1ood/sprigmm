/*
 * RigIC7410.h
 *
 *  Created on: Jul 20, 2015
 *      Author: user1
 */

#ifndef RIGIC7410_H_
#define RIGIC7410_H_

#include "Rig.h"
#include <termios.h>
#include <fcntl.h>
#include <cstring>

class RigIC7410 : public Rig {
public:
	RigIC7410(char*);
	virtual ~RigIC7410();
	int  get_frequency() override;
	void set_frequency(int) override;
	int rig_init_serial(char*);
	int myread(unsigned char*);
	int receive_fb();
	int send_command(unsigned char*);
	int mystrlen(unsigned char*);
	int mystrcmp(unsigned char*, unsigned char*);

private:
	int fd             {-1};
	int cw_pitch      {600};
	int s_meter         {0};
	int operating_mode  {3};		/* CW=03, CW-REV=07, LSB=00, USB=01 */
	int dsp_filter      {1};		/* FIL1=01, FIL2=02, FIL3=03 */
};

#endif /* RIGIC7410_H_ */
