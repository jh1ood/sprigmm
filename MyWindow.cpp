/*
 * MyWindow.cpp
 *
 *  Created on: Jul 11, 2015
 *      Author: user1
 */

#include "MyWindow.h"
#include "MyDrawingArea.h"

MyWindow::MyWindow(vector <Sound*> &slist) {
	myscrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
	set_size_request(1200, 600);
	get_content_area()->pack_start(myscrolledwindow);
	for(auto s : slist) {
		myvbox.pack_start( *(new MyDrawingArea{s}), FALSE, FALSE, 0);
	}
	myscrolledwindow.add(myvbox);
	add(myscrolledwindow);
}

MyWindow::~MyWindow() {
	// TODO Auto-generated destructor stub
}

