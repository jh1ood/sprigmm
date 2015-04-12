/*
 * Scales.h
 *
 *  Created on: Feb 15, 2015
 *      Author: user1
 */

#ifndef SCALES_H_
#define SCALES_H_
#include <gtkmm.h>

class Scales:public Gtk::Box {
  public:
    Scales();
    virtual ~ Scales();
  protected:
    void
     on_adjustment_wpm_value_changed();
    void
     on_adjustment_txp_value_changed();
    Gtk::Box m_HBox, m_VBox_wpm, m_VBox_txp;
    Glib::RefPtr < Gtk::Adjustment > m_adjustment_wpm, m_adjustment_txp;
    Gtk::Scale m_HScale_wpm, m_HScale_txp;

};

#endif				/* SCALES_H_ */
