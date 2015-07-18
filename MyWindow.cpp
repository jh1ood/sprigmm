/*
 * MyWindow.cpp
 *
 *  Created on: Jul 11, 2015
 *      Author: user1
 */

#include "MyWindow.h"
#include "MyDrawingArea.h"

MyWindow::MyWindow(vector <Sound*> &slist) {
	set_title("IC-7410 Rig Control Program (C++ version)");
	set_size_request(1800, 900);
	myscrolledwindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
	get_content_area()->pack_start(myscrolledwindow);
	for(auto s : slist) {
		myvbox.pack_start( *(new MyDrawingArea{s}), FALSE, FALSE, 0);
	}
	myscrolledwindow.add(myvbox);
	add(myscrolledwindow);
	show_all();
}

MyWindow::~MyWindow() {
}
