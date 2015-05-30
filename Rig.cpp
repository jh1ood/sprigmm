#include "mydefine.h"
#include "radiobuttons.h"
#include "drawingarea.h"
#include "MyWindow.h"
#include "MySubwindow1.h"
#include "Scales.h"
#include "Sound.h"
#include "idlefunc.h"
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
int ifreq_in_hz = 7026000;
int jfreq_in_hz = 7020000;
int s_meter;
int operating_mode = 3;		/* CW=03, CW-REV=07, LSB=00, USB=01 */
int dsp_filter     = 1;		/* FIL1=01, FIL2=02, FIL3=03 */
int flag_togo[3] = {0, 0, 0}; /* [0] is dummy */

void rig_init_serial(char *);

Sound  *mysound[2];
struct timeval t0;

int main(int argc, char *argv[])
{
	if (argc != 4) {
		cout << "Usage: " << argv[0] << " hw:2,0 hw:1,0 /dev/ttyUSB1" << endl;
		return 1;
	}

	gettimeofday(&t0, NULL);
	cout << "main(): gettimeofday = " << t0.tv_sec << "." << setfill('0') << setw(6) <<  t0.tv_usec << endl;

	mysound[0] = new Sound{argv[1], 1}; /* IC-7410   */
	mysound[1] = new Sound{argv[2], 2}; /* Soft66LC4 */
	rig_init_serial(argv[3]);

	argc = 1;			/* just for the next line */
	Glib::RefPtr < Gtk::Application > app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

	MyWindow win;
	win.set_title("IC-7410 Rig Control Program (testing now..)");
	win.set_default_size(2058, 50);	/* dummy */
	win.set_border_width(5);
	win.show_all();
	return app->run(win);
}
