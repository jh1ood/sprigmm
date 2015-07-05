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
	int nsamples = 600;
	int nfft = 800;
	int ntime = 200;
	Sound* y = nullptr;
};

#endif /* MYDRAWINGAREA_H_ */
