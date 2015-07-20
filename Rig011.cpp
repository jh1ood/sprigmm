//============================================================================
// Name        : Rig011.cpp
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
using namespace std;

int main(int argc, char *argv[]) {

	if (argc != 4) {
		cout << "Usage example: " << argv[0] << " hw:2,0 hw:1,0 /dev/ttyUSB0" << endl;
		return 1;
	}

	vector <Sound*> soundlist;
	soundlist.push_back( new SoundIC7410{argv[1]} );
	soundlist.push_back( new SoundSoft66{argv[2]} );

	vector <Rig*> riglist;
	riglist.push_back  ( new RigIC7410  {argv[3]} );

	argc = 1;			/* just for the next line */
	Glib::RefPtr < Gtk::Application > app = Gtk::Application::create(argc, argv, "org.gtkmm.example");
	MyWindow win(soundlist, riglist);
	return app->run(win);

}
