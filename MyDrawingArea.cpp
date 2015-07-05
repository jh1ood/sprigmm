/*
 * MyDrawingArea.cpp
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#include "MyDrawingArea.h"
#include <iostream>
using namespace std;

MyDrawingArea::MyDrawingArea() {
}

MyDrawingArea::MyDrawingArea(Sound* x) {
	y = x;
	nch = x->get_channels();
	set_size_request(20 + max(nsamples, nfft), 20 + 100 * nch + ntime);
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &MyDrawingArea::on_timeout), 100);
	cout << "MyDrawingArea::MyDrawingArea(): x = " << x << ", nch = " << nch << endl;
}

MyDrawingArea::~MyDrawingArea() {
	cout << "MyDrawingArea::~MyDrawingArea(): count = " << count << ", nch = " << nch << endl;
}

bool MyDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
	cout << "MyDrawingArea::on_draw(): count = " << count++ << ", nch = " << nch << endl;

	y->asound_read();

	for (int i = 0; i < 10; i++) {
		cout << "MyDrawingArea::on_draw(): audio_signal[" << i << "] = " << y->audio_signal[i] << endl;
	}

	cr->save();
	for(int iy=0;iy<nch;iy++) {
		cr->set_source_rgba(0.1, 0.5*iy, 0.5, 1.0);
		cr->rectangle( 10, 10+100*iy,  nsamples,  80);
		cr->fill();
		cr->stroke();
	}
	cr->restore();

	cr->save();
	cr->set_source_rgba(0.9, 0.5*nch, 0.1, 1.0);
	if(nch == 1) {
		for(int ix=0;ix<nsamples;ix++) {
			cr->line_to(10+ix, 10+50 + y->audio_signal[ix]/32768.0*50.0);
		}
	} else if(nch == 2) {
		for(int ix=0;ix<nsamples;ix++) {
			cr->line_to(10+ix, 10+50 + y->audio_signal[2*ix]/32768.0*50.0);
		}
		cr->move_to(10, 10+150 + y->audio_signal[1]/32768.0*50.0);
		for(int ix=0;ix<nsamples;ix++) {
			cr->line_to(10+ix, 10+150 + y->audio_signal[2*ix+1]/32768.0*50.0);
		}

	}
	cr->stroke();
	cr->restore();

	cr->save();
	cr->set_source_rgba(0.1, 0.1, 0.9, 1.0);
	cr->rectangle(10, 10+100*nch, nfft, ntime);
	cr->fill();
	cr->stroke();
	cr->restore();

	return true;
}

bool MyDrawingArea::on_timeout() {
	cout << "MyDrawingArea::on_timeout(): count = " << count << ", nch = " << nch << endl;

	// force our program to redraw
	Glib::RefPtr<Gdk::Window> win = get_window();
	if (win) {
		Gdk::Rectangle r(0, 0, get_allocation().get_width(),
				get_allocation().get_height());
		win->invalidate_rect(r, false);
	}
	return true;
}
