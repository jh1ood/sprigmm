#include <cairomm/context.h>
#include <cmath>
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <cairo.h>
#include <glibmm/main.h>
#include <asoundlib.h>
#include <fftw3.h>
#include "drawingarea.h"
using namespace std;

#define DEBUG
#define NO_MARKER
#define NFFT                    4096
#define WINDOW_XSIZE            1320
#define WINDOW_YSIZE             500
#define AREA1_XSIZE               99
#define AREA1_YSIZE               50
#define WATERFALL_XSIZE          512
#define WATERFALL_YSIZE          768
#define WAVEFORM_LEN             128
#define BAUDRATE                B19200
#define TIMEOUT_VALUE           100
#define END_OF_COMMAND          0xfd

extern int fd;
extern unsigned int rate;
extern unsigned int channels;	/* count of channels */
extern int byte_per_sample;	/* 16 bit format */
extern unsigned int buffer_time;	/* ring buffer length in us */
extern unsigned int period_time;	/* period time in us */
extern int resample;	/* disable resample */
extern int period_event;	/* produce poll event after each period */
extern double audio_signal[NFFT];
extern double audio_signal_ffted[NFFT];
extern double fft_window[NFFT];
extern int cw_pitch;
extern int iwater;
extern int nsamples;
extern double bin_size, waterfall_scale_x;
extern double amax, amin;
extern long int ifreq_in_hz;
extern int s_meter;
extern int operating_mode;	/* CW=03, CW-REV=07, LSB=00, USB=01 */
extern int dsp_filter;		/* FIL1=01, FIL2=02, FIL3=03 */
extern snd_pcm_sframes_t buffer_size;
extern snd_pcm_sframes_t period_size;
extern snd_pcm_t *handle;
extern snd_pcm_hw_params_t *hwparams;
extern snd_pcm_sw_params_t *swparams;
extern int flag_togo;

void set_freq (long int ifreq_in_hz);
void myclock();
int colormap_r(double);
int colormap_g(double);
int colormap_b(double);

extern double *in;
extern fftw_complex *out;
extern fftw_plan p;

DrawingArea::DrawingArea ()
{
  std::cout << "DrawingArea constructor is called." << std::endl;
  set_size_request (99, 50); /* width is dummy, determined by radiobuttons */

  Glib::signal_timeout().connect( sigc::mem_fun(*this, &DrawingArea::on_timeout), 50 );

  #ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
  //Connect the signal handler if it isn't already a virtual method override:
  signal_draw().connect(sigc::mem_fun(*this, &DrawingArea::on_draw), false);
  #endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

  add_events(Gdk::BUTTON_PRESS_MASK | Gdk::SCROLL_MASK  |Gdk::SMOOTH_SCROLL_MASK);

  bin_size = rate / (double) NFFT;
  for (int i = 0; i < NFFT; i++) {
        fft_window[i] = 0.54 - 0.46 * cos (2.0 * M_PI * i / (double) NFFT);
  }

  in  = malloc (sizeof (double) * NFFT);
  out = (fftw_complex *) fftw_malloc (sizeof (fftw_complex) * (NFFT / 2 + 1));
  p   = fftw_plan_dft_r2c_1d (NFFT, in, out, FFTW_MEASURE);

}

DrawingArea::~DrawingArea ()
{
  std::cout << "DrawingArea destructor is called." << std::endl;
}

bool
DrawingArea::on_draw (const Cairo::RefPtr < Cairo::Context > &cr)
{
	static int icountx = 0;
	cout << "on_draw: icountx = " << icountx++ << endl;

   myclock(); /* to get inf from IC-7410 */
	cout << "on_draw: returned from myclock() \n";

  /* audio signal FFT */
        cout << "audio signal FFT begin1.. \n";
        for (int i = 0; i < NFFT; i++)
  	{
  	  in[i] = fft_window[i] * audio_signal[i];
  	}

        cout << "audio signal FFT begin2.. \n";
        fftw_execute (p);
        cout << "audio signal FFT begin3.. \n";

  /* log10 and normalize */

        for (int i = 0; i < NFFT / 4; i++)
  	{
  	  double val;
  	  val = out[i][0] * out[i][0] + out[i][1] * out[i][1];
  	  if (val < pow (10.0, amin))
  	    {
  	      audio_signal_ffted[i] = 0.0;
  	    }
  	  else if (val > pow (10.0, amax))
  	    {
  	      audio_signal_ffted[i] = 1.0;
  	    }
  	  else
  	    {
  	      audio_signal_ffted[i] = (log10 (val) - amin) / (amax - amin);
  	    }
  	}
    	cout << "on_draw: done fftw, etc. \n";

  Gtk::Allocation allocation = get_allocation ();
  const int width  = allocation.get_width ();
  const int height = allocation.get_height (); /* eg. 50 */
  const int rectangle_width  = width;
  const int rectangle_height = height; /* eg. 50 */

// rectangle box
  cr->save ();
  cr->set_source_rgb(0.0, 0.6, 0.2);
  cr->rectangle(0, 0, rectangle_width, rectangle_height);
  cr->fill();
  cr->stroke ();
  cr->restore ();


// frequency display
  cr->save ();
  draw_text(cr, rectangle_width, rectangle_height);
//  cr->set_source_rgb(0.1, 0.7, 0.3);
//  for(int i=0;i<10;i++) {
//	  cr->move_to(10.0+31.0*i, 3.0);
//	  cr->line_to(10.0+31.0*i,46.0);
//      cr->stroke();
//  }
  cr->restore ();

// S meter
  double s_frac = 2.0 * (double) s_meter / 255.0;
  if (s_frac > 1.0) { s_frac = 1.0; }
  cr->save ();
  cr->set_line_width (2);
  cr->set_source_rgba (colormap_r(s_frac)/255.0, colormap_g(s_frac)/255.0, colormap_b(s_frac), 1.0);	// partially translucent
  cr->rectangle(310,5,200*s_frac,40);
  cr->fill_preserve ();
  cr->set_source_rgb(0.0, 0.0, 0.0);
  cr->stroke();
  cr->restore ();

// Waveform
  cr->save ();
  cr->set_source_rgb(0.9, 0.9, 0.2);
  cr->move_to(520.0, audio_signal[0]/16384.0 * 24.0 + 25.0);
  for(int i=0;i<200;i++) {
	  cr->line_to(520+i, audio_signal[i]/16384.0 * 24.0 + 25.0);
  }
  cr->stroke();
  cr->restore ();

// Spectrum

  cr->save ();
  cr->set_source_rgb(0.2, 0.9, 0.9);
  cr->move_to(780.0, 40.0*(1.0-audio_signal_ffted[0])+5.0);
  for(int i=0;i<170;i++) {
	  cr->line_to(780+i, 40.0*(1.0-audio_signal_ffted[i])+5.0);
  }
  cr->stroke();
  cr->restore ();

  return true;
}

bool DrawingArea::on_timeout()
{
	static int icountw = 0, icountv = 0;
	cout << "on_timeout: icountw = " << icountw++ << endl;
	if(flag_togo == 0) {
		return true;
	}
	flag_togo = 0;
	cout << "on_timeout: icountv = " << icountv++ << endl;

    // force our program to redraw the entire clock.
    Glib::RefPtr<Gdk::Window> win = get_window();
    if (win)
    {
        Gdk::Rectangle r(0, 0, get_allocation().get_width(),
                get_allocation().get_height());
        win->invalidate_rect(r, false);
    }
    return true;
}

void DrawingArea::draw_text(const Cairo::RefPtr<Cairo::Context>& cr,
                       int rectangle_width, int rectangle_height)
{
  Pango::FontDescription font;

  font.set_family("Monospace");
  font.set_weight(Pango::WEIGHT_BOLD);
  font.set_size(40*1000); /* unit = 1/1000 point */

  char string[128];
  sprintf(string, "%9.3f", (double) ifreq_in_hz / 1000.0);
  Glib::RefPtr<Pango::Layout> layout = create_pango_layout(string);

  layout->set_font_description(font);

  int text_width;
  int text_height;

  layout->get_pixel_size(text_width, text_height);
  cr->move_to(10, (rectangle_height-text_height)/2);
  cr->set_source_rgb(1.0, 1.0, 1.0);
  layout->show_in_cairo_context(cr);
  cr->stroke();
}

bool DrawingArea::on_scroll_event(GdkEventScroll *event)
{
    double x, dy;
    x  = event->x;
    dy = event->delta_y;
    int digit_pos;
    digit_pos = (x-10.0) / 31.0;
    cout << "digit_pos = " << digit_pos << endl;

    int frequency_delta = 0.0;
    if(digit_pos >= 0 && digit_pos <=4) {
      frequency_delta = pow(10.0, 7-digit_pos);
    } else if(digit_pos >=6 && digit_pos <=8) {
        frequency_delta = pow(10.0, 8-digit_pos);
    }

    if(dy>0) {
    	frequency_delta = -frequency_delta;
    }

    int ifreq_in_hz_new = ifreq_in_hz + frequency_delta;
    if(ifreq_in_hz_new > 0 && ifreq_in_hz_new < 60000000) {
    	ifreq_in_hz = ifreq_in_hz_new;
    }
    set_freq (ifreq_in_hz);

    return true;
}
