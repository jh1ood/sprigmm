//============================================================================
// Name        : Rig008.cpp
//============================================================================

//#include "Sound.h"
#include "SoundIC7410.h"
#include "SoundSoft66.h"
#include "MyDrawingArea.h"
#include <gtkmm.h>
#include <vector>
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
	if (argc != 3) {
		cout << "Usage example: " << argv[0] << " hw:2,0 hw:1,0 " << endl;
		return 1;
	}

	vector <Sound*> soundlist;
	soundlist.push_back( new SoundIC7410{argv[1]} );
	soundlist.push_back( new SoundSoft66{argv[2]} );

	argc = 1;			/* just for the next line */
	Glib::RefPtr < Gtk::Application > app = Gtk::Application::create(argc, argv, "org.gtkmm.example");

	Gtk::Window   mywindow;
	Gtk::VBox     mybox;

	mywindow.set_title("IC-7410 Rig Control Program (C++ version)");

	for(auto s : soundlist) {
		mybox.pack_start(*(new MyDrawingArea{s}), FALSE, FALSE, 0);
	}

	mywindow.add(mybox);
	mywindow.show_all();

	return app->run(mywindow);
}
