/*
 * Rigcontrol.cpp
 *
 *  Created on: Apr 18, 2015
 *      Author: user1
 */

#include "Rigcontrol.h"
#include <iostream>
#define BAUDRATE                B19200
extern int fd;

Rigcontrol::Rigcontrol(char *serial_port) {
	struct termios tio;

	std::cout << "Rigcontrol::Rigcontrol: serial_port = " << serial_port << endl;

	fd = open(serial_port, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		std::cout << "Rigcontrol::Rigcontrol: error, can not open the serial_port \n";
	}

	memset(&tio, 0, sizeof(tio));
	tio.c_cflag = CS8 | CLOCAL | CREAD;
	tio.c_cc[VEOL] = 0xfd; /* IC-7410 postamble */
	tio.c_lflag = 0; /* non canonical mode */
	tio.c_cc[VTIME] = 0; /* non canonical mode */
	tio.c_cc[VMIN] = 1; /* non canonical mode */

	tio.c_iflag = IGNPAR | ICRNL;
	cfsetispeed(&tio, BAUDRATE);
	cfsetospeed(&tio, BAUDRATE);
	tcsetattr(fd, TCSANOW, &tio);
}

Rigcontrol::~Rigcontrol() {
}
