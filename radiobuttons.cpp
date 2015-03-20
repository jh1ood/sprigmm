#include "radiobuttons.h"
#include "myarea.h"
#include <cairomm/context.h>
#include <iostream>
void myfunc (int);

RadioButtons::RadioButtons() :
  m_Box_Top(Gtk::ORIENTATION_VERTICAL),
  m_Box1   (Gtk::ORIENTATION_HORIZONTAL, 10),
  m_Box2   (Gtk::ORIENTATION_VERTICAL  , 10),
  m_Button_Quit ("Quit")
{
/*
  set_title("Spinor Lab");
  set_default_size(1000,400);
  set_border_width(10);
*/

  char label[100][256] = { "CW", "CW-R", "LSB", "USB", "DSP FIL1", "DSP FIL2", "DSP FIL3", "DSP SHARP", "DSP SOFT",
    "IF FIL1", "IF FIL2", "IF FIL3", "PRE-AMP1", "PRE-AMP2", "ATT 20dB", "BOTH OFF", "AGC FAST", "AGC MID", "AGC SLOW",
    "ANT 1", "ANT 2", "BKIN OFF", "BKIN ON", "BKIN FULL"};
  int ngroup = 8;
  int nbuttons[8] = { 4, 3, 2, 3, 4, 3, 2, 3 };

  for(int i=0;i<ngroup;i++) {
    m_Box10[i].set_orientation(Gtk::ORIENTATION_VERTICAL);
    m_Box10[i].set_border_width(3);
  }

  int index = 0;
  for(int i=0;i<ngroup;i++) {
    for(int j=0;j<nbuttons[i];j++) {
      m_RadioButton[index].set_label(label[index]);
      if(j==0) {
        m_RadioButton[index].set_active();
        m_group[i] = m_RadioButton[index].get_group();
      } else {
        m_RadioButton[index].set_group(m_group[i]);
      }
      m_Box10[i].pack_start(m_RadioButton[index], FALSE, FALSE, 0);
      m_RadioButton[index].signal_clicked().connect(sigc::bind<gint> (mem_fun(*this, &RadioButtons::on_button_clicked9), index));
      index++;
    }
    m_Box1.pack_start(m_Box10[i]);
    if(i != ngroup-1) {
      m_Box1.pack_start(m_VSeparator[i]);
    }
  }

//  add(m_Box_Top);

  m_Box1.set_border_width(10);
  m_Box2.set_border_width(10);
  m_Box2.pack_start(m_Button_Quit);

  pack_start(m_Box1);
  pack_start(m_HSeparator);
  pack_start(m_Box2);
  /*
  m_Box_Top.pack_start(m_Box1);
  m_Box_Top.pack_start(m_HSeparator);
  m_Box_Top.pack_start(m_Box2);
  */

  m_Button_Quit.set_can_default();
  m_Button_Quit.grab_default();
  m_Button_Quit.signal_clicked().connect (sigc::            mem_fun(*this, &RadioButtons::on_button_clicked )    );

  show_all_children();
}

RadioButtons::~RadioButtons()
{
}

void RadioButtons::on_button_clicked()
{
  hide(); //to close the application.
}

void RadioButtons::on_button_clicked9(gint data)
{

  if(m_RadioButton[data].get_active()) {
    std::cout << "button " << data << " is active " << std::endl;
  myfunc(data+1); /*** temporary measure */
  }

}
