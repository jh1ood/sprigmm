/*
 * MyDrawingAreaS.cpp
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#include "MyDrawingAreaS.h"

#include <iostream>
#include <cmath>
using namespace std;

int colormap_r(double);
int colormap_g(double);
int colormap_b(double);

MyDrawingAreaS::MyDrawingAreaS(Sound* ss) : s {ss} {
	current_b4  = chrono::system_clock::now();
	channels    = s->channels;
	nfft        = s->nfft;
	waveform_x  = s->waveform_x;
	waveform_y  = s->waveform_y;
	spectrum_x  = s->spectrum_x;
	spectrum_y  = s->spectrum_y;
	waterfall_x = s->waterfall_x;
	waterfall_y = s->waterfall_y;
	amax        = s->amax;
	amin        = s->amin;

	size_x = 2*xspacing + max(max(waveform_x, spectrum_x), waterfall_x);
	size_y = 2*yspacing + channels * (waveform_y + yspacing) + spectrum_y + waterfall_y ;
	set_size_request(size_x, size_y);

	Glib::signal_timeout().connect( sigc::mem_fun(*this, &MyDrawingAreaS::on_timeout), s->timer_value );
	cout << "MyDrawingAreaS::MyDrawingAreaS(): signal_timeout().connect, timer_value = " << s->timer_value << endl;

	Glib::signal_timeout().connect( sigc::mem_fun(*this, &MyDrawingAreaS::on_timeout2), s->timer_value2 );
	cout << "MyDrawingAreaS::MyDrawingAreaS(): signal_timeout().connect, timer_value2 = " << s->timer_value2 << endl;

	add_events(	Gdk::BUTTON_PRESS_MASK );

	m_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, waterfall_x, waterfall_y);
	rowstride = m_image->get_rowstride();

	for (int j = 0; j < waterfall_y; j++) {
		p = m_image->get_pixels() + j * rowstride;
		for (int i = 0; i < waterfall_x; i++) {
			*p++ = i%256;
			*p++ =  0;
			*p++ =  0;
		}
	}

}

MyDrawingAreaS::~MyDrawingAreaS() {
	cout << "MyDrawingAreaS::~MyDrawingAreaS() destructor.." << endl;
}

bool MyDrawingAreaS::on_button_press_event(GdkEventButton * event) {

	x_press = event->x;
	y_press = event->y;

	switch (channels) {
	case 1: /* IC-7410 */
		if(RigParams::operating_mode == 3) {        /* CW   is LSB */
			RigParams::frequency_to_set = RigParams::ic7410_frequency - ( (x_press - xspacing) * s->bin_size - RigParams::cw_pitch );
		} else if(RigParams::operating_mode == 7) { /* CW-R is USB */
			RigParams::frequency_to_set = RigParams::ic7410_frequency + ( (x_press - xspacing) * s->bin_size - RigParams::cw_pitch );
		} else {
			return false;
		}
		break;
	case 2: /* Soft66LC4 */
		RigParams::frequency_to_set = RigParams::soft66_frequency + ( (x_press - xspacing) - (waterfall_x / 2) ) * s->bin_size;
		break;
	default:
		return false;
	}

	RigParams::frequency_to_go  = true;
	return true;
}

bool MyDrawingAreaS::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {

	current    = chrono::system_clock::now();
	auto diff  = current - current_b4;
	current_b4 = current;

	cout << "MyDrawingAreaS::on_draw(): count = " << count
			<< " , elapsed time (msec) = " << chrono::duration_cast<std::chrono::milliseconds>(diff).count()
			<< " , start = "     << s->signal_start - s->audio_signal
			<< " , end = "       << s->signal_end   - s->audio_signal
			<< " , end-start = " << s->signal_end   - s->signal_start << endl;


	/* get sound samples, and repeat while there is enough data for fft */

//	s->loop_count = s->asound_read(); /* loop_count = 0 or 1 if timer_value is small enough */

	cout << "loop_count = " << s->loop_count << endl;

	if(s->loop_count) {
	}

	if(s->loop_count) {
		while(s->signal_end - s->signal_start >= nfft*channels) {

			s->asound_fftcopy();

			/* forward FFT block */
			s->signal_start += (int) (s->nfft * s->fft_forward_ratio * s->channels);

			fftw_execute(s->plan);

			/* waterfall shiftdown */
			if(s->loop_count) {
				for (int i = waterfall_y - 1; i > 0 ; i--) {
					p = m_image->get_pixels() + i * rowstride;
					for (int j = 0; j < waterfall_x * 3; j++) {
						*p = *(p - rowstride);
						p++;
					}
				}
			}

			/* log10 and normalize */
			double val  =  0.0;
			p = m_image->get_pixels();

			for(int ix=0;ix<spectrum_x;ix++) {
				int ixx = s->get_index(ix, nfft, spectrum_x);
				if(channels == 1 && RigParams::operating_mode == 3) { /* LSB */
					ixx = (spectrum_x - 1) - ixx;
				}
				val = s->out[ixx][0] * s->out[ixx][0] + s->out[ixx][1] * s->out[ixx][1];
				if (val < pow(10.0, amin)) {
					val = 0.0;
				} else if (val > pow(10.0, amax)) {
					val = 1.0;
				} else {
					val = (log10(val) - amin) / (amax - amin);
				}
				val_line[ix] = val;

				*p++ = colormap_r(val);
				*p++ = colormap_g(val);
				*p++ = colormap_b(val);
			}
		}

		/* shift audio_signal */

		auto p = s->audio_signal;
		auto q = s->signal_start;
		int i=0;
		while(q < s->signal_end) {
			*p++ = *q++;
			i++;
		}

		s->signal_start = s->audio_signal;
		s->signal_end   = p;

	}

	/* fill in the whole area */
	{
		cr->save();
//		color_phase = (count % 360) / 360.0 * 2.0 * 3.1415926535;
		cr->set_source_rgba(0.5+0.1*sin(color_phase), 0.5+0.1*cos(color_phase), 0.5, 1.0);
		cr->rectangle(0, 0, size_x, size_y);
		cr->fill();
		cr->stroke();
		cr->restore();
	}

	/* draw waveform */
	{
		cr->save();
		for(int iy=0;iy<channels;iy++) {
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

			int nexpand = 1;
//			int ixx = s->nfft/2 - nexpand*waveform_x/2;
//			cout << "X: s->nfft/2 = " << s->nfft/2 << " , waveform_x/2 = " << waveform_x/2 << " , ixx = " << ixx << endl;
			for(int ix=0;ix<waveform_x;ix++) {
//				cr->line_to(xspacing+ix, yspacing+(waveform_y+yspacing)*iy+(waveform_y/2) + s->audio_signal[channels*ix+iy]/32768.0*(waveform_y/2));
//				cr->line_to(xspacing+ix, yspacing+(waveform_y+yspacing)*iy+(waveform_y/2) + s->in[ix][iy]/32768.0*(waveform_y/2));
				int ixx = min(max(s->nfft/2 - nexpand*waveform_x/2 + nexpand*ix, 0), s->nfft - 1);
//				ixx = max(ixx, 0);
//				ixx = min(ixx, s->nfft - 1);
//				cout << "X: ixx = " << ixx << " , ix = " << ix << endl;
				cr->line_to(xspacing+ix, yspacing+(waveform_y+yspacing)*iy+(waveform_y/2) + s->in[ixx][iy]/32768.0*(waveform_y/2));
			}
			cr->stroke();
		}
		cr->restore();
	}

	/* spectrum box */
	{
		cr->save();
		cr->set_source_rgba(0.1, 0.1, 0.9, 1.0);
		cr->rectangle(xspacing, yspacing+(waveform_y+yspacing)*channels, spectrum_x, spectrum_y);
		cr->fill();
		cr->stroke();
		cr->restore();
	}

	/* draw spectrum */
	cr->save();
	cr->set_source_rgba(0.9, 0.9, 0.1, 1.0);
	for(int ix=0;ix<spectrum_x;ix++) {
		cr->line_to(xspacing+ix, yspacing+(waveform_y+yspacing)*channels+spectrum_y - val_line[ix]*spectrum_y*0.9);
	}
	cr->stroke();
	cr->restore();

	/* show IC-7410 frequency */
	{
		if(channels == 2) {
			cr->save();
			cr->set_source_rgba(1.0, 1.0, 1.0, 0.2);
			int xpos = spectrum_x/2 + (RigParams::ic7410_frequency - RigParams::soft66_frequency) / s->bin_size;
			cr->rectangle(xspacing+xpos-10, yspacing+(waveform_y+yspacing)*channels, 20, spectrum_y);
			cr->fill();
			cr->stroke();
			cr->restore();
		}
	}

	/* waterfall draw */
	Gdk::Cairo::set_source_pixbuf(cr, m_image, xspacing, yspacing+(waveform_y+yspacing)*channels+spectrum_y);
	{
		cr->save();
		cr->paint();
		cr->stroke();
		cr->restore();
	}

	count++;
	return true;
}

bool MyDrawingAreaS::on_timeout() {

	Glib::RefPtr<Gdk::Window> win = get_window();
	if (win) {
		Gdk::Rectangle rect(0, 0, get_allocation().get_width(), get_allocation().get_height());
		win->invalidate_rect(rect, false);
	}

	return true;
}

bool MyDrawingAreaS::on_timeout2() {

	color_phase = (count2++ % 360) / 360.0 * 2.0 * 3.1415926535;
	cout << "count2 = " << count2 << " , color_phase = " << color_phase << endl;

	s->loop_count = s->asound_read(); /* s->loop_count = 0 or 1 if timer_value is small enough */

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
