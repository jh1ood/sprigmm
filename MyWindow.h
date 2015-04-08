/*
 * MyWindow.h
 *
 *  Created on: Feb 28, 2015
 *      Author: user1
 */

#ifndef MYWINDOW_H_
#define MYWINDOW_H_
#include "radiobuttons.h"
#include "drawingarea.h"
#include "waterfall.h"
#include "Scales.h"
#include <gtkmm/window.h>

class MyWindow:public
  Gtk::Window
{
public:
  MyWindow ();
  virtual ~
  MyWindow ();
private:
  Gtk::Box
    mybox;
  RadioButtons
    mybuttons;
  Scales
    myscales;
  DrawingArea
    myarea;
  Waterfall
    mywaterfall;
};

#endif /* MYWINDOW_H_ */
