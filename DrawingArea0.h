/*
 * DrawingArea0.h
 *
 *  Created on: Jun 2, 2015
 *      Author: user1
 */

#ifndef DRAWINGAREA0_H_
#define DRAWINGAREA0_H_

#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>
#include "Sound.h"

class DrawingArea0: public Gtk::DrawingArea {
public:
	DrawingArea0();
	virtual ~DrawingArea0();
  public:
    //Override default signal handler:
    virtual bool on_draw(const Cairo::RefPtr < Cairo::Context > &cr);
    bool         on_timeout();
  private:
    Glib::RefPtr < Gdk::Pixbuf > m_image;
};

#endif /* DRAWINGAREA0_H_ */
