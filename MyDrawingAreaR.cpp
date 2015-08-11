/*
 * MyDrawingAreaR.cpp
 *
 *  Created on: Jul 20, 2015
 *      Author: user1
 */

#include "MyDrawingAreaR.h"

#include <cmath>
#include <iostream>
#include <thread>
#include <stdlib.h>
using namespace std;

void f() {
	system("/usr/local/bin/soft66-control -t 7025400");
}

void g() {
	system("/usr/local/bin/soft66-control -t 7025100");
}

MyDrawingAreaR::MyDrawingAreaR(Rig* rr) : r {rr} {

	set_size_request(512, 60);
	Glib::signal_timeout().connect( sigc::mem_fun(*this, &MyDrawingAreaR::on_timeout), 100 );
	add_events(	Gdk::BUTTON_PRESS_MASK );

	cout << "MyDrawingAreaR::MyDrawingAreaR(): r = " << r << endl;

}

MyDrawingAreaR::~MyDrawingAreaR() {
	cout << "MyDrawingAreaR::~MyDrawingAreaR() destructor.." << endl;
}

bool MyDrawingAreaR::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {

	double phase = (count++ % 360) / 360.0 * 2.0 * 3.141592;
	cout << "MyDrawingAreaR::on_draw: count = " << count << endl;

	/* paint whole widget */
	{
		cr->save();
		cr->set_source_rgba(0.5+0.1*sin(phase), 0.5, 0.5+0.1*cos(phase), 1.0);
		cr->rectangle(0, 0, 530, 60);
		cr->fill();
		cr->stroke();
		cr->restore();
	}

	/* frequency display box */
	{
		cr->save();
		cr->set_source_rgba(0.5+0.2*sin(phase), 0.5+0.2*cos(phase), 0.5, 1.0);
		cr->rectangle(5, 5, 390, 50);
		cr->fill();
		cr->stroke();
		cr->restore();
	}

	/* frequency display */
	{
	cr->save();
	Pango::FontDescription font;
	font.set_family("Monospace");
	font.set_weight(Pango::WEIGHT_BOLD);
	font.set_size(40 * 1000); /* unit = 1/1000 point */
	int rectangle_height = 50;
	int text_width;
	int text_height;

	r->get_frequency();
	r->get_operating_mode();

	char string[128];
	if(r->nch == 1) {
		if(RigParams::operating_mode == 3) {
			sprintf(string, "%9.3fkHz LSB", (double) r->frequency / 1000.0);
		} else {
			sprintf(string, "%9.3fkHz USB", (double) r->frequency / 1000.0);
		}
	} else if(r->nch == 2) {
		sprintf(string, "%9.3fkHz", (double) r->frequency / 1000.0);
	}


	Glib::RefPtr<Pango::Layout> layout = create_pango_layout(string);

	layout->set_font_description(font);
	layout->get_pixel_size(text_width, text_height);
	cr->move_to(10, 5 + (rectangle_height - text_height) / 2);
	cr->set_source_rgb(1.0, 1.0, 1.0);
	layout->show_in_cairo_context(cr);
	cr->stroke();
	cr->restore();
	}

	/* check if waterfall clicked */
	if(RigParams::frequency_to_go) {
		r->set_frequency(RigParams::frequency_to_set);
		RigParams::frequency_to_go = false;
	}

	return true;
}

bool MyDrawingAreaR::on_timeout() {

	Glib::RefPtr<Gdk::Window> win = get_window();
	if (win) {
		Gdk::Rectangle rect(0, 0, get_allocation().get_width(), get_allocation().get_height());
		win->invalidate_rect(rect, false);
	}

	cout << "MyDrawingAreaR::on_timeout: win = " << win << ", width = " << get_allocation().get_width()
							<< ", height = " << get_allocation().get_height() << endl;
	return true;
}

