#include "mydefine.h"
#include "waterfall.h"
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
using namespace std;

void set_freq(long int ifreq_in_hz);
int colormap_r(double);
int colormap_g(double);
int colormap_b(double);

Waterfall::Waterfall()
{
    std::cout << "Waterfall constructor is called." << std::endl;
    set_size_request(WATERFALL_XSIZE + WATERFALL_XOFFSET,
		     WATERFALL_YSIZE + WATERFALL_YOFFSET);
    m_image =
	Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, WATERFALL_XSIZE,
			    WATERFALL_YSIZE);

    guint8 *p;
    p = m_image->get_pixels();

    for (int j = 0; j < WATERFALL_YSIZE; j++) {
	for (int i = 0; i < WATERFALL_XSIZE; i++) {
	    *p++ = i % 256;
	    *p++ = j % 256;
	    *p++ = 0;
	}
    }

    Glib::signal_timeout().connect(sigc::
				   mem_fun(*this, &Waterfall::on_timeout),
				   50);

#ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
    //Connect the signal handler if it isn't already a virtual method override:
    signal_draw().connect(sigc::mem_fun(*this, &Waterfall::on_draw),
			  false);
#endif				//GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

    add_events(Gdk::BUTTON_PRESS_MASK | Gdk::
	       SCROLL_MASK | Gdk::SMOOTH_SCROLL_MASK);

}

Waterfall::~Waterfall()
{
    std::cout << "Waterfall destructor is called." << std::endl;
}

bool Waterfall::on_draw(const Cairo::RefPtr < Cairo::Context > &cr)
{
    static int icountx = 0;
    cout << "Waterfall::on_draw: icountx = " << icountx++ << endl;

    guint8 *p;
    int rowstride = m_image->get_rowstride();

// shift down pixbuf
    for (int i = WATERFALL_YSIZE - 1; i > 0; i--) {
	p = m_image->get_pixels() + i * rowstride;
	for (int j = 0; j < WATERFALL_XSIZE * 3; j++) {
	    *p = *(p - WATERFALL_XSIZE * 3);
	    p++;
	}
    }

// write into the top line
    p = m_image->get_pixels();
    for (int i = 0; i < WATERFALL_XSIZE; i++) {
	int j = ((NFFT - (WATERFALL_XSIZE / 2)) + i) % NFFT;
	double tmp = audio_signal_ffted[j];
	if (i == WATERFALL_XSIZE / 2)
	    tmp = 1.0;
	*p++ = colormap_r(tmp);
	*p++ = colormap_g(tmp);
	*p++ = colormap_b(tmp);
    }

    Gdk::Cairo::set_source_pixbuf(cr, m_image, WATERFALL_XOFFSET,
				  WATERFALL_YOFFSET);
    cr->paint();
    cr->stroke();

    return true;
}

bool Waterfall::on_timeout()
{
    static int icountw = 0, icountv = 0;
    cout << "Waterfall::on_timeout: icountw = " << icountw++ << endl;
    if (flag_togo2 == 0) {
	return true;
    }
    flag_togo2 = 0;
    cout << "Waterfall::on_timeout: icountv = " << icountv++ << endl;

    // force our program to redraw the entire clock.
    Glib::RefPtr < Gdk::Window > win = get_window();
    if (win) {
	Gdk::Rectangle r(0, 0, get_allocation().get_width(),
			 get_allocation().get_height());
	win->invalidate_rect(r, false);
    }
    return true;
}

void Waterfall::draw_text(const Cairo::RefPtr < Cairo::Context > &cr,
			  int rectangle_width, int rectangle_height)
{
    Pango::FontDescription font;

    font.set_family("Monospace");
    font.set_weight(Pango::WEIGHT_BOLD);
    font.set_size(40 * 1000);	/* unit = 1/1000 point */

    char string[128];
    sprintf(string, "%9.3f", (double) ifreq_in_hz / 1000.0);
    Glib::RefPtr < Pango::Layout > layout = create_pango_layout(string);

    layout->set_font_description(font);

    int text_width;
    int text_height;

    layout->get_pixel_size(text_width, text_height);
    cr->move_to(10, (rectangle_height - text_height) / 2);
    cr->set_source_rgb(1.0, 1.0, 1.0);
    layout->show_in_cairo_context(cr);
    cr->stroke();
}

bool Waterfall::on_scroll_event(GdkEventScroll * event)
{
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

bool Waterfall::on_button_press_event(GdkEventButton * event)
{
    std::cout << "Watefall::on_button_press_event:  " << event->x << " : "
	<< event->y << std::endl;
    x_press = event->x;
    y_press = event->y;
    int pressed_freq = x_press * bin_size;

    double freq_delta = pressed_freq - cw_pitch;
    if (operating_mode == 0x03 || operating_mode == 0x00) {	/* CW or LSB */
	ifreq_in_hz -= freq_delta;
    }
    if (operating_mode == 0x07 || operating_mode == 0x01) {	/* CW-R or USB */
	ifreq_in_hz += freq_delta;
    }
    set_freq(ifreq_in_hz);

    std::cout << "Waterfall::on_button_press_event:  x_press = " << x_press
	<< ", pressed_freq = " << pressed_freq
	<< ", cw_pitch = " << cw_pitch
	<< ", freq_delta =" << freq_delta
	<< ", new_freq = " << ifreq_in_hz << std::endl;


    return true;
}
