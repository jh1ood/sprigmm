/*
 * MyWindow.cpp
 *
 *  Created on: Feb 28, 2015
 *      Author: user1
 */

#include "MyWindow.h"

MyWindow::MyWindow()
{
	myscrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
	set_size_request(1920,900);
	get_content_area()->pack_start(myscrolledwindow);

    mybox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    mybox.pack_start(myarea, FALSE, FALSE, 0);
    mybox.pack_start(mywaterfall, FALSE, FALSE, 0);
    mybox.pack_start(mybuttons, FALSE, FALSE, 0);
    mybox.pack_start(myscales, FALSE, FALSE, 0);
    myscrolledwindow.add(mybox);
    add(myscrolledwindow);
}

MyWindow::~MyWindow()
{
}
