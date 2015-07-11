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

class MyDrawingArea : public Gtk::DrawingArea {
public:
	MyDrawingArea(Sound*);
	bool on_draw(const Cairo::RefPtr < Cairo::Context > &cr) override;
	bool on_timeout();
	virtual ~MyDrawingArea();
private:
	int count = 0;
	int xspacing = 5;
	int yspacing = 5;
	int waveform_x = 800;
	int waveform_y = 100;
	int spectrum_x = 1024;
	int spectrum_y = 128;
	int waterfall_x = 1024;
	int waterfall_y =  150;
	int nch = 0;
	int nfft = 0;
	Sound* y = nullptr;
	Glib::RefPtr < Gdk::Pixbuf > m_image;
	guint8 *p = nullptr;
	int rowstride = 0;
};

#endif /* MYDRAWINGAREA_H_ */
