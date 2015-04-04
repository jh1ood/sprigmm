#ifndef GTKMM_EXAMPLE_DRAWINGAREA_H
#define GTKMM_EXAMPLE_DRAWINGAREA_H

#include <gtkmm/drawingarea.h>

class DrawingArea:public
  Gtk::DrawingArea
{
public:
  DrawingArea ();
  virtual ~
  DrawingArea ();

protected:
  //Override default signal handler:
  virtual
    bool
  on_draw (const Cairo::RefPtr < Cairo::Context > &cr);
  bool on_timeout();

private:
  void draw_rectangle(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height);
  void draw_text     (const Cairo::RefPtr<Cairo::Context>& cr, int rectangle_width, int rectangle_height);
};

#endif // GTKMM_EXAMPLE_DRAWINGAREA_H
