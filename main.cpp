//==========================
//       main.cpp
//==========================

#include "Mydefine.h"
#include "SoundIC7410.h"
#include "SoundSoft66.h"
#include "RigIC7410.h"
#include "RigSoft66.h"
#include "MyDrawingArea.h"
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
//	SoundIC7410 s1{argv[1]};
//	SoundSoft66 s2{argv[2]};
//	slist.push_back(&s1);
//	slist.push_back(&s2);
//
//	vector <Rig*> rlist;
//	RigIC7410 r1{argv[3]};
//	RigSoft66 r2{nullptr};
//	rlist.push_back(&r1);
//	rlist.push_back(&r2);

	argc = 1; /* just for the next line */
	Glib::RefPtr < Gtk::Application > app = Gtk::Application::create(argc, argv, "app.id");
//	MyWindow win(slist, rlist);
	MyWindow win(argv[1], argv[2], argv[3]);
	return app->run(win);

}
