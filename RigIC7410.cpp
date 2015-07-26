/*
 * RigIC7410.cpp
 *
 *  Created on: Jul 20, 2015
 *      Author: user1
 */

#include "RigIC7410.h"
#include <iostream>
#include <unistd.h>
using namespace std;
#define END_OF_COMMAND          0xfd

RigIC7410::RigIC7410(char *s) {
	rig_init_serial(s);
}

RigIC7410::~RigIC7410() {
}

int RigIC7410::myread(unsigned char *myresponse)
{
	int res = 0;
	unsigned char mybuf[256], *p;
	cout << "Sound::myread is called.." << endl;

	p = myresponse;
	do {
		int t = read(fd, mybuf, 1);
		if (t != 1) {
			fprintf(stderr, "error in read \n");
			return -1;
		}
		*p++ = mybuf[0];
		res++;
	}
	while (mybuf[0] != 0xfd);

	return res;
}

int RigIC7410::receive_fb(void)
{
	unsigned char response[256];
	unsigned char fb_message[6] = { 0xfe, 0xfe, 0xe0, 0x80, 0xfb, 0xfd };
	int n;

	n = myread(response);	/* get echo back */

	if (mystrcmp(response, fb_message) != 0) {
		fprintf(stderr, "*** error *** not a FB message. ");
		for (int i = 0; i < n; i++) {
			fprintf(stderr, "%02x ", response[i]);
		}
		fprintf(stderr, "\n");
		return false;
	}

	return true;
}

int RigIC7410::mystrlen(unsigned char *string)
{
	unsigned char *t;
	for (t = string; *t != END_OF_COMMAND; t++) {;
	}
	return (t - string) + 1;	/* +1 to include EOC */
}

int RigIC7410::mystrcmp(unsigned char *string1, unsigned char *string2)
{
	unsigned char *t1;
	unsigned char *t2;

	for (t1 = string1, t2 = string2; *t1 == *t2 && *t1 != END_OF_COMMAND;
			t1++, t2++) {;
	}
	return *t1 - *t2;
}

int RigIC7410::send_command(unsigned char *partial_command)
{
	int n_partial, n_command;
	int n_echoback;
	unsigned char command[256] = { 0xfe, 0xfe, 0x80, 0xe0 };	/* preamble */
	unsigned char echoback[256];

	n_partial = mystrlen(partial_command);
	n_command = n_partial + 4;	/* add preamble(4) */
	for (int i = 0; i < n_partial; i++) {
		command[4 + i] = partial_command[i];
	}
	command[n_command - 1] = 0xfd;
	cout << "send_command: fd = " << fd << endl;
	write(fd, command, n_command);
	n_echoback = myread(echoback);

#ifdef DEBUG
	unsigned char *s;
	s = command;
	fprintf(stderr, "send_command: n_command  = %2d, command  = ",
			n_command);
	for (int i = 0; i < n_command; i++) {
		fprintf(stderr, "[%02x] ", *s++);
	}
	fprintf(stderr, "\n");
	fprintf(stderr, "              n_echoback = %2d, echoback = ",
			n_echoback);
	s = echoback;
	for (int i = 0; i < n_echoback; i++) {
		fprintf(stderr, "[%02x] ", *s++);
	}
	fprintf(stderr, "\n");
#endif

	if ((n_echoback != n_command) || (mystrcmp(command, echoback) != 0)) {
		fprintf(stderr,
				"   send_command():  *** error *** echoback does not much. \n");
		return false;
	}

	return true;
}

int RigIC7410::get_operating_mode() {
	/* read operating mode, response in char[5]-char[6] */
	static unsigned char command3[2] = { 0x04, 0xfd };
	unsigned char buf[255];
	int res;

	send_command(command3);
	res = myread(buf);
	if (res != 8) {
		fprintf(stderr, "operating mode response is wrong! \n");
	}
	operating_mode = buf[5];
	dsp_filter     = buf[6];
	cout << "myclock: operating_mode = " << operating_mode << ", dsp_filter = " << dsp_filter << endl;

	return operating_mode;
}

int RigIC7410::get_frequency() {
	/* freq response in char[8]-char[5] */

	static unsigned char command1[2] = { 0x03, 0xfd };
	char string[256];
	unsigned char buf[255];
	int res;

	send_command(command1);
	res = myread(buf);
	if (res != 11) {
		cout << "frequency response is wrong!" << endl;
	}
	sprintf(string, "%02x%02x%02x%02x", buf[8], buf[7], buf[6], buf[5]);
	frequency = atoi(string);
	cout << "res = " << res << ", frequency = " << frequency << endl;
	ic7410_frequency = frequency;

	return frequency;
}

void RigIC7410::set_frequency(int freq) {

	cout << "RigIC7410::set_frequency() freq = " << freq << endl;
	static unsigned char command1[7] =
	{ 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd };
	long int ifreq_wrk;
	int idigit[8];

	ifreq_wrk = freq;

	for (int i = 0; i < 8; i++) {
		idigit[i] = ifreq_wrk % 10;
		ifreq_wrk /= 10;
	}
	command1[1] = 16 * idigit[1] + idigit[0];
	command1[2] = 16 * idigit[3] + idigit[2];
	command1[3] = 16 * idigit[5] + idigit[4];
	command1[4] = 16 * idigit[7] + idigit[6];
	send_command(command1);
	receive_fb();

}

int RigIC7410::rig_init_serial(char *serial_port)
{
	struct termios tio;

	std::cout << "rig_init_serial: begin ... \n";

	fd = open(serial_port, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		std::cout << "error: can not open " << serial_port << endl;
		return 1;
	}
	cout << "fd = " << fd << endl;

	memset(&tio, 0, sizeof(tio));
	tio.c_cflag = CS8 | CLOCAL | CREAD;
	tio.c_cc[VEOL] = 0xfd;	/* IC-7410 postamble */
	tio.c_lflag = 0;		/* non canonical mode */
	tio.c_cc[VTIME] = 0;	/* non canonical mode */
	tio.c_cc[VMIN] = 1;		/* non canonical mode */

	tio.c_iflag = IGNPAR | ICRNL;
	cfsetispeed(&tio, B19200); /* baudrate 19200 */
	cfsetospeed(&tio, B19200);
	tcsetattr(fd, TCSANOW, &tio);

	return 0;
}
