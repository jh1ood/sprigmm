/*
 * MyWindow.cpp
 *
 *  Created on: Feb 28, 2015
 *      Author: user1
 */

#include "MyWindow.h"

MyWindow::MyWindow()
{
    mybox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    mybox.pack_start(myarea, FALSE, FALSE, 0);
    mybox.pack_start(mywaterfall1, FALSE, FALSE, 0);
//    mybox.pack_start(mywaterfall2, FALSE, FALSE, 0);
    mybox.pack_start(mybuttons, FALSE, FALSE, 0);
    mybox.pack_start(myscales, FALSE, FALSE, 0);
    add(mybox);

//    win3.set_title("from MyWindow");
//    win3.set_default_size(1024, 250);
//    win3.set_border_width(5);
//    win3.show_all();

}

MyWindow::~MyWindow()
{
}
