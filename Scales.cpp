#include "Scales.h"
#include <iostream>
using namespace std;
void set_cw_speed(int wpm);
void set_tx_power(int txp);

Scales::Scales():
m_HBox(Gtk::ORIENTATION_HORIZONTAL, 20),
m_VBox_wpm(Gtk::ORIENTATION_VERTICAL, 20),
m_VBox_txp(Gtk::ORIENTATION_VERTICAL, 20),
m_adjustment_wpm(Gtk::Adjustment::create(25.0, 6.0, 49.0, 1.0, 1.0, 1.0)),
m_adjustment_txp(Gtk::Adjustment::create(10.0, 5.0, 101.0, 1.0, 1.0, 1.0)),
m_HScale_wpm(m_adjustment_wpm, Gtk::ORIENTATION_HORIZONTAL),
m_HScale_txp(m_adjustment_txp, Gtk::ORIENTATION_HORIZONTAL)
{

    m_HScale_wpm.set_digits(0);
    m_HScale_wpm.set_value_pos(Gtk::POS_TOP);
    m_HScale_wpm.set_draw_value();
    m_adjustment_wpm->signal_value_changed().connect(sigc::mem_fun(*this,
								   &Scales::
								   on_adjustment_wpm_value_changed));
    m_HScale_txp.set_digits(0);
    m_HScale_txp.set_value_pos(Gtk::POS_TOP);
    m_HScale_txp.set_draw_value();
    m_adjustment_txp->signal_value_changed().connect(sigc::mem_fun(*this,
								   &Scales::
								   on_adjustment_txp_value_changed));

    add(m_HBox);
    m_HBox.set_border_width(10);
    m_HBox.pack_start(m_VBox_wpm);
    m_HBox.pack_start(m_VBox_txp);

    m_VBox_wpm.set_border_width(10);
    m_VBox_wpm.pack_start(m_HScale_wpm);
    m_VBox_wpm.pack_start(*Gtk::
			  manage(new Gtk::Label("CW Speed [wpm]", 0)),
			  Gtk::PACK_SHRINK);

    m_VBox_txp.set_border_width(10);
    m_VBox_txp.pack_start(m_HScale_txp);
    m_VBox_txp.pack_start(*Gtk::
			  manage(new Gtk::Label("TX Power [watts]", 0)),
			  Gtk::PACK_SHRINK);

    show_all();
}

Scales::~Scales()
{
}

void
 Scales::on_adjustment_wpm_value_changed()
{
    const double val = m_adjustment_wpm->get_value();
    cout << "CW Key Speed = " << val << endl;
    set_cw_speed((int) val);
}

void Scales::on_adjustment_txp_value_changed()
{
    const double val = m_adjustment_txp->get_value();
    cout << "TX Power = " << val << endl;
    set_tx_power((int) val);
}

//void
//Scales::on_button_quit ()
//{
//  hide ();
//}
