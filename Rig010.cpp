//============================================================================
// Name        : Rig010.cpp
//============================================================================

#include "Mydefine.h"
#include "SoundIC7410.h"
#include "SoundSoft66.h"
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
	soundlist.push_back( new SoundIC7410{argv[1], argv[3]} );
	soundlist.push_back( new SoundSoft66{argv[2], nullptr} ); /* no rig control yet */

	argc = 1;			/* just for the next line */
	Glib::RefPtr < Gtk::Application > app = Gtk::Application::create(argc, argv, "org.gtkmm.example");
	MyWindow win(soundlist);
	return app->run(win);

}
