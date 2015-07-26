//==========================
//       main.cpp
//==========================

#include "Mydefine.h"
#include "SoundIC7410.h"
#include "SoundSoft66.h"
#include "RigIC7410.h"
#include "RigSoft66.h"
#include "MyDrawingArea.h"
#include "MyDrawingArea2.h"
#include "MyWindow.h"
#include <gtkmm.h>
#include <vector>
#include <iostream>
#include <stdlib.h>
using namespace std;

int main(int argc, char *argv[]) {

	if (argc != 4) {
		cout << "Usage example: " << argv[0] << " hw:2,0 hw:1,0 /dev/ttyUSB0" << endl;
		return 1;
	}

	if(system(NULL)) {
		system("/usr/local/bin/soft66-control -t 7020000");
	}

//	vector <Sound*> slist;
	SoundIC7410 s1{argv[1]};
	SoundSoft66 s2{argv[2]};
//	slist.push_back(&s1);
//	slist.push_back(&s2);
//
//	vector <Rig*> rlist;
	RigIC7410 r1{argv[3]};
	RigSoft66 r2{nullptr};
//	rlist.push_back(&r1);
//	rlist.push_back(&r2);

	int argc_dummy = 1; /* to ignore argv[>=1] */
	auto app = Gtk::Application::create(argc_dummy, argv, "app.id");

//	MyWindow win(slist, rlist);
//	MyWindow win(argv[1], argv[2], argv[3]);
	Gtk::Window win;
	win.set_title("IC-7410 Rig Control Program (C++ version)");

	MyDrawingArea  area1 {&s1};
	MyDrawingArea  area2 {&s2};
	MyDrawingArea2 area8 {&r1};
	MyDrawingArea2 area9 {&r2};

	Gtk::Box   myvbox(Gtk::ORIENTATION_VERTICAL,   0);
	Gtk::Box   myhbox(Gtk::ORIENTATION_HORIZONTAL, 0);
	myhbox.pack_start(area8,  Gtk::PACK_SHRINK);
	myhbox.pack_start(area9,  Gtk::PACK_SHRINK);
//	myvbox.pack_start(area8,  Gtk::PACK_SHRINK);
//	myvbox.pack_start(area9,  Gtk::PACK_SHRINK);
	myvbox.pack_start(myhbox, Gtk::PACK_SHRINK);
	myvbox.pack_start(area2,  Gtk::PACK_SHRINK);
	myvbox.pack_start(area1,  Gtk::PACK_SHRINK);

//	win.set_size_request(1610, 900);
	win.add(myvbox);
	win.show_all();

	return app->run(win);
}
