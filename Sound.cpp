/*
 * Sound.cpp
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#include "Sound.h"
#include "Mydefine.h"
#include <iostream>
using namespace std;

int  AlsaParams::ic7410_frequency {7026000};   /* static */
int  AlsaParams::soft66_frequency {7020000};   /* static */
bool AlsaParams::ic7410_changed   {false};     /* static */
bool AlsaParams::soft66_changed   {false};     /* static */

Sound::Sound() {
	cout << "Sound::Sound() begin.." << endl;
}

Sound::~Sound() {
	cout << "Sound::~Sound() begin.." << endl;
}

void Sound::set_ic7410_frequency(int i) { /* static function */
	ic7410_frequency = i;
    ic7410_changed   = true;
}

int Sound::asound_init() {
	int err;

	cout << "Sound::asound_init() begin.. \n"
		 << "  channels = " << channels << endl;

	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);

	if ((err = snd_pcm_open(&handle, sound_device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		cout  << "snd_pcm_open() error. " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if ((err = asound_set_hwparams()) < 0) {
		cout  << "setting of hwparams failed. " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if ((err = asound_set_swparams()) < 0) {
		cout  << "setting of swparams failed. " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if ((err = snd_pcm_prepare(handle)) < 0) {
		cout  << "snd_pcm_prepare error: " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if (snd_pcm_state(handle) == SND_PCM_STATE_PREPARED) {
		if ((err = snd_pcm_start(handle)) < 0) {
			cout  << "pcm_start error: " << snd_strerror(err) << endl;
			exit(EXIT_FAILURE);
		}
	} else {
		cout  << "snd_pcm_state is not PREPARED" << endl;
		exit(EXIT_FAILURE);
	}

	if (snd_pcm_state(handle) == SND_PCM_STATE_PREPARED) {
		if ((err = snd_pcm_start(handle)) < 0) {
			cout << "Sound::asound_go(): pcm_start error: " << snd_strerror(err) << endl;
			exit(EXIT_FAILURE);
		}
	}

	return 0;
}

int Sound::asound_read() {

//	cout << "Sound::asound_read(): count = " << count++ << ", channels = " << channels << endl;

	avail = snd_pcm_avail_update(handle);
//	cout << "Sound::asound_read(): " << "avail = " << avail << endl;

	if (avail == -EPIPE) {    /* under-run */
		cout << "Sound::asound_read(): " << "underrun occurred, trying to recover now .." << endl;
//		int err = snd_pcm_prepare(handle);
		int err = snd_pcm_recover(handle, -EPIPE, 0);
		if (err < 0) {
			cout << "Sound::asound_read(): " << "can not recover from underrun: " << snd_strerror(err) << endl;
		}
		cout << "Sound::asound_read(): " << "return here because underrun occurred." << endl;
	}

	int loop_count = 0;
	while (avail >= (snd_pcm_sframes_t) period_size) {
		frames_actually_read = snd_pcm_readi(handle, samples, period_size);
		cout << "Sound::asound_read(): " << "loop_count = " << loop_count++ << ", frames_actually_read = " << frames_actually_read << endl;

		if (frames_actually_read < 0) {
			cout << "Sound::asound_read(): " << "snd_pcm_readi error: " << snd_strerror(frames_actually_read) << endl;
			exit(EXIT_FAILURE);
		}
		if (frames_actually_read != (int) period_size) {
			cout << "Sound::asound_read(): " << "frames_actually_read (" << frames_actually_read
					<< ") does not match the period_size (" << period_size << endl;
			exit(EXIT_FAILURE);
		}

		/* copy samples into audio_signal */
		for (int i = 0; i < (int) (period_size * channels); i++) {
			audio_signal[i] = samples[i];
		}
		avail = snd_pcm_avail_update(handle);
		cout << "Sound::asound_read(): " << "in the while loop, avail = " << avail << endl;
	}

	return loop_count;
}

int Sound::asound_set_hwparams() {
	unsigned int rrate;
	int err;
	cout << "Sound::asound_set_hwparams() begin... \n"
	     << "  channels = " << channels << endl;

	/* choose all parameters */
	err = snd_pcm_hw_params_any(handle, hwparams);
	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Broken configuration for playback: no configurations available."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set hardware resampling disabled */
	err = snd_pcm_hw_params_set_rate_resample(handle, hwparams, resample);
	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Resampling setup failed for playback."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set the interleaved read/write format */
	err = snd_pcm_hw_params_set_access(handle, hwparams,
			SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Access type not available for playback."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set the sample format */
	err = snd_pcm_hw_params_set_format(handle, hwparams, SND_PCM_FORMAT_S16);

	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Sample format not available for playback."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set the count of channels */
	err = snd_pcm_hw_params_set_channels(handle, hwparams, channels);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: channels = " << channels
				<< " is not available." << snd_strerror(err)
				<< endl;
		return err;
	}

	/* set the stream rate */
	rrate = rate;
	err = snd_pcm_hw_params_set_rate_near(handle, hwparams, &rrate, 0);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: Rate " << rate
				<< " is not available for playbacks." << snd_strerror(err)
				<< endl;
		return err;
	}
	if (rrate != rate) {
		cout << "Sound::asound_set_hwparams: Rate does not match. Requested "
				<< rate << ", but get " << err << endl;
		return -EINVAL;
	}

	/* set the buffer size */
	err = snd_pcm_hw_params_set_buffer_size_near(handle, hwparams, &buffer_size);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: buffer size error = " << err << endl;
		return err;
	}

	/* set the period size */
	snd_pcm_hw_params_set_period_size_near(handle, hwparams, &period_size, NULL);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: period size error = " << err << endl;
		return err;
	}

	/* write the parameters to device */
	err = snd_pcm_hw_params(handle, hwparams);
	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Unable to set hw params for playback: "
		<< snd_strerror(err) << endl;
		return err;
	}

	return 0;
}

int Sound::asound_set_swparams() {
	int err;
	cout << "Sound::asound_set_swparams() begin... \n"
	     << "  channels = " << channels << endl;

	/* get the current swparams */
	err = snd_pcm_sw_params_current(handle, swparams);
	if (err < 0) {
		cout
		<< "Sound::asound_set_swparams: Unable to determine current swparams: "
		<< snd_strerror(err) << endl;
		return err;
	}

	/* start the transfer when the buffer is half full */
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, period_size * 2);
	if (err < 0) {
		cout
		<< "Sound::asound_set_swparams: Unable to set start threshold mode: "
		<< snd_strerror(err) << endl;
		return err;
	}

	/* allow the transfer when at least period_size samples can be processed */
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size * 2);
	if (err < 0) {
		cout
		<< "Sound::asound_set_swparams: Unable to set avail min: "
		<< snd_strerror(err) << endl;
		return err;
	}

	/* write the parameters to the device */
	err = snd_pcm_sw_params(handle, swparams);
	if (err < 0) {
		cout
		<< "Sound::asound_set_swparams: Unable to set sw params: "
		<< snd_strerror(err) << endl;
		return err;
	}

	return 0;
}

int Sound::rig_init_serial(char *serial_port)
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

int Sound::myread(unsigned char *myresponse)
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

int Sound::receive_fb(void)
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

int Sound::send_command(unsigned char *partial_command)
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

int Sound::send_commandx(const vector < unsigned char >&partial_command)
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

void Sound::set_operating_mode()
{
	static unsigned char command1[4] = { 0x06, 0x03, 0x01, 0xfd };

	command1[1] = operating_mode;
	command1[2] = dsp_filter;
	send_command(command1);
	receive_fb();
}

void Sound::set_operating_modex()
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

void set_ic7410_freq(long int ifreq_in_hz)
{

//	if(0) {
//	} else if(ifreq_in_hz >= 7000000 && ifreq_in_hz <= 7200000) {
//		jfreq_in_hz = 7000000 + 20000;
//	} else if(ifreq_in_hz >= 10100000 && ifreq_in_hz <= 10150000) {
//		jfreq_in_hz = 10100000 + 20000;
//	} else if(ifreq_in_hz >= 14000000 && ifreq_in_hz <= 14350000) {
//		jfreq_in_hz = 14000000 + 20000;
//	} else if(ifreq_in_hz >= 18068000 && ifreq_in_hz <= 18168000) {
//		jfreq_in_hz = 18068000 + 20000;
//	} else if(ifreq_in_hz >= 21000000 && ifreq_in_hz <= 21450000) {
//		jfreq_in_hz = 21000000 + 20000;
//	} else if(ifreq_in_hz >= 24890000 && ifreq_in_hz <= 24990000) {
//		jfreq_in_hz = 24890000 + 20000;
//	} else if(ifreq_in_hz >= 28000000 && ifreq_in_hz <= 29700000) {
//		jfreq_in_hz = 28000000 + 20000;
//	} else if(ifreq_in_hz >= 50000000 && ifreq_in_hz <= 54000000) {
//		jfreq_in_hz = 54000000 + 20000;
//	}
//
//	cout << "set_freq(): ifreq_in_hz = " << ifreq_in_hz << ", jfreq_in_hz = " << jfreq_in_hz << endl;
//
//	char string[128];
//	sprintf(string, "/usr/local/bin/soft66-control -t %8d",  jfreq_in_hz);
//	cout << "set_freq(): right now nogo because xrun occures, string = " << string << endl;
//	system(string);

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
//	send_command(command1);
//	receive_fb();
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
//	send_command(command1);
//	receive_fb();
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
//	send_command(command1);
//	receive_fb();
}

void Sound::myclock()
{
	static unsigned char command1[2] = { 0x03, 0xfd };
	static unsigned char command2[3] = { 0x15, 0x02, 0xfd };
	static unsigned char command3[2] = { 0x04, 0xfd };
	char string[256];
	unsigned char buf[255];
	int res;

	cout << "myclock(): begin.. \n";

	/* read operating mode, response in char[5]-char[6] */

	send_command(command3);
	res = myread(buf);
	if (res != 8) {
		fprintf(stderr, "operating mode response is wrong! \n");
	}
	operating_mode = buf[5];
	dsp_filter = buf[6];
	cout << "myclock: operating_mode = " << operating_mode << ", dsp_filter = " << dsp_filter << endl;

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
//	ic7410_freq_in_hz = atoi(string);
//	cout << "ifreq_in_hz = " << ic7410_freq_in_hz << endl;

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
