#include <cairomm/context.h>
#include <cmath>
#include <ctime>
#include <iostream>
#include <glibmm/main.h>
#include "drawingarea.h"
extern int s_meter;
void myclock();
int colormap_r(double);
int colormap_g(double);
int colormap_b(double);

DrawingArea::DrawingArea ()
{
  std::cout << "DrawingArea constructor is called." << std::endl;
  set_size_request (100, 200);

  Glib::signal_timeout().connect( sigc::mem_fun(*this, &DrawingArea::on_timeout), 250 );

  #ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
  //Connect the signal handler if it isn't already a virtual method override:
  signal_draw().connect(sigc::mem_fun(*this, &DrawingArea::on_draw), false);
  #endif //GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED

}

DrawingArea::~DrawingArea ()
{
  std::cout << "DrawingArea destructor is called." << std::endl;
}

bool
DrawingArea::on_draw (const Cairo::RefPtr < Cairo::Context > &cr)
{
  std::cout << "MyArea on_draw is called." << std::endl;

  myclock(); /* to get inf from IC-7410 */

  Gtk::Allocation allocation = get_allocation ();
  const int width = allocation.get_width ();
  const int height = allocation.get_height ();
  const int lesser = MIN (width, height);

  int xc, yc;
  xc = width / 2;
  yc = height / 2;

  cr->set_line_width (lesser * 0.02);

  double s_frac = (double) s_meter / 255.0;

  // now draw a circle
  cr->save ();
  cr->arc (xc/4, yc, (0.2+s_frac)* lesser / 4.0, 0.0, 2.0 * M_PI);	// full circle
  cr->set_source_rgba (colormap_r(s_frac)/255.0, colormap_g(s_frac)/255.0, colormap_b(s_frac), 1.0);	// partially translucent
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
