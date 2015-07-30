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

	int  get_frequency      ()    override;
	void set_frequency      (int) override;
	int  get_operating_mode ()    override;
	void set_operating_mode (int) override { ; };

	int rig_init_serial(char*);
	int myread(unsigned char*);
	int receive_fb();
	int send_command(unsigned char*);
	int mystrlen(unsigned char*);
	int mystrcmp(unsigned char*, unsigned char*);

};

#endif /* RIGIC7410_H_ */
