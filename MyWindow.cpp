/*
 * MyWindow.cpp
 *
 *  Created on: Jul 11, 2015
 *      Author: user1
 */


#include "MyWindow.h"

#include "MyDrawingAreaR.h"
#include "MyDrawingAreaS.h"

MyWindow::MyWindow(const vector <Sound*> &slist, const vector <Rig*> &rlist) {
	set_title("IC-7410 Rig Control Program (C++ version), build=0812b");

	for(auto r : rlist) {
		myvbox.pack_start( *(new MyDrawingAreaR{r}), FALSE, FALSE, 0);
	}

	for(auto s : slist) {
		myvbox.pack_start( *(new MyDrawingAreaS{s}), FALSE, FALSE, 0);
	}

	add(myvbox);
	show_all();
}

MyWindow::~MyWindow() {
}
