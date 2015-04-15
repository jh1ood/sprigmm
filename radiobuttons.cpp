#include "mydefine.h"
#include "radiobuttons.h"
#include <cairomm/context.h>
#include <iostream>
#include <string>
#include "drawingarea.h"
using namespace std;

void send_commandx(const vector < unsigned char >&partial_command);
void set_operating_modex();

RadioButtons::RadioButtons():
m_Box_Top(Gtk::ORIENTATION_VERTICAL),
m_Box1(Gtk::ORIENTATION_HORIZONTAL, 10),
m_Box2(Gtk::ORIENTATION_VERTICAL, 10)
{
    vector < vector < string >> label {
	{
//	"CW", "CW-R", "RTTY", "RTTY-R", "LSB", "USB", "AM", "FM"}, {
	"CW", "CW-R", "LSB", "USB"}, {
	"DSP FIL 1", "DSP FIL 2", "DSP FIL 3"}, {
	"DSP SHARP", "DSP SOFT"}, {
	"IF FIL 1", "IF FIL 2", "IF FIL 3"}, {
	"PRE-AMP OFF", "PRE-AMP 1", "PRE-AMP 2"}, {
	"ATT OFF", "ATT 20dB"}, {
	"AGC FAST", "AGC MID", "AGC SLOW"}, {
	"ANT 1", "ANT 2"}, {
    "BKIN OFF", "BKIN SEMI", "BKIN FULL"},};

    int index = 0;
    for (unsigned int i = 0; i < label.size(); i++) {
	m_BoxGroup[i].set_orientation(Gtk::ORIENTATION_VERTICAL);
	m_BoxGroup[i].set_border_width(3);
	if (i != 0) {
	    m_Box1.pack_start(m_VSeparator[i]);
	}
	for (unsigned int j = 0; j < label.at(i).size(); j++) {
	    m_RadioButton[index].set_label(label[i][j]);
	    map_from_label_to_index[label[i][j]] = index; /* map */
	    if (j == 0) {
		m_RadioButton[index].set_active();
		m_group[i] = m_RadioButton[index].get_group();
	    } else {
		m_RadioButton[index].set_group(m_group[i]);
	    }
	    m_BoxGroup[i].pack_start(m_RadioButton[index], FALSE, FALSE,
				     0);
//	    m_RadioButton[index].signal_clicked().connect(sigc::bind < gint >   (mem_fun(*this,&RadioButtons::on_button_clicked_all), index));
	    m_RadioButton[index].signal_clicked().connect(sigc::bind < string > (mem_fun(*this,&RadioButtons::on_button_clicked_all), label[i][j]));
	    index++;
	}
	m_Box1.pack_start(m_BoxGroup[i]);
    }

    m_Box1.set_border_width(10);
    m_Box2.set_border_width(10);
    m_Box_Top.pack_start(m_Box1);
    m_Box_Top.pack_start(m_Box2);
    pack_start(m_Box_Top);

    show_all_children();

    Glib::signal_timeout().connect(sigc::
				   mem_fun(*this,
					   &RadioButtons::on_timeout),
				   500);

}

RadioButtons::~RadioButtons()
{
}

bool RadioButtons::on_timeout()
{
    if (operating_mode == 0x03) {	/* CW */
	m_RadioButton[0].set_active();
    }
    if (operating_mode == 0x07) {	/* CW-R */
	m_RadioButton[1].set_active();
    }
    if (operating_mode == 0x00) {	/* LSB */
	m_RadioButton[4].set_active();
    }
    if (operating_mode == 0x01) {	/* USB */
	m_RadioButton[5].set_active();
    }
    if (dsp_filter == 0x01) {	/* FIL1 */
	m_RadioButton[8].set_active();
    }
    if (dsp_filter == 0x02) {	/* FIL2 */
	m_RadioButton[9].set_active();
    }
    if (dsp_filter == 0x03) {	/* FIL3 */
	m_RadioButton[10].set_active();
    }
    cout << "RadioButtons::on_timeout() operating_mode = " <<
	operating_mode << ", dsp_filter = " << dsp_filter << endl;

    return true;
}

void RadioButtons::on_button_clicked_quit()
{
    hide();			//to close the application.
}

//void RadioButtons::on_button_clicked_all(int index)
void RadioButtons::on_button_clicked_all(string label)
{
//    static vector < unsigned char >command1x = { 0x06, 0x03, 0x01 };	/* OP MODE & FIL SET */
    static vector < unsigned char >command46x = { 0x16, 0x55, 0x00 };	/* IF FIL1/2/3 */
    static vector < unsigned char >command19x = { 0x16, 0x56, 0x00 };	/* DSP SHARP/SOFT */
    static vector < unsigned char >command49x = { 0x16, 0x02, 0x00 };	/* PRE-AMP OFF/1/2 */
    static vector < unsigned char >command12x = { 0x11, 0x00 };	/* ATT OFF/20dB  */
    static vector < unsigned char >command16x = { 0x16, 0x12, 0x01 };	/* AGC FAST/MID/SLOW  */
    static vector < unsigned char >command14x = { 0x12, 0x00 };	/* ANT 1/2 */
    static vector < unsigned char >command43x = { 0x16, 0x47, 0x00 };	/* BKIN OFF/SEMI/FULL  */

    int index = map_from_label_to_index[label];

    if (m_RadioButton[index].get_active()) {
	cout << "RadioButtons::on_button_clicked_all(): index = " << index
	     << ", label = " << label << endl;
    }

	if(0) {
	} else if(label == "CW") {
		operating_mode = 0x03;
		set_operating_modex();
		return;
    } else if(label == "CW-R") {
		operating_mode = 0x07;
		set_operating_modex();
		return;
    } else if(label == "RTTY") {
		operating_mode = 0x04;
		set_operating_modex();
		return;
    } else if(label == "RTTY-R") {
		operating_mode = 0x08;
		set_operating_modex();
		return;
    } else if(label == "LSB") {
		operating_mode = 0x00;
		set_operating_modex();
		return;
    } else if(label == "USB") {
		operating_mode = 0x01;
		set_operating_modex();
		return;
    } else if(label == "AM") {
		operating_mode = 0x02;
		set_operating_modex();
		return;
    } else if(label == "FM") {
		operating_mode = 0x05;
		set_operating_modex();
		return;
    } else if(label == "DSP FIL 1" || label == "DSP FIL 2" || label == "DSP FIL 3") {
		dsp_filter = index - map_from_label_to_index["DSP FIL 1"] + 0x01; /* start from 0x01 */
		set_operating_modex();
		return;
    } else if(label == "DSP SHARP" || label == "DSP SOFT") {
	    command19x[2] = index - map_from_label_to_index["DSP SHARP"];
	    send_commandx(command19x);
	    receive_fb();
		return;
		return;
    } else if(label == "IF FIL 1" || label == "IF FIL 2" || label == "IF FIL 3") {
	    command46x[2] = index - map_from_label_to_index["IF FIL 1"];
	    send_commandx(command46x);
	    receive_fb();
	    return;
    } else if(label == "PRE-AMP OFF" || label == "PRE-AMP 1" || label == "PRE-AMP 2") {
	    command49x[2] = index - map_from_label_to_index["PRE-AMP OFF"];;
	    send_commandx(command49x);
	    receive_fb();
	    return;
    } else if(label == "ATT OFF" || label == "ATT 20dB") {
	    command12x[1] = (index - map_from_label_to_index["ATT OFF"]) * 0x20; /* need 0x20 */
	    send_commandx(command12x);
	    receive_fb();
	    return;
    } else if(label == "AGC FAST" || label == "AGC MID" || label == "AGC SLOW") {
	    command16x[2] = (index - map_from_label_to_index["AGC FAST"]) + 0x01;	/* start from 0x01 */
	    send_commandx(command16x);
	    receive_fb();
	    return;
    } else if(label == "ANT 1" || label == "ANT 2") {
	    command14x[1] = index - map_from_label_to_index["ANT 1"];
	    send_commandx(command14x);
	    receive_fb();
	    return;
    } else if(label == "BKIN OFF" || label == "BKIN SEMI" || label == "BKIN FULL") {
	    command43x[2] = index - map_from_label_to_index["BKIN OFF"];
	    send_commandx(command43x);
	    receive_fb();
	    return;
    }

}
