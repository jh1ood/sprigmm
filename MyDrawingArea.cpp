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

MyDrawingArea::MyDrawingArea(Sound* ss) : s {ss} {
	nch         = s->channels;
	nfft        = s->nfft;
	spectrum_x  = s->spectrum_x;
	spectrum_y  = s->spectrum_y;
	waterfall_x = s->waterfall_x;
	waterfall_y = s->waterfall_y;

	set_size_request(2*xspacing + max(max(waveform_x, spectrum_x), waterfall_x)
			, 2*yspacing + nch * (waveform_y + yspacing) + spectrum_y + waterfall_y);
	Glib::signal_timeout().connect( sigc::mem_fun(*this, &MyDrawingArea::on_timeout), s->timervalue );
	add_events(	Gdk::BUTTON_PRESS_MASK );

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

	cout << "MyDrawingArea::MyDrawingArea(): s = " << s << ", nch = " << nch << endl;
}

MyDrawingArea::~MyDrawingArea() {
	cout << "MyDrawingArea::~MyDrawingArea(): count = " << count << ", nch = " << nch << endl;
}

bool MyDrawingArea::on_button_press_event(GdkEventButton * event) {

	x_press = event->x;
	y_press = event->y;

//	RigParams::frequency_to_set = s;
//	RigParams::frequency_to_go  = true;

	int freq;
	switch (nch) {
	case 1: /* IC-7410 */
//		if(s->operating_mode == 3) { /* CW is LSB */
//			freq = AlsaParams::ic7410_frequency - ( (x_press - xspacing) * s->bin_size - s->cw_pitch );
//		} else if(s->operating_mode == 7) { /* CW-R is USB */
//			freq = AlsaParams::ic7410_frequency + ( (x_press - xspacing) * s->bin_size - s->cw_pitch );
//		} else {
//			;
//		}
		break;
	case 2: /* Soft66LC4 */
		RigParams::frequency_to_set = RigParams::soft66_frequency + ( (x_press - xspacing) - (waterfall_x / 2) ) * s->bin_size;
		RigParams::frequency_to_go  = true;
		break;
	default:
		return false;
	}

	return true;
}

bool MyDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
	cout << "MyDrawingArea::on_draw(): count = " << count++ << ", nch = " << nch << endl;

	/* get sound, and fft */
	s->asound_read();
	s->asound_fftcopy();
	fftw_execute(s->plan);

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
			cr->line_to(xspacing+ix, yspacing+(waveform_y+yspacing)*iy+(waveform_y/2) + s->audio_signal[nch*ix+iy]/32768.0*(waveform_y/2));
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
	double val  =  0.0;
	int ixx = 0;

	for(int ix=0;ix<spectrum_x;ix++) {
		ixx = s->get_index(ix, nfft, spectrum_x);
		val = s->out[ixx][0] * s->out[ixx][0] + s->out[ixx][1] * s->out[ixx][1];
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
		cr->line_to(xspacing+ix, yspacing+(waveform_y+yspacing)*nch+spectrum_y - val*spectrum_y*0.9);
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

	Glib::RefPtr<Gdk::Window> win = get_window();
	if (win) {
		Gdk::Rectangle rrr(0, 0, get_allocation().get_width(), get_allocation().get_height());
		win->invalidate_rect(rrr, false);
	}
	cout << "MyDrawingArea ::on_timeout: win = " << win << ", width = " << get_allocation().get_width()
			<< ", height = " << get_allocation().get_height() << endl;
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
