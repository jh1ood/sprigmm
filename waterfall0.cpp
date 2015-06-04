#include "mydefine.h"
#include "waterfall0.h"
#include "Sound.h"
#include <cairomm/context.h>
#include <gtkmm/drawingarea.h>
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
extern Sound *mysound[];
extern struct timeval t0;

void set_ic7410_freq(long int ifreq_in_hz);
int colormap_r(double);
int colormap_g(double);
int colormap_b(double);

Waterfall0::Waterfall0() {
	string myid ="Waterfall0::Waterfall0: ";
	cout << myid << "constructor is called." << endl;

	set_size_request(WATERFALL0_XSIZE, WATERFALL_YSIZE + WATERFALL_ZSIZE);
	m_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, WATERFALL0_XSIZE, WATERFALL_YSIZE + WATERFALL_ZSIZE);

	guint8* p = m_image->get_pixels();

	nfft = 4 * 1024; /* IC-7410 */
	bin_size = (double) mysound[0]->rate / (double) nfft;
	cout << myid << "nfft = " << nfft << ", rate =" << mysound[0]->rate << ", bin_size = " << bin_size << endl;

	fft_window         = new double [nfft];
	audio_signal_ffted = new double [nfft];
	for (int j = 0; j < nfft; j++) {
		fft_window[j] = 0.54 - 0.46 * cos(2.0 * M_PI * j / (double) nfft);
	}

	/* plan: FFTW_ESTIMATE, FFTW_MEASURE, FFTW_PATIENT, FFTW_EXHAUSTIVE */
	in_real = new double[nfft];
	out  = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * ( nfft/2 + 1 )*2 );
	plan = fftw_plan_dft_r2c_1d(nfft, in_real, out, FFTW_MEASURE);

	/* IC-7410 spectrum area and waterfall area */
	for (int j = 0; j < WATERFALL_ZSIZE + WATERFALL_YSIZE; j++) {
		for (int i = 0; i < WATERFALL0_XSIZE; i++) {
			*p++ = 0;
			*p++ = (int) ( 255.0 * (double)i / (double) WATERFALL0_XSIZE );
			*p++ = 0;
		}
	}

	Glib::signal_timeout().connect(sigc::mem_fun(*this, &Waterfall0::on_timeout), 150 ); /**************/

#ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
	// actually, this ifndef part is not necessary.
	cout << myid << "should not occur." << endl;
	signal_draw().connect(sigc::mem_fun(*this, &Waterfall0::on_draw), false);
#endif				//GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

	add_events(	Gdk::BUTTON_PRESS_MASK );
	mysound[0]->Sound_go(); /* start audio device */

	std::cout << "Waterfall0 constructor end.." << std::endl;
}

Waterfall0::~Waterfall0() {
	std::cout << "Waterfall0 destructor is called." << std::endl;
}

bool Waterfall0::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
	static int icountx = 0;
	static struct timeval t1, t1b4, t9;
	gettimeofday(&t1, NULL);
	string myid = "Waterfall0::on_draw: ";

	cout << myid << "icountx = " << icountx++ << endl;
	int rowstride = m_image->get_rowstride();

	bool ready = {false};
	ready = mysound[0]->asound_myread();
	if(ready) {
		cout << myid << " ready" << endl;
	} else {
		cout << myid << " not ready" << endl;
	}

	/* shift down pixbuf for waterfall area */
	guint8* p = m_image->get_pixels() + (WATERFALL_ZSIZE + WATERFALL_YSIZE) * rowstride - 1;
	guint8* q = p - WATERFALL0_XSIZE * 3;
	for(int i = 0;i<WATERFALL_YSIZE*WATERFALL0_XSIZE*3;i++) {
		*p-- = *q--;
	}

	/* audio signal FFT */
	for (int i = 0; i < nfft; i++) {
		in_real[i] = fft_window[i] * mysound[0]->audio_signal[i];
	}
	fftw_execute(plan);

	/* log10 and normalize */

	amax = 14.0;
	amin =  7.0;
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

	// write one line for IC-7410
	p = m_image->get_pixels() + rowstride * WATERFALL_ZSIZE;
	for (int i = 0; i < WATERFALL0_XSIZE; i++) {
		double tmp = audio_signal_ffted[i];
		*p++ = colormap_r(tmp);
		*p++ = colormap_g(tmp);
		*p++ = colormap_b(tmp);
	}

	Gdk::Cairo::set_source_pixbuf(cr, m_image, WATERFALL_XOFFSET, WATERFALL_YOFFSET);
	cr->paint();
	cr->stroke();

	/* IC-7410 spectrum */
	cr->save();
	cr->set_source_rgba(0.2, 0.9, 0.9, 1.0);
	cr->move_to  (0.0, 40.0 * (1.0 - audio_signal_ffted[0]) + 5.0 + 0.0);
	for (int i = 0; i < WATERFALL0_XSIZE; i++) {
		cr->line_to(i, 40.0 * (1.0 - audio_signal_ffted[i]) + 5.0 + 0.0);
	}
	cr->stroke();
	cr->restore();

	/* cw pitch marker */
	cr->save();
	cr->set_source_rgba(0.9, 0.9, 0.0, 0.8);
	for(int ii=-1;ii<=+1;ii++) {
		int itone = (cw_pitch + ii*100) / bin_size;
		cr->move_to(itone, 50.0);
		cr->line_to(itone, 40.0);
	}
	cr->stroke();
	cr->restore();

	gettimeofday(&t9, NULL);
	cout_gettimeofday_diff(myid+"             elapsed:  ", t0  , t1);
	cout_gettimeofday_diff(myid+"             interval: ", t1b4, t1);
	cout_gettimeofday_diff(myid+"             duration: ", t1  , t9);
	t1b4 = t1;

	return true;
}

bool Waterfall0::on_timeout() {
	static int icount = 0;

	static struct timeval t1, t1b4, t9;
	gettimeofday(&t1, NULL);
	string myid = "Waterfall0::on_timeout: ";

	cout << myid << "icountw = " << icount++ <<  endl;

	Glib::RefPtr<Gdk::Window> win = get_window();
	if (win) {
		win->invalidate(false);
	} else {
		cout << "Waterfall0::on_timeout: get_window NG" << endl;
		exit(EXIT_FAILURE);
	}

	gettimeofday(&t9, NULL);
	cout_gettimeofday_diff(myid+"          elapsed:  ", t0  , t1);
	cout_gettimeofday_diff(myid+"          interval: ", t1b4, t1);
	cout_gettimeofday_diff(myid+"          duration: ", t1  , t9);
	t1b4 = t1;

	return true;
}

bool Waterfall0::on_button_press_event(GdkEventButton * event) {

	x_press = event->x;
	y_press = event->y;

	std::cout << "Waterfall::on_button_press_event:  x_press = " << x_press
			<< ", y_press = "     << y_press << endl;

	if (y_press <= WATERFALL_YSIZE + WATERFALL_ZSIZE) { /* IC-7410 area */
		if(operating_mode == 3) { /* CW is LSB */
			ic7410_freq_in_hz -= x_press * bin_size - cw_pitch;
		} else if(operating_mode == 7) { /* CW-R is USB */
			ic7410_freq_in_hz += x_press * bin_size - cw_pitch;
		} else {
			;
		}
		set_ic7410_freq(ic7410_freq_in_hz);
	}

//	if (y_press >= WATERFALL_YSIZE/2 + WATERFALL_ZSIZE) { /* Soft66LC4 area */
//		ic7410_freq_in_hz = 7020000 + (x_press - (WATERFALL0_XSIZE/2) ) * bin_size;
//		set_ic7410_freq(ic7410_freq_in_hz);
//	}

	return true;
}
