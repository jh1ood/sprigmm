/*
 * MySubwindow1.h
 *
 *  Created on: Apr 11, 2015
 *      Author: user1
 */

#ifndef MYSUBWINDOW1_H_
#define MYSUBWINDOW1_H_

#include "Subarea.h"
#include <gtkmm/window.h>

class MySubwindow1:public Gtk::Window {
  public:
    MySubwindow1();
    virtual ~ MySubwindow1();
  private:
    Subarea mysubarea;
};

#endif				/* MYSUBWINDOW1_H_ */
