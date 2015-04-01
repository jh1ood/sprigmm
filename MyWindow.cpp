/*
 * MyWindow.cpp
 *
 *  Created on: Feb 28, 2015
 *      Author: user1
 */

#include "MyWindow.h"

MyWindow::MyWindow() {
	  mybox.set_orientation(Gtk::ORIENTATION_VERTICAL);
	  mybox.pack_start(mybuttons, FALSE, FALSE, 0);
	  mybox.pack_start(myscales , FALSE, FALSE, 0);
	  mybox.pack_start(myarea   , FALSE, FALSE, 0);
	  add(mybox);
}

MyWindow::~MyWindow() {
}

