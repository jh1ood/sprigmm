#include <cairomm/context.h>
#include <cmath>
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <cairo.h>
#include <glibmm/main.h>
#include "drawingarea.h"
using namespace std;

extern int s_meter;
extern long int ifreq_in_hz;
void set_freq (long int ifreq_in_hz);
void myclock();
int colormap_r(double);
int colormap_g(double);
int colormap_b(double);

DrawingArea::DrawingArea ()
{
  std::cout << "DrawingArea constructor is called." << std::endl;
  set_size_request (99, 50); /* width is dummy, determined by radiobuttons */

  Glib::signal_timeout().connect( sigc::mem_fun(*this, &DrawingArea::on_timeout), 100 );

  #ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
  //Connect the signal handler if it isn't already a virtual method override:
  signal_draw().connect(sigc::mem_fun(*this, &DrawingArea::on_draw), false);
  #endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

  add_events(Gdk::BUTTON_PRESS_MASK | Gdk::SCROLL_MASK  |Gdk::SMOOTH_SCROLL_MASK);
}

DrawingArea::~DrawingArea ()
{
  std::cout << "DrawingArea destructor is called." << std::endl;
}

bool
DrawingArea::on_draw (const Cairo::RefPtr < Cairo::Context > &cr)
{
  std::cout << "DrawingArea on_draw is called." << std::endl;

  myclock(); /* to get inf from IC-7410 */

  Gtk::Allocation allocation = get_allocation ();
  const int width  = allocation.get_width ();
  const int height = allocation.get_height (); /* eg. 50 */
  const int lesser = MIN (width, height);
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
  cr->set_line_width (lesser * 0.01);
  cr->set_source_rgba (colormap_r(s_frac)/255.0, colormap_g(s_frac)/255.0, colormap_b(s_frac), 1.0);	// partially translucent
  cr->rectangle(310,5,200*s_frac,40);
  cr->fill_preserve ();
  cr->restore ();		// back to opaque black

  cr->stroke ();

  return true;
}

bool DrawingArea::on_timeout()
{
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
