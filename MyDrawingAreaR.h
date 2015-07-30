/*
 * MyDrawingArea2.h
 *
 *  Created on: Jul 20, 2015
 *      Author: user1
 */

#ifndef MYDRAWINGAREAR_H_
#define MYDRAWINGAREAR_H_

#include "RigIC7410.h"
#include "RigSoft66.h"
#include <gtkmm.h>

class MyDrawingAreaR : public Gtk::DrawingArea, public RigParams {
public:
	MyDrawingAreaR(Rig*);
	virtual ~MyDrawingAreaR();

	bool on_draw(const Cairo::RefPtr < Cairo::Context > &cr) override;
	bool on_timeout();
private:
	Rig* r  {nullptr};
	int  count    {0};
};

#endif /* MYDRAWINGAREAR_H_ */
