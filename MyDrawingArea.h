/*
 * MyDrawingArea.h
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#ifndef MYDRAWINGAREA_H_
#define MYDRAWINGAREA_H_

#include "Sound.h"
#include <gtkmm.h>
#include <iostream>
using namespace std;

class MyDrawingArea : public Gtk::DrawingArea, public AlsaParams {
public:
	MyDrawingArea();
	MyDrawingArea(Sound*);
	bool on_draw(const Cairo::RefPtr < Cairo::Context > &cr) override;
	bool on_timeout();
	virtual ~MyDrawingArea();
private:
	int count = 0;
	int nch = 0;
	int nsamples = 512;
	int nfft = 1024;
	int ntime = 128;
	const int waterfall_x = 512;
	const int waterfall_y = 128;
	Sound* y = nullptr;
	Glib::RefPtr < Gdk::Pixbuf > m_image;
};

#endif /* MYDRAWINGAREA_H_ */
