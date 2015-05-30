/*
 * MyWindow.cpp
 *
 *  Created on: Feb 28, 2015
 *      Author: user1
 */

#include "MyWindow.h"

MyWindow::MyWindow()
{

	cout << "MyWindow constructor is called." << endl;

	myscrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
	set_size_request(2058,900);
	get_content_area()->pack_start(myscrolledwindow);

    mybox.set_orientation(Gtk::ORIENTATION_VERTICAL);
    mybox.pack_start(myarea, FALSE, FALSE, 0);
    mybox.pack_start(mywaterfall1, FALSE, FALSE, 0);
//    mybox.pack_start(mywaterfall2, FALSE, FALSE, 0);
    mybox.pack_start(mybuttons, FALSE, FALSE, 0);
    mybox.pack_start(myscales, FALSE, FALSE, 0);
    myscrolledwindow.add(mybox);
    add(myscrolledwindow);

	cout << "MyWindow constructor end.." << endl;
}

MyWindow::~MyWindow()
{
	cout << "MyWindow destructor is called." << endl;
}
