#include "radiobuttons.h"
#include "drawingarea.h"
#include "MyWindow.h"
#include "Scales.h"
#include <iostream>
#include <gtkmm/application.h>
#include <gtkmm/window.h>

void rig_init_serial (char *);
void rig_init_sound (char *);
//MyWindow win;

int
main (int argc, char *argv[])
{
  if (argc != 3)
    {
      std::cout << "Usage example: " << argv[0] << " /dev/ttyUSB0 hw:2,0 \n";
      std::
	cout <<
	" --> try % ls -l /dev/ttyUSB*, and % arecord -l to know these parameters.\n";
      return false;
    }
  std::
    cout << "serial_port = " << argv[1] << ", sound_device = " << argv[2] <<
    std::endl;
  rig_init_serial (argv[1]);
  rig_init_sound (argv[2]);

  argc = 1;			/* just for the next line */
  Glib::RefPtr < Gtk::Application > app =
    Gtk::Application::create (argc, argv, "org.gtkmm.example");

  MyWindow win;
  win.set_title ("IC-7410 Rig Control Program (C++ version)");
  win.set_default_size (50, 50);
  win.set_border_width (5);
  win.show_all_children ();

  return app->run (win);
}
