/*
 * MyDrawingArea2.h
 *
 *  Created on: Jul 20, 2015
 *      Author: user1
 */

#ifndef MYDRAWINGAREA2_H_
#define MYDRAWINGAREA2_H_

#include "Mydefine.h"
#include "RigIC7410.h"
#include "RigSoft66.h"
#include <gtkmm.h>

class MyDrawingArea2 : public Gtk::DrawingArea, public RigParams {
public:
	MyDrawingArea2(Rig*);
	bool on_draw(const Cairo::RefPtr < Cairo::Context > &cr) override;
	bool on_timeout();
	virtual ~MyDrawingArea2();
private:
	Rig* r  {nullptr};
	int  count    {0};
};

#endif /* MYDRAWINGAREA2_H_ */
