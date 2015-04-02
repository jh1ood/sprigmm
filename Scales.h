/*
 * Scales.h
 *
 *  Created on: Feb 15, 2015
 *      Author: user1
 */

#ifndef SCALES_H_
#define SCALES_H_
#include <gtkmm.h>

class Scales:public
  Gtk::Box
{
public:
  Scales ();
  virtual ~
  Scales ();
protected:
  void
  on_checkbutton_toggled ();
  void
  on_combo_position ();
  void
  on_adjustment0_value_changed ();
  void
  on_adjustment1_value_changed ();
  void
  on_adjustment2_value_changed ();
  void
  on_button_quit ();
  Gtk::Box
    m_VBox_Top,
    m_VBox2,
    m_VBox_HScale;
  Gtk::Box
    m_HBox_Scales,
    m_HBox_Combo,
    m_HBox_Digits,
    m_HBox_PageSize;
  Glib::RefPtr <
    Gtk::Adjustment >
    m_adjustment,
    m_adjustment_digits,
    m_adjustment_pagesize;
  Gtk::Scale
    m_VScale;
  Gtk::Scale
    m_HScale,
	m_Scale,
    m_Scale_Digits,
    m_Scale_PageSize;
  Gtk::Separator
    m_Separator;
  Gtk::CheckButton
    m_CheckButton;
  Gtk::Scrollbar
    m_Scrollbar;

  class
    ModelColumns:
    public
    Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns ()
    {
      add (m_col_position_type);
      add (m_col_title);
    }

    Gtk::TreeModelColumn <
      Gtk::PositionType >
      m_col_position_type;
    Gtk::TreeModelColumn <
      Glib::ustring >
      m_col_title;
  };

  ModelColumns
    m_Columns;

  Gtk::ComboBox m_ComboBox_Position;
  Glib::RefPtr < Gtk::ListStore > m_refTreeModel;
  Gtk::Button m_Button_Quit;
};

#endif /* SCALES_H_ */
