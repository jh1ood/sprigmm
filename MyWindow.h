/*
 * MyWindow.h
 *
 *  Created on: Feb 28, 2015
 *      Author: user1
 */

#ifndef MYWINDOW_H_
#define MYWINDOW_H_
#include "mydefine.h"
#include "radiobuttons.h"
#include "drawingarea.h"
#include "DrawingArea0.h"
#include "waterfall.h"
#include "waterfall0.h"
#include "waterfall1.h"
#include "Scales.h"
#include <gtkmm/window.h>
#include <gtkmm/scrolledwindow.h>

//class MyWindow:public Gtk::Window {
class MyWindow:public Gtk::Dialog {
  public:
    MyWindow();
    virtual ~ MyWindow();
  private:
    Gtk::ScrolledWindow myscrolledwindow;
    Gtk::Box     mybox0;
    Gtk::Box     mybox1;
    RadioButtons mybuttons;
    Scales       myscales;
    DrawingArea  myarea;
    DrawingArea0 myarea0;
    Waterfall0   mywaterfall0;
    Waterfall1   mywaterfall1;
private:
//    Waterfall2 mywaterfall2;
};

#endif				/* MYWINDOW_H_ */
