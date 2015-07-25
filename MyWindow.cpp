/*
 * MyWindow.cpp
 *
 *  Created on: Jul 11, 2015
 *      Author: user1
 */

#include "MyWindow.h"
#include "MyDrawingArea.h"
#include "MyDrawingArea2.h"

//MyWindow::MyWindow(const vector <Sound*> &slist, const vector <Rig*> &rlist) {
MyWindow::MyWindow(char* c1, char* c2, char* c3) {
	set_title("IC-7410 Rig Control Program (C++ version)");
	set_size_request(1610, 900);

//	int countr = 0;
//	for(auto r : rlist) {
//		cout << "MyWindow::MyWindow: countr = " << countr++ << endl;
//		myvbox.pack_start( *(new MyDrawingArea2{r}), FALSE, FALSE, 0);
//	}
//
//	int counts = 0;
//	for(auto s : slist) {
//		cout << "MyWindow::MyWindow: counts = " << counts++ << endl;
//		myvbox.pack_start( *(new MyDrawingArea{s}), FALSE, FALSE, 0);
//	}

//	SoundIC7410    sound1{c1};
//	SoundSoft66    sound2{c2};
//	RigIC7410      rig1  {c3};
//	MyDrawingArea2 area9 {&rig1};
//	MyDrawingArea  area1 {&sound1};
//	MyDrawingArea  area2 {&sound2};

	SoundIC7410* sound1 = new SoundIC7410{c1};
	SoundSoft66* sound2 = new SoundSoft66{c2};
	RigIC7410*   rig1   = new RigIC7410  {c3};
	MyDrawingArea* area1 = new MyDrawingArea{sound1};
	MyDrawingArea* area2 = new MyDrawingArea{sound2};
	MyDrawingArea2* area9 = new MyDrawingArea2{rig1};

	myvbox.pack_start(*area9);
	myvbox.pack_start(*area1);
	myvbox.pack_start(*area2);
	add(myvbox);
	show_all();
}

MyWindow::~MyWindow() {
}
