/*
 * Subarea.h
 *
 *  Created on: Apr 11, 2015
 *      Author: user1
 */

#ifndef SUBAREA_H_
#define SUBAREA_H_

#include <gtkmm/drawingarea.h>
#include <gdkmm/pixbuf.h>

class Subarea:public Gtk::DrawingArea {
  public:
    Subarea();
    virtual ~ Subarea();
};

#endif				/* SUBAREA_H_ */
