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
	set_size_request(5+2048+5,900);
	get_content_area()->pack_start(myscrolledwindow);

    mybox0.set_orientation(Gtk::ORIENTATION_VERTICAL);
    mybox1.set_orientation(Gtk::ORIENTATION_HORIZONTAL);

    mybox1.pack_start(mywaterfall0, FALSE, FALSE, 0);
    mybox1.pack_start(myarea0     , FALSE, FALSE, 0);

    mybox0.pack_start(myarea      , FALSE, FALSE, 0);
    mybox0.pack_start(mybox1      , FALSE, FALSE, 0);
    mybox0.pack_start(mywaterfall1, FALSE, FALSE, 0);
    mybox0.pack_start(mybuttons   , FALSE, FALSE, 0);
    mybox0.pack_start(myscales    , FALSE, FALSE, 0);
    myscrolledwindow.add(mybox0);
    add(myscrolledwindow);

	cout << "MyWindow constructor end.." << endl;
}

MyWindow::~MyWindow()
{
	cout << "MyWindow destructor is called." << endl;
}
