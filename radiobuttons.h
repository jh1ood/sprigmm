#ifndef GTKMM_EXAMPLE_RADIOBUTTONS_H
#define GTKMM_EXAMPLE_RADIOBUTTONS_H

#include <gtkmm/box.h>
#include <gtkmm/window.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/separator.h>

class RadioButtons : public Gtk::Box
{
public:
  RadioButtons();
  virtual ~RadioButtons();
private:
  void on_button_clicked_quit();
  void on_button_clicked_all(gint data);
  Gtk::Box                m_Box_Top;
  Gtk::Box                m_Box1;
  Gtk::Box                m_Box2;
  Gtk::Box                m_BoxGroup    [100];
  Gtk::RadioButton::Group m_group       [100];
  Gtk::RadioButton        m_RadioButton [100];
  Gtk::HSeparator         m_HSeparator;
  Gtk::VSeparator         m_VSeparator  [100];
  Gtk::Button             m_Button_Quit;
};

#endif //GTKMM_EXAMPLE_RADIOBUTTONS_H
