/* Rig.cpp */
#include "mydefine.h"
#include "radiobuttons.h"
#include "drawingarea.h"
#include "MyWindow.h"
#include "MySubwindow1.h"
#include "Scales.h"
#include "Sound.h"
#include <iostream>
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <asoundlib.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

int fd = -1;

unsigned int rate;		/* stream rate */
unsigned int channels;		/* count of channels */
int byte_per_sample = 2;	/* 16 bit format */
unsigned int buffer_time = 500000;	/* ring buffer length in us */
unsigned int period_time = 128000;	/* period time in us */
int resample = 0;		/* disable resample */
int period_event = 0;		/* produce poll event after each period */
int cw_pitch = 600;
int iwater = 0;
int nsamples;
double bin_size, waterfall_scale_x;
double amax = 14.0, amin = 7.0;
int ifreq_in_hz = 7026000;
int jfreq_in_hz = 7020000;
int s_meter;
int operating_mode = 3;		/* CW=03, CW-REV=07, LSB=00, USB=01 */
int dsp_filter = 1;		/* FIL1=01, FIL2=02, FIL3=03 */
snd_pcm_sframes_t buffer_size;
snd_pcm_sframes_t period_size;
snd_pcm_t *handle;
snd_pcm_hw_params_t *hwparams;
snd_pcm_sw_params_t *swparams;
double *in_real;
fftw_complex *in, *out;
fftw_plan p;
int flag_togo1 = 0, flag_togo2 = 0, flag_togo3 = 0, flag_togo4 = 0;

void rig_init_serial(char *);

Sound *mysound1;
Sound *mysound2;

int main(int argc, char *argv[])
{
    if (argc == 5) {
    	mysound1 = new Sound{argv[2], argv[3], argv[4]};
    } else if (argc == 8) {
        mysound1 = new Sound{argv[2], argv[3], argv[4]};
        mysound2 = new Sound{argv[5], argv[6], argv[7]};
    	char string[128];
    	sprintf(string, "/usr/local/bin/soft66-control -t %8d",  jfreq_in_hz);
    	cout << "main(): string = " << string << endl;
    	system(string);
//        system("/usr/local/bin/soft66-control -t 7020000");
    } else {
	cout << "Usage (IC-7410 only)      : " << argv[0] <<
	    " /dev/ttyUSB0 hw:2,0 32000 1 \n";
	cout << "Usage (IC-7410 and Soft66): " << argv[0] <<
	    " /dev/ttyUSB0 hw:2,0 32000 1 hw:0,0 48000 2 \n";
	return false;
    }

    cout << "main: serial_port = " << argv[1] << ", sound_device = " <<
	argv[2]
	<< ", rate = " << argv[3] << ", channels = " << argv[4] << endl;

    rig_init_serial(argv[1]);
    rate = atoi(argv[3]);
    channels = atoi(argv[4]);

    argc = 1;			/* just for the next line */
    Glib::RefPtr < Gtk::Application > app =
	Gtk::Application::create(argc, argv, "org.gtkmm.example");

    MyWindow win;
    win.set_title("IC-7410 Rig Control Program (C++ version)");
    win.set_default_size(50, 50);	/* dummy */
    win.set_border_width(5);
    win.show_all();
    return app->run(win);
}
