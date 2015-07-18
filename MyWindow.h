/*
 * MyWindow.h
 *
 *  Created on: Jul 11, 2015
 *      Author: user1
 */

#ifndef MYWINDOW_H_
#define MYWINDOW_H_

#include "Sound.h"
#include <gtkmm.h>
#include <vector>
using namespace std;

class MyWindow : public Gtk::Dialog {
public:
	MyWindow(vector <Sound*> &slist);
	virtual ~MyWindow();
private:
  Gtk::ScrolledWindow myscrolledwindow;
  Gtk::VBox           myvbox;
};

#endif /* MYWINDOW_H_ */
