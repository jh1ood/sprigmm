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
	nch         = s->get_channels   ();
	nfft        = s->get_nfft       ();
	spectrum_x  = s->get_spectrum_x ();
	spectrum_y  = s->get_spectrum_y ();
	waterfall_x = s->get_waterfall_x();
	waterfall_y = s->get_waterfall_y();

	set_size_request(2*xspacing + max(max(waveform_x, spectrum_x), waterfall_x)
			, 2*yspacing + nch * (waveform_y + yspacing) + spectrum_y + waterfall_y);
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &MyDrawingArea::on_timeout), s->get_timervalue());
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

	int freq;
	switch (nch) {
	case 1: /* IC-7410 */
		if(s->operating_mode == 3) { /* CW is LSB */
			freq = AlsaParams::ic7410_frequency - ( (x_press - xspacing) * s->bin_size - s->cw_pitch );
		} else if(s->operating_mode == 7) { /* CW-R is USB */
			freq = AlsaParams::ic7410_frequency + ( (x_press - xspacing) * s->bin_size - s->cw_pitch );
		} else {
			;
		}
		break;
	case 2: /* Soft66LC4 area */
		freq = AlsaParams::soft66_frequency + ( (x_press - xspacing) - (waterfall_x / 2) ) * s->bin_size;
		break;
	default:
		return false;
	}
	Sound::set_ic7410_frequency(freq);

	std::cout << "button_press_event: nch = " << nch << ", x_press = " << x_press
			<< ", waterfall_x = " << waterfall_x
			<< ", y_press = "     << y_press
			<< ", freq = " << freq << ", bin_size = " << s->bin_size << endl;

	return true;
}

//bool MyDrawingArea::on_scroll_event(GdkEventScroll * event) {
//	double x, dy;
//	x = event->x;
//	dy = event->delta_y;
//	int digit_pos;
//	digit_pos = (x - 10.0) / 31.0;
//	cout << "digit_pos = " << digit_pos << endl;
//
//	int frequency_delta = 0.0;
//	if (digit_pos >= 0 && digit_pos <= 4) {
//		frequency_delta = pow(10.0, 7 - digit_pos);
//	} else if (digit_pos >= 6 && digit_pos <= 8) {
//		frequency_delta = pow(10.0, 8 - digit_pos);
//	}
//
//	if (dy > 0) {
//		frequency_delta = -frequency_delta;
//	}
//
//	int ifreq_in_hz_new = ic7410_frequency + frequency_delta;
//	if (ifreq_in_hz_new > 0 && ifreq_in_hz_new < 60000000) {
//		ic7410_frequency = ifreq_in_hz_new;
//	}
//	set_ic7410_freq(ic7410_frequency);
//
//	return true;
//}

bool MyDrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
	cout << "MyDrawingArea::on_draw(): count = " << count++ << ", nch = " << nch << endl;

	/* get sound, and fft */
	s->asound_read();
	s->asound_fftcopy();
	fftw_execute(s->plan);

	/* check if freq is modified via waterfall */
	if(s->ic7410_changed) {
		s->ic7410_changed = false;
		static unsigned char command1[7] =
		{ 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd };
		long int ifreq_wrk;
		int idigit[8];

		ifreq_wrk = s->ic7410_frequency;

		for (int i = 0; i < 8; i++) {
			idigit[i] = ifreq_wrk % 10;
			ifreq_wrk /= 10;
		}
		command1[1] = 16 * idigit[1] + idigit[0];
		command1[2] = 16 * idigit[3] + idigit[2];
		command1[3] = 16 * idigit[5] + idigit[4];
		command1[4] = 16 * idigit[7] + idigit[6];
		s->send_command(command1);
		s->receive_fb();
	}

	int freq1 = s->get_frequency();
	int freq2 = s->get_other_frequency();
	cout << "freq1 = " << freq1 << ", freq2 = " << freq2 << endl;
	cout << "ic7410_frequency = " << s->ic7410_frequency << ", soft66_frequency = " << s->soft66_frequency << endl;

	/* frequency display */
	cr->save();
	cr->set_source_rgba(0.1, 0.1, 0.5, 1.0);
	Pango::FontDescription font;
	font.set_family("Monospace");
	font.set_weight(Pango::WEIGHT_BOLD);
	font.set_size(40 * 1000); /* unit = 1/1000 point */
	int text_width;
	int text_height;

	char string[128];
	sprintf(string, "%9.3fkHz  %9.3fkHz", (double)freq1 / 1000.0, (double)freq2 / 1000.0);
	Glib::RefPtr<Pango::Layout> layout = create_pango_layout(string);

	layout->set_font_description(font);
	layout->get_pixel_size(text_width, text_height);
	cr->move_to(xspacing+xoffset, yspacing+waveform_y/2 - text_height / 2);
	layout->show_in_cairo_context(cr);
	cr->stroke();

	cr->restore();

	/* S meter */
	//	if(s->get_smeter()) {
	//		double s_frac = 2.0 * (double) s_meter / 255.0;
	//		if (s_frac > 1.0) {
	//			s_frac = 1.0;
	//		}
	//		cr->save();
	//		cr->set_line_width(2);
	//		cr->set_source_rgba(colormap_r(s_frac) / 255.0, colormap_g(s_frac) / 255.0,
	//				colormap_b(s_frac), 1.0);	// partially translucent
	//		cr->rectangle(1250, 5, count%256, waveform_y);
	//		cr->fill_preserve();
	//		cr->set_source_rgb(0.0, 0.0, 0.0);
	//		cr->stroke();
	//		cr->restore();
	//	}

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

	/* show CW pitch for IC-7410 waterfall */
	if(nch == 1) {
		cr->save();
		cr->set_source_rgba(1.0, 1.0, 1.0, 0.8);
		cr->rectangle(xspacing + s->cw_pitch / s->bin_size - 1, yspacing+(waveform_y+yspacing)*nch, 2, spectrum_y);
		cr->fill();
		cr->stroke();
	}

	/* show IC-7410 VFO on Soft66LC4 waterfall */
	if(nch == 2) {
		cr->save();
		cr->set_source_rgba(0.9, 0.9, 0.9, 0.4);
		cr->rectangle(xspacing + spectrum_x/2 + (AlsaParams::ic7410_frequency-AlsaParams::soft66_frequency)/ s->bin_size - 15, yspacing+(waveform_y+yspacing)*nch, 30, spectrum_y);
		cr->fill();
		cr->stroke();
	}

	/* spectrum draw, and waterfall put one line */
	cr->set_source_rgba(0.9, 0.9, 0.1, 1.0);
	p = m_image->get_pixels();

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
