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
//MyWindow::MyWindow(char* c1, char* c2, char* c3) {
	set_title("IC-7410 Rig Control Program (C++ version), build=123");
	set_size_request(1610, 900);

	int countr = 0;
	for(auto r : rlist) {
		cout << "MyWindow::MyWindow: countr = " << countr++ << endl;
		myvbox.pack_start( *(new MyDrawingAreaR{r}), FALSE, FALSE, 0);
	}

	int counts = 0;
	for(auto s : slist) {
		cout << "MyWindow::MyWindow: counts = " << counts++ << endl;
		myvbox.pack_start( *(new MyDrawingAreaS{s}), FALSE, FALSE, 0);
	}

//	SoundIC7410*    sound1 = new SoundIC7410{c1};
//	SoundSoft66*    sound2 = new SoundSoft66{c2};
//	RigIC7410*      rig1   = new RigIC7410  {c3};
//	MyDrawingAreaS*  area1  = new MyDrawingAreaS{sound1};
//	MyDrawingAreaS*  area2  = new MyDrawingAreaS{sound2};
//	MyDrawingAreaR* area9  = new MyDrawingAreaR{rig1};
//
//	myvbox.pack_start(*area9);
//	myvbox.pack_start(*area1);
//	myvbox.pack_start(*area2);

	add(myvbox);
	show_all();
}

MyWindow::~MyWindow() {
}
