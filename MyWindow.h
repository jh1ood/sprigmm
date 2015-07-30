/*
 * MyWindow.h
 *
ls - */

#ifndef MYWINDOW_H_
#define MYWINDOW_H_

#include "Sound.h"
#include "Rig.h"
#include <gtkmm.h>
#include <vector>
using namespace std;

class MyWindow : public Gtk::Window {
public:
	MyWindow(const vector <Sound*> &slist, const vector <Rig*> &rlist);
//	MyWindow(char*, char*, char*);
	virtual ~MyWindow();
private:
  Gtk::Window mywindow;
  Gtk::VBox   myvbox;
};

#endif /* MYWINDOW_H_ */
