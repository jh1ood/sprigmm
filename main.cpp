//============================================================================
// Name        :main.cpp
//============================================================================

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

	vector <Sound*> soundlist;
	SoundIC7410 sound1{argv[1]};
	SoundSoft66 sound2{argv[2]};
	soundlist.push_back(&sound1);
	soundlist.push_back(&sound2);

	vector <Rig*> riglist;
	RigIC7410 rig1{argv[3]};
	RigSoft66 rig2{nullptr};
	riglist.push_back(&rig1);
	riglist.push_back(&rig2);

	argc = 1;			/* just for the next line */
	Glib::RefPtr < Gtk::Application > app = Gtk::Application::create(argc, argv, "org.gtkmm.example");
	MyWindow win(soundlist, riglist);
	return app->run(win);

}
