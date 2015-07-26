/*
 * MyDrawingArea.h
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#ifndef MYDRAWINGAREA_H_
#define MYDRAWINGAREA_H_

#include "Mydefine.h"
#include "Sound.h"
#include "SoundIC7410.h"
#include "SoundSoft66.h"
#include "RigParams.h"
#include <gtkmm.h>

class MyDrawingArea : public Gtk::DrawingArea, public RigParams {
public:
	MyDrawingArea(Sound*);
	bool on_draw(const Cairo::RefPtr < Cairo::Context > &cr) override;
	bool on_button_press_event(GdkEventButton * event) override;
	bool on_timeout();
	virtual ~MyDrawingArea();
private:
	Sound* s  {nullptr};
	int nch         {0}; /* parameters that depend on Sound* */
	int nfft        {0};
	int spectrum_x  {0};
	int spectrum_y  {0};
	int waterfall_x {0};
	int waterfall_y {0};

	double x_press;      /* for on_button_press_event() */
    double y_press;

	int count         {0};
	int xspacing     {10};
	int yspacing     {10};
	int waveform_x    {0};
	int waveform_y    {0};
	int size_x      {  0};
	int size_y      {  0};

	Glib::RefPtr < Gdk::Pixbuf > m_image;
	guint8 *p {nullptr};
	int rowstride    {0};
};

#endif /* MYDRAWINGAREA_H_ */
