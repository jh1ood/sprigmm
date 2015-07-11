/*
 * MyDrawingArea.cpp
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#include "MyDrawingArea.h"
#include <iostream>
using namespace std;

int colormap_r(double);
int colormap_g(double);
int colormap_b(double);

MyDrawingArea::MyDrawingArea(Sound* x) {
	y = x;
	nch         = x->get_channels   ();
	nfft        = x->get_nfft       ();
	spectrum_x  = x->get_spectrum_x ();
	spectrum_y  = x->get_spectrum_y ();
	waterfall_x = x->get_waterfall_x();
	waterfall_y = x->get_waterfall_y();

	set_size_request(2*xspacing + max(max(waveform_x, spectrum_x), waterfall_x)
				   , 2*yspacing + nch * (waveform_y + yspacing) + spectrum_y + waterfall_y);
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &MyDrawingArea::on_timeout), 100);

	m_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, waterfall_x, waterfall_y);
	rowstride = m_image->get_rowstride();
	p = m_image->get_pixels();
	for (int j = 0; j < waterfall_y; j++) {
		for (int i = 0; i < waterfall_x; i++) {
			*p++ = 153;
			*p++ = 214;
			*p++ = 173;
		}
	}

	cout << "MyDrawingArea::MyDrawingArea(): x = " << x << ", nch = " << nch << endl;
}

MyDrawingArea::~MyDrawingArea() {
	cout << "MyDrawingArea::~MyDrawingArea(): count = " << count << ", nch = " << nch << endl;
}

bool MyDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
	cout << "MyDrawingArea::on_draw(): count = " << count++ << ", nch = " << nch << endl;

	y->asound_read();
	y->asound_fftcopy();
	fftw_execute(y->plan);

	/* waveform */
	cr->save();
	for(int iy=0;iy<nch;iy++) {
		/* draw a box */
		cr->set_source_rgba(0.1, 0.1, 0.5, 1.0);
		cr->rectangle(xspacing, yspacing+(waveform_y+yspacing)*iy,  waveform_x,  waveform_y);
		cr->fill();
		cr->stroke();

		/* draw a baseline */
		cr->set_source_rgba(1.0, 1.0, 1.0, 0.5);
		cr->move_to(xspacing+waveform_x, yspacing+(waveform_y+yspacing)*iy+(waveform_y/2));
		cr->line_to(xspacing           , yspacing+(waveform_y+yspacing)*iy+(waveform_y/2));
		cr->stroke();

		/* draw a waveform */
		cr->set_source_rgba(0.4, 0.9, 0.1, 1.0);
		for(int ix=0;ix<waveform_x;ix++) {
			cr->line_to(xspacing+ix, yspacing+(waveform_y+yspacing)*iy+(waveform_y/2) + y->audio_signal[nch*ix+iy]/32768.0*(waveform_y/2));
		}
		cr->stroke();
	}
	cr->restore();

	/* waterfall shiftdown */
	for (int i = waterfall_y - 1; i > 0 ; i--) {
			p = m_image->get_pixels() + i * rowstride;
			for (int j = 0; j < waterfall_x * 3; j++) {
				*p = *(p - waterfall_x * 3);
				p++;
			}
		}

	/* spectrum box */
	cr->save();
	cr->set_source_rgba(0.1, 0.1, 0.9, 1.0);
	cr->rectangle(xspacing, yspacing+(waveform_y+yspacing)*nch, spectrum_x, spectrum_y);
	cr->fill();
	cr->stroke();

	/* log10 and normalize */
	double amax = 14.0;
	double amin =  7.0;
	double val;

	/* spectrum draw, and waterfall put one line */
	cr->set_source_rgba(0.9, 0.9, 0.1, 1.0);
	p = m_image->get_pixels();

	int ixx;
	for(int ix=0;ix<spectrum_x;ix++) {
		ixx = y->get_index(ix, nfft, spectrum_x);
		val = y->out[ixx][0] * y->out[ixx][0] + y->out[ixx][1] * y->out[ixx][1];
		if (val < pow(10.0, amin)) {
			val = 0.0;
		} else if (val > pow(10.0, amax)) {
			val = 1.0;
		} else {
			val = (log10(val) - amin) / (amax - amin);
		}
		*p++ = colormap_r(val);
		*p++ = colormap_g(val);
		*p++ = colormap_b(val);
		cr->line_to(xspacing+ix, yspacing+(waveform_y+yspacing)*nch+spectrum_y - val*spectrum_y);
	}
	cr->stroke();
	cr->restore();

	/* waterfall draw */
	Gdk::Cairo::set_source_pixbuf(cr, m_image, xspacing, yspacing+(waveform_y+yspacing)*nch+spectrum_y);
	cr->save();
	cr->paint();
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
