/*
 * DrawingArea0.cpp
 *
 *  Created on: Jun 2, 2015
 *      Author: user1
 */

#include "mydefine.h"
#include "DrawingArea0.h"
#include "Sound.h"
#include "Usbsound.h"
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
extern struct timeval t0;
//extern Sound *mysound[];
extern Usbsound *mysound[];

DrawingArea0::DrawingArea0() {
	string myid ="DrawingArea0::DrawingArea0: ";
	cout << myid << "constructor is called." << endl;

	set_size_request(WATERFALL_XSIZE-WATERFALL0_XSIZE, WATERFALL_YSIZE + WATERFALL_ZSIZE);
	m_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, WATERFALL_XSIZE-WATERFALL0_XSIZE, WATERFALL_YSIZE + WATERFALL_ZSIZE);

//	guint8* p = m_image->get_pixels();

	Glib::signal_timeout().connect(sigc::mem_fun(*this, &DrawingArea0::on_timeout), 150 ); /**************/
}

DrawingArea0::~DrawingArea0() {
}

bool DrawingArea0::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
	static int icountx = 0;
	static struct timeval t1, t1b4, t9;
	gettimeofday(&t1, NULL);
	string myid = "DrawingArea0::on_draw: ";

	cout << myid << "icountx = " << icountx++ << endl;
//	int rowstride = m_image->get_rowstride();

	bool ready = {false};
	ready = mysound[0]->asound_myread();
	if(ready) {
		cout << myid << " ready" << endl;
	} else {
		cout << myid << " not ready" << endl;
	}

	const int w1 = 5;
	const int w2 = (WATERFALL_ZSIZE + WATERFALL_YSIZE - 4 * w1) / 3;
	const int ixstart = 5;

	Gdk::Cairo::set_source_pixbuf(cr, m_image, WATERFALL_XOFFSET, WATERFALL_YOFFSET);
	cr->paint();
	cr->stroke();

	/* box for waveform for IC-7410 */
	cr->save();
	cr->set_source_rgba(0.2, 0.2, 0.8, 1.0);
	cr->rectangle(ixstart, w1, WATERFALL_XSIZE-WATERFALL0_XSIZE, w2);
	cr->fill();
	cr->stroke();
	cr->restore();

	/* box for waveform for Soft66LC4 I signal */
	cr->save();
	cr->set_source_rgba(0.1, 0.5, 0.1, 1.0);
	cr->rectangle(ixstart, 2*w1+w2, WATERFALL_XSIZE-WATERFALL0_XSIZE, w2);
	cr->fill();
	cr->stroke();
	cr->restore();

	/* box for waveform for Soft66LC4 Q signal */
	cr->save();
	cr->set_source_rgba(0.2, 0.4, 0.2, 1.0);
	cr->rectangle(ixstart, 3*w1+2*w2, WATERFALL_XSIZE-WATERFALL0_XSIZE, w2);
	cr->fill();
	cr->stroke();
	cr->restore();

	/* waveform for IC-7410 */
	cr->save();
	cr->set_source_rgba(0.9, 0.9, 0.0, 0.8);
	cr->move_to(ixstart,
			mysound[0]->audio_signal[ixstart] / 16384.0 * 24.0 + w1+w2/2);
	for (int i = ixstart; i < WATERFALL_XSIZE-WATERFALL0_XSIZE; i++) {
		cr->line_to(i,
				mysound[0]->audio_signal[i] / 16384.0 * 24.0 + w1+w2/2);
	}
	cr->stroke();
	cr->restore();

	/* waveform for Soft66LC4 I signal */
	cr->set_source_rgba(0.1, 0.9, 0.0, 0.8);
	cr->move_to(ixstart,
			mysound[1]->audio_signal[ixstart] / 16384.0 * 24.0 + 2*w1+3*w2/2);
	for (int i = ixstart; i < WATERFALL_XSIZE-WATERFALL0_XSIZE; i++) {
		cr->line_to(i,
				mysound[1]->audio_signal[2*i] / 16384.0 * 24.0 + 2*w1+3*w2/2);
	}
	cr->stroke();
	cr->restore();

	/* waveform for Soft66LC4 Q signal */
	cr->set_source_rgba(0.1, 0.9, 0.9, 0.8);
	cr->move_to(ixstart,
			mysound[1]->audio_signal[ixstart] / 16384.0 * 24.0 + 3*w1+5*w2/2);
	for (int i = ixstart; i < WATERFALL_XSIZE-WATERFALL0_XSIZE; i++) {
		cr->line_to(i,
				mysound[1]->audio_signal[2*i+1] / 16384.0 * 24.0 + 3*w1+5*w2/2);
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

bool DrawingArea0::on_timeout() {
	static int icount = 0;

	static struct timeval t1, t1b4, t9;
	gettimeofday(&t1, NULL);
	string myid = "DrawingArea0::on_timeout: ";

	cout << myid << "icountw = " << icount++ <<  endl;

	Glib::RefPtr<Gdk::Window> win = get_window();
	if (win) {
		win->invalidate(false);
	} else {
		cout << "DrawingArea0::on_timeout: get_window NG" << endl;
		exit(EXIT_FAILURE);
	}

	gettimeofday(&t9, NULL);
	cout_gettimeofday_diff(myid+"          elapsed:  ", t0  , t1);
	cout_gettimeofday_diff(myid+"          interval: ", t1b4, t1);
	cout_gettimeofday_diff(myid+"          duration: ", t1  , t9);
	t1b4 = t1;

	return true;
}
