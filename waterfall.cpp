#include "waterfall.h"
#include <cairomm/context.h>
#include <gdkmm/general.h> // set_source_pixbuf()
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
extern int flag_togo1, flag_togo2;

void set_freq (long int ifreq_in_hz);
void myclock();
int colormap_r(double);
int colormap_g(double);
int colormap_b(double);

extern double *in;
extern fftw_complex *out;
extern fftw_plan p;

Waterfall::Waterfall ()
{
  std::cout << "Waterfall constructor is called." << std::endl;
  set_size_request (1024, 384); /* width is dummy, determined by radiobuttons */

//  m_image = Gdk::Pixbuf::create_from_file("sprig_image.png");
//  if(m_image) { cout << "m_image OK \n"; } else { cout << "m_image failed \n"; }

  m_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, 1024, 384);
  cout << "m_image size is" << m_image->get_width() << ", " << m_image->get_height() << endl;

  char* p;
  p = m_image->get_pixels();

  for(int j=0;j<2;j++) {
  for(int i=0;i<1024*64;i++) {
	  *p++ = i % 256;
	  *p++ = 0;
	  *p++ = 0;
  }
  for(int i=0;i<1024*64;i++) {
	  *p++ = 0;
	  *p++ = i % 256;
	  *p++ = 0;
  }
  for(int i=0;i<1024*64;i++) {
	  *p++ = 0;
	  *p++ = 0;
	  *p++ = i % 256;
  }
  }

  Glib::signal_timeout().connect( sigc::mem_fun(*this, &Waterfall::on_timeout), 50 );

  #ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
  //Connect the signal handler if it isn't already a virtual method override:
  signal_draw().connect(sigc::mem_fun(*this, &Waterfall::on_draw), false);
  #endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

  add_events(Gdk::BUTTON_PRESS_MASK | Gdk::SCROLL_MASK  |Gdk::SMOOTH_SCROLL_MASK);

}

Waterfall::~Waterfall ()
{
  std::cout << "Waterfall destructor is called." << std::endl;
}

bool
Waterfall::on_draw (const Cairo::RefPtr < Cairo::Context > &cr)
{

	if(!m_image) {
		cout << "Waterfall::on_draw: error m_image \n";
		return false;
	}
	static int icountx = 0;
	cout << "Waterfall::on_draw: icountx = " << icountx++ << endl;

//  Gtk::Allocation allocation = get_allocation ();
//  const int width  = allocation.get_width ();
//  const int height = allocation.get_height (); /* eg. 50 */

	  char* p;
	  p = m_image->get_pixels() + (icountx % 384) * 3 * 1024;
	  cout << "Waterfall::on_draw: p = " << p << endl;
	  for(int i=0;i<1024;i++) {
		  double tmp = audio_signal_ffted[i];
		  *p++ = colormap_r(tmp);
		  *p++ = colormap_g(tmp);
		  *p++ = colormap_b(tmp);
	  }


	  Gdk::Cairo::set_source_pixbuf(cr, m_image, 0,0);
	  cr->paint();
	  cr->stroke ();

  return true;
}

bool Waterfall::on_timeout()
{
	static int icountw = 0, icountv = 0;
	cout << "Waterfall::on_timeout: icountw = " << icountw++ << endl;
	if(flag_togo2 == 0) {
		return true;
	}
	flag_togo2 = 0;
	cout << "Waterfall::on_timeout: icountv = " << icountv++ << endl;

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

void Waterfall::draw_text(const Cairo::RefPtr<Cairo::Context>& cr,
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

bool Waterfall::on_scroll_event(GdkEventScroll *event)
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
