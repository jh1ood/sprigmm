#ifndef GTKMM_EXAMPLE_WATERFALL_H
#define GTKMM_EXAMPLE_WATERFALL_H

#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>

class Waterfall:public Gtk::DrawingArea {
  public:
    Waterfall();
    virtual ~ Waterfall();

  protected:
    //Override default signal handler:
    virtual bool on_draw(const Cairo::RefPtr < Cairo::Context > &cr);
    virtual bool on_button_press_event(GdkEventButton * event);
    bool on_scroll_event(GdkEventScroll * event);
    bool on_timeout();
    Glib::RefPtr < Gdk::Pixbuf > m_image;

  private:
    void draw_text(const Cairo::RefPtr < Cairo::Context > &cr,
		   int rectangle_width, int rectangle_height);
    double x_press;
    double y_press;
};

#endif				// GTKMM_EXAMPLE_WATERFALL_H
