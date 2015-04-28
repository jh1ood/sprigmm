#include "mydefine.h"
#include "radiobuttons.h"
#include "drawingarea.h"
#include "Scales.h"
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <iostream>
#include <termios.h>
#include <fcntl.h>
using namespace std;

int rig_init_serial(char *serial_port)
{
	struct termios tio;

	std::cout << "rig_init_serial: begin ... \n";

	fd = open(serial_port, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		std::cout << "error: can not open " << serial_port << endl;
		return 1;
	}

	memset(&tio, 0, sizeof(tio));
	tio.c_cflag = CS8 | CLOCAL | CREAD;
	tio.c_cc[VEOL] = 0xfd;	/* IC-7410 postamble */
	tio.c_lflag = 0;		/* non canonical mode */
	tio.c_cc[VTIME] = 0;	/* non canonical mode */
	tio.c_cc[VMIN] = 1;		/* non canonical mode */

	tio.c_iflag = IGNPAR | ICRNL;
	cfsetispeed(&tio, BAUDRATE);
	cfsetospeed(&tio, BAUDRATE);
	tcsetattr(fd, TCSANOW, &tio);

	return 0;
}


int mystrlen(unsigned char *string)
{
	unsigned char *t;
	for (t = string; *t != END_OF_COMMAND; t++) {;
	}
	return (t - string) + 1;	/* +1 to include EOC */
}

int mystrcmp(unsigned char *string1, unsigned char *string2)
{
	unsigned char *t1;
	unsigned char *t2;

	for (t1 = string1, t2 = string2; *t1 == *t2 && *t1 != END_OF_COMMAND;
			t1++, t2++) {;
	}
	return *t1 - *t2;
}

int myread(unsigned char *myresponse)
{
	int res = 0;
	unsigned char mybuf[256], *p;

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

int receive_fb(void)
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

int send_command(unsigned char *partial_command)
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

int send_commandx(const vector < unsigned char >&partial_command)
{
	int n_partial, n_command;
	int n_echoback;
	unsigned char command[256] = { 0xfe, 0xfe, 0x80, 0xe0 };	/* preamble */
	unsigned char echoback[256];

	n_partial = partial_command.size();
	cout << "send_commandx: n_partial = " << n_partial << endl;

	n_command = 4 + n_partial + 1;	/* add preamble(4) and EOC(1) */
	for (int i = 0; i < n_partial; i++) {
		command[4 + i] = partial_command.at(i);
	}
	command[n_command - 1] = 0xfd;	/* End_Of_Command */

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
				"   send_commandx():  *** error *** echoback does not much. \n");
		return false;
	}

	return true;
}

void set_operating_mode(void)
{
	static unsigned char command1[4] = { 0x06, 0x03, 0x01, 0xfd };

	command1[1] = operating_mode;
	command1[2] = dsp_filter;
	send_command(command1);
	receive_fb();
}

void set_operating_modex(void)
{
	vector < unsigned char >command1 = { 0x06, 0x03, 0x01 };	/* without EOC */

	cout << "set_operating_modex: operating_mode = " << operating_mode << " , dsp_filter = " << dsp_filter << endl;
	command1[1] = operating_mode;
	command1[2] = dsp_filter;
	send_commandx(command1);
	receive_fb();
}

struct async_private_data {
	signed short *xsamples;
	snd_pcm_sframes_t xperiod_size;
	int dummy;
};

void set_freq(long int ifreq_in_hz)
{
	fprintf(stderr, "freq set to %12.3f [kHz] \n",
			(double) ifreq_in_hz / 1000.0);
	static unsigned char command1[7] =
	{ 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd };
	long int ifreq_wrk;
	int idigit[8];

	ifreq_wrk = ifreq_in_hz;

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

int colormap_r(double tmp)
{
	double val;
	if (tmp < 0.50) {
		val = 0.0;
	} else if (tmp > 0.75) {
		val = 1.0;
	} else {
		val = 4.0 * tmp - 2.0;
	}
	return (int) (255.0 * val);
}

int colormap_g(double tmp)
{
	double val;
	if (tmp < 0.25) {
		val = 4.0 * tmp;
	} else if (tmp > 0.75) {
		val = -4.0 * tmp + 4.0;
	} else {
		val = 1.0;
	}
	return (int) (255.0 * val);
}

int colormap_b(double tmp)
{
	double val;
	if (tmp < 0.25) {
		val = 1.0;
	} else if (tmp > 0.50) {
		val = 0.0;
	} else {
		val = -4.0 * tmp + 2.0;
	}
	return (int) (255.0 * val);
}

void set_cw_speed(int wpm)
{
	static unsigned char command1[5] = { 0x14, 0x0c, 0x00, 0x32, 0xfd };
	int iii, i100, i10, i1;

	if (wpm < 6)
		wpm = 6;
	if (wpm > 48)
		wpm = 48;
	iii = 255 * (wpm - 6) / (48 - 6);
	i100 = iii / 100;
	i10 = (iii - 100 * i100) / 10;
	i1 = iii % 10;
	//  fprintf(stderr, "wpm changed %d %d %d %d \n", wpm, i100, i10, i1);
	command1[2] = i100;
	command1[3] = 16 * i10 + i1;
	send_command(command1);
	receive_fb();
}

void set_tx_power(int txp)
{
	static unsigned char command1[5] = { 0x14, 0x0a, 0x00, 0x32, 0xfd };
	int iii, i100, i10, i1;

	if (txp < 2)
		txp = 2;
	if (txp > 100)
		txp = 100;
	iii = 255.0 * (txp - 2) / 100.0;
	i100 = iii / 100;
	i10 = (iii - 100 * i100) / 10;
	i1 = iii % 10;
	//  fprintf(stderr, "txpower changed %d %d %d %d \n", txpower, i100, i10, i1);
	command1[2] = i100;
	command1[3] = 16 * i10 + i1;
	send_command(command1);
	receive_fb();
}

void myclock()
{
	static unsigned char command1[2] = { 0x03, 0xfd };
	static unsigned char command2[3] = { 0x15, 0x02, 0xfd };
	static unsigned char command3[2] = { 0x04, 0xfd };
	char string[256];
	unsigned char buf[255];
	int res;

	cout << "myclock(): begin.. \n";

	/* read operating mode, response in char[5]-char[6] */

//	send_command(command3);
//	res = myread(buf);
//	if (res != 8) {
//		fprintf(stderr, "operating mode response is wrong! \n");
//	}
//	operating_mode = buf[5];
//	dsp_filter = buf[6];
//	cout << "myclock: operating_mode = " << operating_mode << ", dsp_filter = " << dsp_filter << endl;

	/* freq response in char[8]-char[5] */
	cout << "myclock(): going to send_command() \n";
	send_command(command1);
	cout << "myclock(): going to myread() \n";
	res = myread(buf);
	cout << "myclock(): returned from myread() \n";

#ifdef DEBUG
	unsigned char *s;
	fprintf(stderr, "response for frequncy read, res = %2d : ", res);
	s = buf;
	for (int i = 0; i < res; i++) {
		fprintf(stderr, "[%02x] ", *s++);
	}
	fprintf(stderr, "\n");
#endif

	if (res != 11) {
		fprintf(stderr, "frequency response is wrong! \n");
	}
	sprintf(string, "%02x%02x%02x%02x", buf[8], buf[7], buf[6], buf[5]);
	ifreq_in_hz = atoi(string);
	cout << "ifreq_in_hz = " << ifreq_in_hz << endl;

	/* S-meter response in char[6]-char[5] */

	send_command(command2);
	res = myread(buf);

	if (res != 9) {
		fprintf(stderr, "S-meter response is wrong! \n");
	}

	sprintf(string, "%02x%02x", buf[6], buf[7]);
	s_meter = atoi(string);
	cout << "s_meter = " << s_meter << endl;

	cout << "myclock(): end.. \n";

	return;
}
