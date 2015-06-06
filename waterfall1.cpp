#include "mydefine.h"
#include "waterfall1.h"
#include "Sound.h"
#include "Usbsound.h"
#include <cairomm/context.h>
#include <gdkmm/general.h>	// set_source_pixbuf()
#include <glibmm/fileutils.h>
#include <cmath>
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <cairo.h>
#include <glibmm/main.h>
#include <asoundlib.h>
#include <fftw3.h>
#include <time.h>
#include <sys/time.h>
#include <ctime>

using namespace std;
//extern Sound *mysound[];
extern Usbsound *mysound[];
extern struct timeval t0;

void set_ic7410_freq(long int ifreq_in_hz);
int colormap_r(double);
int colormap_g(double);
int colormap_b(double);

Waterfall1::Waterfall1() {
	std::cout << "Waterfall1 constructor is called." << std::endl;
	string myid ="Waterfall1::Waterfall1: ";

	set_size_request(WATERFALL_XSIZE, WATERFALL_WSIZE + WATERFALL_YSIZE);
	m_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, WATERFALL_XSIZE, WATERFALL_WSIZE + WATERFALL_YSIZE);

	guint8* p = m_image->get_pixels();

	nfft = 2 * 1024; /* Soft66LC4 */

	bin_size = (double) mysound[1]->rate / (double) nfft;
	cout << myid << "nfft = " << nfft << ", rate =" << mysound[1]->rate << ", bin_size = " << bin_size << endl;

	fft_window         = new double [nfft];
	audio_signal_ffted = new double [nfft];
	for (int j = 0; j < nfft; j++) {
		fft_window[j] = 0.54 - 0.46 * cos(2.0 * M_PI * j / (double) nfft);
	}

	/* plan: FFTW_ESTIMATE, FFTW_MEASURE, FFTW_PATIENT, FFTW_EXHAUSTIVE */
	in   = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft);
	out  = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft);
	plan = fftw_plan_dft_1d    (nfft, in, out, FFTW_FORWARD, FFTW_MEASURE);

	/* Soft66LC4 marker area */
	for (int j = 0; j < WATERFALL_WSIZE; j++) {
		for (int i = 0; i < WATERFALL_XSIZE; i++) {
			int tmp = 128 + 127 * ( ( (int) ( abs(i-WATERFALL_XSIZE/2) * bin_size / 5000.0) ) % 2);
			*p++ = tmp;
			*p++ = tmp;
			*p++ = tmp;
		}
	}

	/* Soft66LC4 waterfall area */
	for (int j = 0; j < WATERFALL_YSIZE; j++) {
		for (int i = 0; i < WATERFALL_XSIZE; i++) {
			*p++ = 0;
			*p++ = (i-j) % 256;
			*p++ = j % 256;
		}
	}

	Glib::signal_timeout().connect(sigc::mem_fun(*this, &Waterfall1::on_timeout), 160 ); /**************/

#ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
	// actually, this ifndef part is not necessary.
	cout << myid << "should not occur." << endl;
	signal_draw().connect(sigc::mem_fun(*this, &Waterfall1::on_draw), false);
#endif				//GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

	add_events(	Gdk::BUTTON_PRESS_MASK );
//	mysound[1]->Sound_go(); /* start audio device */
	mysound[1]->Usbsound_go(); /* start audio device */

	std::cout << "Waterfall1 constructor end.." << std::endl;

}

Waterfall1::~Waterfall1() {
	std::cout << "Waterfall1 destructor is called." << std::endl;
}

bool Waterfall1::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
	static int icountx = 0;
	static struct timeval t1, t1b4, t9;
	gettimeofday(&t1, NULL);
	string myid = "Waterfall1::on_draw: ";

	cout << myid << "icountx = " << icountx++ << endl;
	int rowstride = m_image->get_rowstride();

	bool ready = {false};
	ready = mysound[1]->asound_myread();
	if(ready) {
		cout << myid << " ready" << endl;
	} else {
		cout << myid << " not ready" << endl;
	}

	/* shift down pixbuf for waterfall area */
	guint8* p = m_image->get_pixels() + (WATERFALL_WSIZE + WATERFALL_YSIZE) * rowstride - 1;
	guint8* q = p - WATERFALL_XSIZE * 3;
	for(int i = 0;i<WATERFALL_YSIZE*WATERFALL_XSIZE*3;i++) {
		*p-- = *q--;
	}

	/* audio signal FFT */
	for (int i = 0; i < nfft; i++) { /* I and Q reversed */
		in[i][1] = fft_window[i] * mysound[1]->audio_signal[2 * i];
		in[i][0] = fft_window[i] * mysound[1]->audio_signal[2 * i + 1];
	}
	fftw_execute(plan);

	/* log10 and normalize */

	amax = 13.0;
	amin =  6.0;
	for (int i = 0; i < nfft; i++) {
		double val;
		val = out[i][0] * out[i][0] + out[i][1] * out[i][1];
		if (val < pow(10.0, amin)) {
			audio_signal_ffted[i] = 0.0;
		} else if (val > pow(10.0, amax)) {
			audio_signal_ffted[i] = 1.0;
		} else {
			audio_signal_ffted[i] = (log10(val) - amin) / (amax - amin);
		}
	}

	// write one line for Soft66LC4
	p = m_image->get_pixels() + rowstride * WATERFALL_WSIZE;
	for (int i = 0; i < WATERFALL_XSIZE; i++) {
		double tmp = audio_signal_ffted[(i + WATERFALL_XSIZE/2) % WATERFALL_XSIZE];
		*p++ = colormap_r(tmp);
		*p++ = colormap_g(tmp);
		*p++ = colormap_b(tmp);
	}

	/* Soft66LC4 marker area */
	p = m_image->get_pixels() + rowstride * 0;
	for (int j = 0; j < WATERFALL_WSIZE; j++) {
		for (int i = 0; i < WATERFALL_XSIZE; i++) {
			int tmp = 128 + 127 * ( ( (int) ( abs(i-WATERFALL_XSIZE/2) * bin_size / 5000.0) ) % 2);
			if( abs( (i-WATERFALL_XSIZE/2) * bin_size + 7020000 - ic7410_freq_in_hz) < 200.0) {
				tmp = 0;
			}
			*p++ = tmp;
			*p++ = tmp;
			*p++ = tmp;
		}
	}


	Gdk::Cairo::set_source_pixbuf(cr, m_image, WATERFALL_XOFFSET, WATERFALL_YOFFSET);
	cr->paint();
	cr->stroke();

	gettimeofday(&t9, NULL);
	cout_gettimeofday_diff(myid+"             elapsed:  ", t0  , t1);
	cout_gettimeofday_diff(myid+"             interval: ", t1b4, t1);
	cout_gettimeofday_diff(myid+"             duration: ", t1  , t9);
	t1b4 = t1;

	return true;
}

bool Waterfall1::on_timeout() {
	static int icount = 0;

	static struct timeval t1, t1b4, t9;
	gettimeofday(&t1, NULL);
	string myid = "Waterfall1::on_timeout: ";

	cout << myid << "icountw = " << icount++ <<  endl;

	Glib::RefPtr<Gdk::Window> win = get_window();
	if (win) {
		cout << "Waterfall1::on_timeout: get_window OK" << endl;
		cout << "Waterfall1::on_timeout: width  " << get_allocation().get_width() << endl;
		cout << "Waterfall1::on_timeout: height " << get_allocation().get_height() << endl;
		win->invalidate(false);
	} else {
		cout << "Waterfall1::on_timeout: get_window NG" << endl;
	}

	gettimeofday(&t9, NULL);
	cout_gettimeofday_diff(myid+"          elapsed:  ", t0  , t1);
	cout_gettimeofday_diff(myid+"          interval: ", t1b4, t1);
	cout_gettimeofday_diff(myid+"          duration: ", t1  , t9);
	t1b4 = t1;

	return true;
}

bool Waterfall1::on_button_press_event(GdkEventButton * event) {

	x_press = event->x;
	y_press = event->y;

	std::cout << "Waterfall1::on_button_press_event:  x_press = " << x_press
			<< ", y_press = "     << y_press << endl;

	ic7410_freq_in_hz = 7020000 + (x_press - (WATERFALL_XSIZE/2) ) * bin_size;
	set_ic7410_freq(ic7410_freq_in_hz);

	return true;
}
