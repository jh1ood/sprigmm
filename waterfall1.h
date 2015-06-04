#ifndef GTKMM_EXAMPLE_WATERFALL1_H
#define GTKMM_EXAMPLE_WATERFALL1_H

#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>
#include "Sound.h"

class Waterfall1:public Gtk::DrawingArea {
  public:
    Waterfall1();
    virtual ~ Waterfall1();

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

	int    nfft;
	double bin_size;
	signed short audio_signal_max;
	double dc0;
	double dc1;
	double amax, amin;

	double *fft_window;
public:
	double *audio_signal_ffted;
private:
	fftw_complex *in; /* for Soft66LC4 only */
	fftw_complex *out;
	fftw_plan     plan;

};

#endif				// GTKMM_EXAMPLE_WATERFALL_H
