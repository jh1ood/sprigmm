#include "mydefine.h"
#include "radiobuttons.h"
#include <cairomm/context.h>
#include <iostream>
#include <string>
#include "drawingarea.h"
using namespace std;

void myfunc (int);

RadioButtons::RadioButtons ():
m_Box_Top (Gtk::ORIENTATION_VERTICAL),
m_Box1 (Gtk::ORIENTATION_HORIZONTAL, 10),
m_Box2 (Gtk::ORIENTATION_VERTICAL, 10)
{

  vector < vector < string >> label
  {
    {
    "CW", "CW-R", "RTTY", "RTTY-R", "LSB", "USB", "AM", "FM"},
    {
    "DSP FIL 1", "DSP FIL 2", "DSP FIL 3"},
    {
    "DSP SHARP", "DSP SOFT"},
    {
    "IF FIL 1", "IF FIL 2", "IF FIL 3"},
    {
    "PRE-AMP OFF", "PRE-AMP 1", "PRE-AMP 2"},
    {
    "ATT OFF", "ATT 20dB"},
    {
    "AGC FAST", "AGC MID", "AGC SLOW"},
    {
    "ANT 1", "ANT 2"},
    {
  "BKIN OFF", "BKIN SEMI", "BKIN FULL"},};

  int index = 0;
  for (unsigned int i = 0; i < label.size (); i++)
    {
      m_BoxGroup[i].set_orientation (Gtk::ORIENTATION_VERTICAL);
      m_BoxGroup[i].set_border_width (3);
      if (i != 0)
	{
	  m_Box1.pack_start (m_VSeparator[i]);
	}
      for (unsigned int j = 0; j < label.at (i).size (); j++)
	{
	  m_RadioButton[index].set_label (label[i][j]);
	  if (j == 0)
	    {
	      m_RadioButton[index].set_active ();
	      m_group[i] = m_RadioButton[index].get_group ();
	    }
	  else
	    {
	      m_RadioButton[index].set_group (m_group[i]);
	    }
	  m_BoxGroup[i].pack_start (m_RadioButton[index], FALSE, FALSE, 0);
	  m_RadioButton[index].signal_clicked ().connect (sigc::bind < gint >
							  (mem_fun
							   (*this,
							    &RadioButtons::
							    on_button_clicked_all),
							   index));
	  index++;
	}
      m_Box1.pack_start (m_BoxGroup[i]);
    }

  m_Box1.set_border_width (10);
  m_Box2.set_border_width (10);
  m_Box_Top.pack_start (m_Box1);
  m_Box_Top.pack_start (m_Box2);
  pack_start (m_Box_Top);

  show_all_children ();

  Glib::signal_timeout().connect( sigc::mem_fun(*this, &RadioButtons::on_timeout), 500 );

}


RadioButtons::~RadioButtons ()
{
}

bool RadioButtons::on_timeout()
{
	  if (operating_mode == 0x03)
	    {				/* CW */
		  m_RadioButton[0].set_active ();
	    }
	  if (operating_mode == 0x07)
	    {				/* CW-R */
		  m_RadioButton[1].set_active ();
	    }
	  if (operating_mode == 0x00)
	    {				/* LSB */
		  m_RadioButton[4].set_active ();
	    }
	  if (operating_mode == 0x01)
	    {				/* USB */
		  m_RadioButton[5].set_active ();
	    }
	  if (dsp_filter == 0x01)
	    {				/* FIL1 */
		  m_RadioButton[8].set_active ();
	    }
	  if (dsp_filter == 0x02)
	    {				/* FIL2 */
		  m_RadioButton[9].set_active ();
	    }
	  if (dsp_filter == 0x03)
	    {				/* FIL3 */
		  m_RadioButton[10].set_active ();
	    }
cout << "RadioButtons::on_timeout() operating_mode = " << operating_mode << ", dsp_filter = " << dsp_filter << endl;

	return true;
}

void
RadioButtons::on_button_clicked_quit ()
{
  hide ();			//to close the application.
}

void
RadioButtons::on_button_clicked_all (int index)
{

  if (m_RadioButton[index].get_active ())
    {
      std::cout << "button " << index << " is active " << std::endl;
      myfunc (index);
		   /*** temporary measure */
    }

}
