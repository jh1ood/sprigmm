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
#include <gtkmm/scrolledwindow.h>

//class MyWindow:public Gtk::Window {
class MyWindow:public Gtk::Dialog {
  public:
    MyWindow();
    virtual ~ MyWindow();
  private:
    Gtk::ScrolledWindow myscrolledwindow;
    Gtk::Box mybox;
    RadioButtons mybuttons;
    Scales myscales;
    DrawingArea myarea;
    Waterfall mywaterfall;
//    Gtk::Window win3;
};

#endif				/* MYWINDOW_H_ */
