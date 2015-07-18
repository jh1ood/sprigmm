/*
 * Sound.h
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#ifndef SOUND_H_
#define SOUND_H_

#include "AlsaParams.h"
#include <asoundlib.h>
#include <termios.h>
#include <fcntl.h>
#include <vector>
using namespace std;

class Sound : public AlsaParams {
public:
	Sound();
	virtual ~Sound();

	virtual int  get_channels   () const = 0;
	virtual int  get_nfft       () const = 0;
	virtual int  get_spectrum_x () const = 0;
	virtual int  get_spectrum_y () const = 0;
	virtual int  get_waterfall_x() const = 0;
	virtual int  get_waterfall_y() const = 0;
	virtual int  get_timervalue () const = 0;
	virtual int  get_index(int, int, int) const = 0;
	virtual int  get_smeter     ()       = 0;
	virtual int  asound_fftcopy () = 0;
	virtual int  get_frequency  () = 0;
	virtual int  get_other_frequency() = 0;
	virtual void set_frequnecy(int) = 0;

	static void set_ic7410_frequency(int i);
	static void set_soft66_frequency(int i)  { soft66_frequency = i; }

	int asound_init();
	int asound_read();
	int asound_set_hwparams();
	int asound_set_swparams();

	int rig_init_serial(char *serial_port);
	int myread(unsigned char *myresponse);
	int receive_fb(void);
	int send_command(unsigned char *partial_command);
	int send_commandx(const vector < unsigned char >&partial_command);
	void set_operating_mode();
	void set_operating_modex();
	void myclock();

};

#endif /* SOUND_H_ */
