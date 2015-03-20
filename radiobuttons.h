#ifndef GTKMM_EXAMPLE_RADIOBUTTONS_H
#define GTKMM_EXAMPLE_RADIOBUTTONS_H

#include <gtkmm/box.h>
#include <gtkmm/window.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/separator.h>

// class RadioButtons : public Gtk::Window
class RadioButtons : public Gtk::Box
{
public:
  RadioButtons();
  virtual ~RadioButtons();
protected:
  void on_button_clicked ();
  void on_button_clicked9(gint data);
  Gtk::Box         m_Box_Top, m_Box1, m_Box2;
  Gtk::Box         m_Box10       [  8];
  Gtk::RadioButton::Group m_group[  8];
  Gtk::RadioButton m_RadioButton [100];
  Gtk::HSeparator  m_HSeparator;
  Gtk::VSeparator  m_VSeparator  [  7];
  Gtk::Button      m_Button_Quit;
};

#endif //GTKMM_EXAMPLE_RADIOBUTTONS_H
