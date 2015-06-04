#include "mydefine.h"
#include "radiobuttons.h"
#include "drawingarea.h"
#include "MyWindow.h"
#include "Scales.h"
#include "Sound.h"
#include <iostream>
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <asoundlib.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <ctime>
#include <iomanip>
using namespace std;

int fd = -1;
int cw_pitch = 600;
int ic7410_freq_in_hz = 7026000;
int soft66_freq_in_hz = 7020000;
int s_meter;
int operating_mode = 3;		/* CW=03, CW-REV=07, LSB=00, USB=01 */
int dsp_filter     = 1;		/* FIL1=01, FIL2=02, FIL3=03 */

Sound  *mysound[2];	/* IC-7410, and Soft66LC4 */
struct timeval t0;

int main(int argc, char *argv[])
{
	if (argc != 4) {
		cout << "Usage example: " << argv[0] << " hw:2,0 hw:1,0 /dev/ttyUSB1" << endl;
		cout << "Try % arecord -l, and % ls -l /dev/ttyUSB* for appropriate parameters." << endl;
		return 1;
	}

	gettimeofday(&t0, NULL);
	cout << "main(): gettimeofday = " << t0.tv_sec << "." << setfill('0') << setw(6) <<  t0.tv_usec << endl;

	mysound[0] = new Sound{argv[1], 1}; /* IC-7410   sound output */
	mysound[1] = new Sound{argv[2], 2}; /* Soft66LC4 sound output */
	if( rig_init_serial(argv[3]) ) {	/* IC-7410 serial port for rig control */
		cout << "main(): error in rig_init_serial(" << argv[3] << ")" << endl;
		return 1;
	}

	argc = 1;			/* just for the next line */
	Glib::RefPtr < Gtk::Application > app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

	MyWindow win;
	win.set_title("IC-7410 Rig Control Program (C++ version)");
	win.set_default_size(5+2048+5, 50);	/* dummy */
	win.set_border_width(5);
	win.show_all();
	return app->run(win);
}
