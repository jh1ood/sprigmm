#ifndef GTKMM_EXAMPLE_WATERFALL_H
#define GTKMM_EXAMPLE_WATERFALL_H

#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>
#include "Sound.h"

class Waterfall:public Gtk::DrawingArea {
  public:
    Waterfall();
    virtual ~ Waterfall();

  protected:
  public:
    //Override default signal handler:
    virtual bool on_draw(const Cairo::RefPtr < Cairo::Context > &cr);
    virtual bool on_button_press_event(GdkEventButton * event);
    bool on_timeout();
    Glib::RefPtr < Gdk::Pixbuf > m_image;

  private:
    void draw_text(const Cairo::RefPtr < Cairo::Context > &cr,
		   int rectangle_width, int rectangle_height);
    double x_press;
    double y_press;

	int    nfft[2];
	double bin_size[2];
	signed short audio_signal_max[2];
	double dc0[2];
	double dc1[2];
	double amax[2], amin[2];

	double *fft_window[2];
public:
	double *audio_signal_ffted[2];
private:
	double *in_real;  /* for IC-7410 only */
	fftw_complex *in; /* for Soft66LC4 only */
	fftw_complex *out[2];
	fftw_plan     plan[2];

};

#endif				// GTKMM_EXAMPLE_WATERFALL_H
