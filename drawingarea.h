#ifndef GTKMM_EXAMPLE_DRAWINGAREA_H
#define GTKMM_EXAMPLE_DRAWINGAREA_H

#include <gtkmm/drawingarea.h>

class DrawingArea : public Gtk::DrawingArea
{
public:
  DrawingArea();
  virtual ~DrawingArea();

protected:
  //Override default signal handler:
  virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
};

#endif // GTKMM_EXAMPLE_DRAWINGAREA_H
