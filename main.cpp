//==========================
//       main.cpp
//==========================

#include "SoundIC7410.h"
#include "SoundSoft66.h"
#include "RigIC7410.h"
#include "RigSoft66.h"
#include "MyWindow.h"
#include "MyDrawingAreaR.h"
#include "MyDrawingAreaS.h"
#include <vector>
#include <iostream>
#include <chrono>
#include <stdlib.h>
#include <stdio.h>
#include <gtkmm.h>

using namespace std;

int main(int argc, char *argv[]) {

	if (argc != 4) {
		cout << "Usage example: " << argv[0] << " hw:2,0 hw:1,0 /dev/ttyUSB0" << endl;
		return 1;
	}
	if(system(NULL)) {
		system("/usr/local/bin/soft66-control -t 7020000");
	}

	std::chrono::system_clock::time_point start = chrono::system_clock::now();

	vector <Sound*> slist;
	SoundIC7410 s1{argv[1]};
//	SoundSoft66 s2{argv[2]};
	slist.push_back(&s1);
//	slist.push_back(&s2);

	vector <Rig*> rlist;
	RigIC7410 r1{argv[3]};
	RigSoft66 r2{nullptr};
	rlist.push_back(&r1);
	rlist.push_back(&r2);

	int argc_dummy = 1; /* to ignore argv */
	auto app = Gtk::Application::create(argc_dummy, argv, "app.id");

	MyWindow win(slist, rlist);

//	MyWindow win(argv[1], argv[2], argv[3]);
//	Gtk::Window win;
//	win.set_title("IC-7410 Rig Control Program (C++ version)");
//
//	MyDrawingAreaS area1 {&s1};
//	MyDrawingAreaS area2 {&s2};
//	MyDrawingAreaR area8 {&r1};
//	MyDrawingAreaR area9 {&r2};
//
//	Gtk::Box   myvbox(Gtk::ORIENTATION_VERTICAL,   10);
//	Gtk::Box   myhbox(Gtk::ORIENTATION_HORIZONTAL, 10);
//	myhbox.pack_start(area8,  Gtk::PACK_SHRINK);
//	myhbox.pack_start(area9,  Gtk::PACK_SHRINK);
//	myvbox.pack_start(area8,  Gtk::PACK_SHRINK);
//	myvbox.pack_start(area9,  Gtk::PACK_SHRINK);
//	myvbox.pack_start(myhbox, Gtk::PACK_SHRINK);
//	myvbox.pack_start(area2,  Gtk::PACK_SHRINK);
//	myvbox.pack_start(area1,  Gtk::PACK_SHRINK);
//	win.set_size_request(1610, 900);
//	win.add(myvbox);
//	win.show_all();

	auto end = chrono::system_clock::now();
	auto diff = end - start;
	cout << "elapsed time = " << chrono::duration_cast<std::chrono::milliseconds>(diff).count() << " msec." << endl;
	return app->run(win);
}
