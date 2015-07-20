/*
 * MyDrawingArea2.cpp
 *
 *  Created on: Jul 20, 2015
 *      Author: user1
 */

#include "MyDrawingArea2.h"

#include <iostream>
#include <cmath>
using namespace std;

MyDrawingArea2::MyDrawingArea2(Rig* rr) : r {rr} {

	set_size_request(400, 60);
	Glib::signal_timeout().connect( sigc::mem_fun(*this, &MyDrawingArea2::on_timeout), 100 );
	add_events(	Gdk::BUTTON_PRESS_MASK );

}

MyDrawingArea2::~MyDrawingArea2() {
}

bool MyDrawingArea2::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {

	static int count = 0;
	double phase = (count++ % 360) / 360.0 * 2.0 * 3.141592;

	/* frequency display box */
	cr->save();
	cr->set_source_rgba(0.5, 0.5+0.2*sin(phase), 0.5+0.2*cos(phase), 1.0);
	cr->rectangle(5, 5, 390, 50);
	cr->fill();
	cr->stroke();

	/* frequency display */
	Pango::FontDescription font;
	font.set_family("Monospace");
	font.set_weight(Pango::WEIGHT_BOLD);
	font.set_size(40 * 1000); /* unit = 1/1000 point */
	int rectangle_height = 50;
	int text_width;
	int text_height;

	char string[128];
	sprintf(string, "%9.3fkHz", (double) r->get_frequency() / 1000.0);
	Glib::RefPtr<Pango::Layout> layout = create_pango_layout(string);

	layout->set_font_description(font);
	layout->get_pixel_size(text_width, text_height);
	cr->move_to(10, 5 + (rectangle_height - text_height) / 2);
	cr->set_source_rgb(1.0, 1.0, 1.0);
	layout->show_in_cairo_context(cr);
	cr->stroke();

	/* check if waterfall clicked */
	if(RigParams::frequency_to_go) {
		r->set_frequency(RigParams::frequency_to_set);
		RigParams::frequency_to_go = false;
	}


	return true;
}

bool MyDrawingArea2::on_timeout() {

	Glib::RefPtr<Gdk::Window> win = get_window();
	if (win) {
		Gdk::Rectangle rrr(0, 0, get_allocation().get_width(),
				get_allocation().get_height());
		win->invalidate_rect(rrr, false);
	}

	cout << "MyDrawingArea2::on_timeout: win = " << win << ", width = " << get_allocation().get_width()
					<< ", height = " << get_allocation().get_height() << endl;
	return true;
}

