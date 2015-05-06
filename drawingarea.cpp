#include "mydefine.h"
#include "drawingarea.h"
#include "Sound.h"
#include <cairomm/context.h>
#include <gdkmm/general.h>	// set_source_pixbuf()
#include <glibmm/fileutils.h>
#include <cmath>
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <cairo.h>
#include <glibmm/main.h>
using namespace std;
extern Sound *mysound1;
extern Sound *mysound2;

DrawingArea::DrawingArea() {
	std::cout << "DrawingArea constructor is called." << std::endl;
	set_size_request(1200, 240); /* width is dummy, determined by radiobuttons */

	Glib::signal_timeout().connect(
			sigc::mem_fun(*this, &DrawingArea::on_timeout), 50);

#ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
	//Connect the signal handler if it isn't already a virtual method override:
	signal_draw().connect(sigc::mem_fun(*this, &DrawingArea::on_draw),
			false);
#endif				//GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

	add_events(
			Gdk::BUTTON_PRESS_MASK | Gdk::SCROLL_MASK
					| Gdk::SMOOTH_SCROLL_MASK);

}

DrawingArea::~DrawingArea() {
	std::cout << "DrawingArea destructor is called." << std::endl;
}

bool DrawingArea::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
	static int icountx = 0;
	cout << "on_draw: icountx = " << icountx++ << endl;

	myclock(); /* to get inf from IC-7410 */
	cout << "on_draw: returned from myclock() \n";

	Gtk::Allocation allocation = get_allocation();
	const int width = allocation.get_width();
//	const int height = allocation.get_height(); /* eg. 50 */
	const int rectangle_width = width;
	const int rectangle_height = 50; /* eg. 50 */

// rectangle box for frequency display, S-meter, waveform
	cr->save();
	cr->set_source_rgba(0.0, 0.6, 0.2, 1.0);
	cr->rectangle(0, 0, WATERFALL_XSIZE, rectangle_height);
	cr->fill();
	cr->stroke();
	cr->restore();

	cr->save();
	cr->set_source_rgba(0.2, 0.6, 0.6, 1.0);
	cr->rectangle(0, 60, WATERFALL_XSIZE, rectangle_height);
	cr->fill();
	cr->stroke();
	cr->restore();

	cr->save();
	cr->set_source_rgba(0.2, 0.6, 0.6, 1.0);
	cr->rectangle(0, 120, WATERFALL_XSIZE, rectangle_height);
	cr->fill();
	cr->stroke();
	cr->restore();

	cr->save();
	cr->set_source_rgba(0.2, 0.1, 0.6, 1.0);
	cr->rectangle(0, 180, WATERFALL_XSIZE, rectangle_height);
	cr->fill();
	cr->stroke();
	cr->restore();

// frequency display
	cr->save();
	draw_text(cr, rectangle_width, rectangle_height);
//  cr->set_source_rgba(0.1, 0.7, 0.3,1.0);
//  for(int i=0;i<10;i++) {
//        cr->move_to(10.0+31.0*i, 3.0);
//        cr->line_to(10.0+31.0*i,46.0);
//      cr->stroke();
//  }
	cr->restore();

// S meter
	double s_frac = 2.0 * (double) s_meter / 255.0;
	if (s_frac > 1.0) {
		s_frac = 1.0;
	}
	cr->save();
	cr->set_line_width(2);
	cr->set_source_rgba(colormap_r(s_frac) / 255.0, colormap_g(s_frac) / 255.0,
			colormap_b(s_frac), 1.0);	// partially translucent
	cr->rectangle(400, 5, 200 * s_frac + 20, 40);
	cr->fill_preserve();
	cr->set_source_rgb(0.0, 0.0, 0.0);
	cr->stroke();
	cr->restore();

// Waveform for IC-7410
		cr->save();
		cr->set_source_rgba(0.9, 0.9, 0.2, 1.0);
		cr->move_to(0.0,
				mysound1->audio_signal[0] / 16384.0 * 24.0 + 25.0 + 60.0);
		for (int i = 0; i < WATERFALL_XSIZE; i++) {
			cr->line_to(i,
					mysound1->audio_signal[i] / 16384.0 * 24.0 + 25.0 + 60.0);
		}
		cr->stroke();
		cr->restore();

// Waveform for Soft66LC4
		cr->save();
		cr->set_source_rgba(0.7, 0.9, 0.9, 1.0);
		cr->move_to(0.0, mysound2->audio_signal[0] / 16384.0 * 24.0 + 25.0 + 120.0);
		for (int i = 0; i < WATERFALL_XSIZE; i++) {
			cr->line_to(i, mysound2->audio_signal[2 * i] / 16384.0 * 24.0 + 25.0 + 120.0);
		}
		cr->stroke();

		cr->set_source_rgba(0.9, 0.2, 0.9, 1.0);
		cr->move_to(0.0, mysound2->audio_signal[0] / 16384.0 * 24.0 + 25.0 + 120.0);
		for (int i = 0; i < WATERFALL_XSIZE; i++) {
			cr->line_to(i,
					mysound2->audio_signal[2 * i + 1] / 16384.0 * 24.0 + 25.0 + 120.0);
		}
		cr->stroke();
		cr->restore();

// Spectrum for IC-7410
	cr->save();
	cr->set_source_rgba(0.2, 0.9, 0.9, 1.0);
	cr->move_to(0.0,
			40.0 * (1.0 - mysound1->audio_signal_ffted[0]) + 5.0
					+ 180.0);
	for (int i = 0; i < WATERFALL_XSIZE; i++) {
		cr->line_to(i, 40.0 * (1.0 - mysound1->audio_signal_ffted[i]) + 5.0 + 180.0);
	}
	cr->stroke();
	cr->restore();

// Spectrum for Soft66LC4
		cr->save();
		cr->set_source_rgba(0.9, 0.9, 0.2, 1.0);
		cr->move_to(0.0,
				40.0 * (1.0 - mysound2->audio_signal_ffted[mysound2->nfft - (WINDOW_XSIZE / 2)]) + 5.0
						+ 180.0);
		for (int i = 0; i < WATERFALL_XSIZE; i++) {
			int j = ((mysound2->nfft - (WATERFALL_XSIZE / 2)) + i) % mysound2->nfft;
			cr->line_to(i, 40.0 * (1.0 - mysound2->audio_signal_ffted[j]) + 5.0 + 180.0);
		}
		cr->stroke();
		cr->restore();

// frequency marker
	cr->save();
	cr->set_source_rgba(0.0, 0.0, 0.0, 1.0);
	for (int i = 400; i <= 800; i += 200) {
		cr->move_to( (i / mysound1->bin_size), 230.0);
		cr->line_to( (i / mysound1->bin_size), 240.0);
		cr->move_to( (i / mysound1->bin_size), 230.0);
		cr->line_to( (i / mysound1->bin_size), 240.0);
	}
	cr->stroke();
	cr->restore();

	return true;
}

bool DrawingArea::on_timeout() {
	static int icountw = 0, icountv = 0;
	cout << "on_timeout: icountw = " << icountw++ << endl;
	if (flag_togo1 == 0) {
		return true;
	}
	flag_togo1 = 0;
	cout << "on_timeout: icountv = " << icountv++ << endl;

	// force our program to redraw the entire clock.
	Glib::RefPtr<Gdk::Window> win = get_window();
	if (win) {
		Gdk::Rectangle r(0, 0, get_allocation().get_width(),
				get_allocation().get_height());
		win->invalidate_rect(r, false);
	}
	return true;
}

void DrawingArea::draw_text(const Cairo::RefPtr<Cairo::Context> &cr,
		int rectangle_width, int rectangle_height) {
	Pango::FontDescription font;

	font.set_family("Monospace");
	font.set_weight(Pango::WEIGHT_BOLD);
	font.set_size(40 * 1000); /* unit = 1/1000 point */
	int text_width;
	int text_height;

	char string[128];
	sprintf(string, "%9.3fkHz                                   %9.3fkHz",
			(double) ifreq_in_hz / 1000.0, (double) jfreq_in_hz / 1000.0);
	Glib::RefPtr<Pango::Layout> layout = create_pango_layout(string);

	layout->set_font_description(font);
	layout->get_pixel_size(text_width, text_height);
	cr->move_to(10, (rectangle_height - text_height) / 2);
	cr->set_source_rgb(1.0, 1.0, 1.0);
	layout->show_in_cairo_context(cr);
	cr->stroke();

}

bool DrawingArea::on_scroll_event(GdkEventScroll * event) {
	double x, dy;
	x = event->x;
	dy = event->delta_y;
	int digit_pos;
	digit_pos = (x - 10.0) / 31.0;
	cout << "digit_pos = " << digit_pos << endl;

	int frequency_delta = 0.0;
	if (digit_pos >= 0 && digit_pos <= 4) {
		frequency_delta = pow(10.0, 7 - digit_pos);
	} else if (digit_pos >= 6 && digit_pos <= 8) {
		frequency_delta = pow(10.0, 8 - digit_pos);
	}

	if (dy > 0) {
		frequency_delta = -frequency_delta;
	}

	int ifreq_in_hz_new = ifreq_in_hz + frequency_delta;
	if (ifreq_in_hz_new > 0 && ifreq_in_hz_new < 60000000) {
		ifreq_in_hz = ifreq_in_hz_new;
	}
	set_freq(ifreq_in_hz);

	return true;
}
