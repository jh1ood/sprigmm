/*
 * Rigcontrol.h
 *
 *  Created on: Apr 18, 2015
 *      Author: user1
 */

#ifndef RIGCONTROL_H_
#define RIGCONTROL_H_
#include <iostream>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
using namespace std;

class Rigcontrol {
public:
	Rigcontrol(char *serial_port);
	virtual ~Rigcontrol();
};

#endif /* RIGCONTROL_H_ */
