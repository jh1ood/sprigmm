/*
 * MyDrawingArea.cpp
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#include "MyDrawingArea.h"
#include <iostream>
using namespace std;

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

MyDrawingArea::MyDrawingArea() {
}

MyDrawingArea::MyDrawingArea(Sound* x) {
	y = x;
	nch = x->get_channels();
	set_size_request(20 + max(max(nsamples, nfft), waterfall_x), 20 + 100 * nch + ntime + waterfall_y);
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &MyDrawingArea::on_timeout), 100);

	m_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, waterfall_x, waterfall_y);
	guint8* p = m_image->get_pixels();
	cout << "pointer = " << m_image->get_pixels() << endl;
	if (p==nullptr) cout << "ERROR \n";
	cout << "p = " << p << endl;
	for (int j = 0; j < waterfall_y; j++) {
		for (int i = 0; i < waterfall_x; i++) {
			*p++ = 64*(nch-1);
			*p++ = (int) ( 255.0 * (double)i / (double) waterfall_x );
			*p++ = 0;
		}
	}
	cout << "p = " << p << endl;

	cout << "MyDrawingArea::MyDrawingArea(): x = " << x << ", nch = " << nch << endl;
}

MyDrawingArea::~MyDrawingArea() {
	cout << "MyDrawingArea::~MyDrawingArea(): count = " << count << ", nch = " << nch << endl;
}

bool MyDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
	cout << "MyDrawingArea::on_draw(): count = " << count++ << ", nch = " << nch << endl;

	y->asound_read();

	for (int i = 0; i < 5; i++) {
		cout << "MyDrawingArea::on_draw(): audio_signal[" << i << "] = " << y->audio_signal[i] << endl;
	}

	y->asound_fftcopy();

	/* audio signal FFT */
	fftw_execute(y->plan);

	/* log10 and normalize */
	double amax = 14.0;
	double amin =  7.0;
	for (int i = 0; i < nfft; i++) {
		double val;
		val = y->out[i][0] * y->out[i][0] + y->out[i][1] * y->out[i][1];
		if (val < pow(10.0, amin)) {
			y->audio_signal_ffted[i] = 0.0;
		} else if (val > pow(10.0, amax)) {
			y->audio_signal_ffted[i] = 1.0;
		} else {
			y->audio_signal_ffted[i] = (log10(val) - amin) / (amax - amin);
		}
		if(i < 5 || i> nfft-6) cout << "i = " << i << ", nfft = " << nfft << ", audio_signal_ffted = " << y->audio_signal_ffted[i] << endl;
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
	for(int iy=0;iy<nch;iy++) {
		cr->move_to(10, 10+100*iy+50 + y->audio_signal[iy]/32768.0*50.0);
		for(int ix=0;ix<nsamples;ix++) {
			cr->line_to(10+ix, 10+100*iy+50 + y->audio_signal[nch*ix+iy]/32768.0*50.0);
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

	cr->save();
	cr->set_source_rgba(0.9, 0.9, 0.1, 1.0);
	for(int ix=0;ix<nsamples;ix++) {
		cr->line_to(10+ix, 10+100*nch+100 - y->audio_signal_ffted[ix]*80.0);
	}
	cr->stroke();
	cr->restore();

	guint8 *p = nullptr;
	int rowstride = m_image->get_rowstride();

	for (int i = waterfall_y - 1; i > 0 ; i--) {
			p = m_image->get_pixels() + i * rowstride;
			for (int j = 0; j < waterfall_x * 3; j++) {
				*p = *(p - waterfall_x * 3);
				p++;
			}
		}

	p = m_image->get_pixels();
	cout << "p = " << p << endl;
	for (int i = 0; i < waterfall_x; i++) {
			double tmp = y->audio_signal_ffted[i];
			*p++ = colormap_r(tmp);
			*p++ = colormap_g(tmp);
			*p++ = colormap_b(tmp);
	}

	Gdk::Cairo::set_source_pixbuf(cr, m_image, 10, 10+100*nch+ntime);
	cr->paint();
	cr->stroke();

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
