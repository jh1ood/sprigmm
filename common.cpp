#include "radiobuttons.h"
#include "drawingarea.h"
#include "Scales.h"
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <iostream>
#include <termios.h>
#include <fcntl.h>
using namespace std;

#define DEBUG
#define NO_MARKER
#define NFFT                    4096
#define WINDOW_XSIZE            1320
#define WINDOW_YSIZE             500
#define AREA1_XSIZE               99
#define AREA1_YSIZE               50
#define WATERFALL_XSIZE          512
#define WATERFALL_YSIZE          768
#define WAVEFORM_LEN             128
#define BAUDRATE                B19200
#define TIMEOUT_VALUE           100
#define END_OF_COMMAND          0xfd

int fd;
static int operating_mode = 3, dsp_filter = 1;

int
rig_init_serial (char *serial_port)
{
  struct termios tio;

  std::cout << "rig_init_serial: begin ... \n";

  fd = open (serial_port, O_RDWR | O_NOCTTY);
  if (fd < 0)
    {
      std::cout << "error: can not open myrig. \n";
      return 1;
    }

  memset (&tio, 0, sizeof (tio));
  tio.c_cflag = CS8 | CLOCAL | CREAD;
  tio.c_cc[VEOL] = 0xfd;	/* IC-7410 postamble */
  tio.c_lflag = 0;		/* non canonical mode */
  tio.c_cc[VTIME] = 0;		/* non canonical mode */
  tio.c_cc[VMIN] = 1;		/* non canonical mode */

  tio.c_iflag = IGNPAR | ICRNL;
  cfsetispeed (&tio, BAUDRATE);
  cfsetospeed (&tio, BAUDRATE);
  tcsetattr (fd, TCSANOW, &tio);

  return 0;
}


int
mystrlen (unsigned char *string)
{
  unsigned char *t;
  for (t = string; *t != END_OF_COMMAND; t++)
    {;
    }
  return (t - string) + 1;	/* +1 to include EOC */
}

int
mystrcmp (unsigned char *string1, unsigned char *string2)
{
  unsigned char *t1;
  unsigned char *t2;

  for (t1 = string1, t2 = string2; *t1 == *t2 && *t1 != END_OF_COMMAND;
       t1++, t2++)
    {;
    }
  return *t1 - *t2;
}

int
myread (unsigned char *myresponse)
{
  int res = 0;
  unsigned char mybuf[256], *p;

  p = myresponse;
  do
    {
      int t = read (fd, mybuf, 1);
      if (t != 1)
	{
	  fprintf (stderr, "error in read \n");
	  return -1;
	}
      *p++ = mybuf[0];
      res++;
    }
  while (mybuf[0] != 0xfd);

  return res;
}

int
receive_fb (void)
{
  unsigned char response[256];
  unsigned char fb_message[6] = { 0xfe, 0xfe, 0xe0, 0x80, 0xfb, 0xfd };
  int n;

  n = myread (response);	/* get echo back */

  if (mystrcmp (response, fb_message) != 0)
    {
      fprintf (stderr, "*** error *** not a FB message. ");
      for (int i = 0; i < n; i++)
	{
	  fprintf (stderr, "%02x ", response[i]);
	}
      fprintf (stderr, "\n");
      return false;
    }

  return true;
}

int
send_command (unsigned char *partial_command)
{
  int n_partial, n_command;
  int n_echoback;
  unsigned char command[256] = { 0xfe, 0xfe, 0x80, 0xe0 };	/* preamble */
  unsigned char echoback[256];

  n_partial = mystrlen (partial_command);
  n_command = n_partial + 4;	/* add preamble(4) */
  for (int i = 0; i < n_partial; i++)
    {
      command[4 + i] = partial_command[i];
    }
  command[n_command - 1] = 0xfd;

  write (fd, command, n_command);
  n_echoback = myread (echoback);

#ifdef DEBUG
  unsigned char *s;
  s = command;
  fprintf (stderr, "send_command: n_command  = %2d, command  = ", n_command);
  for (int i = 0; i < n_command; i++)
    {
      fprintf (stderr, "[%02x] ", *s++);
    }
  fprintf (stderr, "\n");
  fprintf (stderr, "              n_echoback = %2d, echoback = ", n_echoback);
  s = echoback;
  for (int i = 0; i < n_echoback; i++)
    {
      fprintf (stderr, "[%02x] ", *s++);
    }
  fprintf (stderr, "\n");
#endif

  if ((n_echoback != n_command) || (mystrcmp (command, echoback) != 0))
    {
      fprintf (stderr,
	       "              *** error *** echoback does not much. \n");
      return false;
    }

  return true;
}

int
send_commandx (const vector < unsigned char >&partial_command)
{
  int n_partial, n_command;
  int n_echoback;
  unsigned char command[256] = { 0xfe, 0xfe, 0x80, 0xe0 };	/* preamble */
  unsigned char echoback[256];

  n_partial = partial_command.size ();
  cout << "send_commandx: n_partial = " << n_partial << endl;

  n_command = 4 + n_partial + 1;	/* add preamble(4) and EOC(1) */
  for (int i = 0; i < n_partial; i++)
    {
      command[4 + i] = partial_command.at (i);
    }
  command[n_command - 1] = 0xfd;	/* End_Of_Command */

  write (fd, command, n_command);
  n_echoback = myread (echoback);

#ifdef DEBUG
  unsigned char *s;
  s = command;
  fprintf (stderr, "send_command: n_command  = %2d, command  = ", n_command);
  for (int i = 0; i < n_command; i++)
    {
      fprintf (stderr, "[%02x] ", *s++);
    }
  fprintf (stderr, "\n");
  fprintf (stderr, "              n_echoback = %2d, echoback = ", n_echoback);
  s = echoback;
  for (int i = 0; i < n_echoback; i++)
    {
      fprintf (stderr, "[%02x] ", *s++);
    }
  fprintf (stderr, "\n");
#endif

  if ((n_echoback != n_command) || (mystrcmp (command, echoback) != 0))
    {
      fprintf (stderr,
	       "              *** error *** echoback does not much. \n");
      return false;
    }

  return true;
}

void
set_operating_mode (void)
{
  static unsigned char command1[4] = { 0x06, 0x03, 0x01, 0xfd };

  command1[1] = operating_mode;
  command1[2] = dsp_filter;;
  send_command (command1);
  receive_fb ();
}

void
set_operating_modex (void)
{
  vector < unsigned char >command1 = { 0x06, 0x03, 0x01 };	/* without EOC */

  command1[1] = operating_mode;
  command1[2] = dsp_filter;
  send_commandx (command1);
  receive_fb ();
}

void
myfunc (int index)
{
  static vector < unsigned char >command1x = { 0x06, 0x03, 0x01 };	/* OP MODE & FIL SET */
  static vector < unsigned char >command6x = { 0x16, 0x55, 0x00 };	/* IF FIL1/2/3 */
  static vector < unsigned char >command19x = { 0x16, 0x56, 0x00 };	/* DSP SHARP/SOFT */
  static vector < unsigned char >command9x = { 0x16, 0x02, 0x00 };	/* PRE-AMP OFF/1/2 */
  static vector < unsigned char >command12x = { 0x11, 0x00 };	/* ATT OFF  */
  static vector < unsigned char >command16x = { 0x16, 0x12, 0x01 };	/* AGC FAST  */
  static vector < unsigned char >command14x = { 0x12, 0x00 };	/* ANT 1/2 */
  static vector < unsigned char >command3x = { 0x16, 0x47, 0x00 };	/* BKIN OFF/SEMI/FULL  */

  std::cout << "myfunc: index = " << index << std::endl;
  if (index >= 0 && index <= 7)
    {				/* OP MODE */
      switch (index)
	{
	case 0:		/* CW */
	  operating_mode = 0x03;
	  break;
	case 1:		/* CW-R */
	  operating_mode = 0x07;
	  break;
	case 2:		/* RTTY */
	  operating_mode = 0x04;
	  break;
	case 3:		/* RTTY-R */
	  operating_mode = 0x08;
	  break;
	case 4:		/* LSB */
	  operating_mode = 0x00;
	  break;
	case 5:		/* USB */
	  operating_mode = 0x01;
	  break;
	case 6:		/* AM */
	  operating_mode = 0x02;
	  break;
	case 7:		/* FM */
	  operating_mode = 0x05;
	  break;
	}
      command1x[1] = operating_mode;
      send_commandx (command1x);
      receive_fb ();
      return;
    }

  if (index >= 8 && index <= 10)
    {				/* DAP SHARP/SOFT */
      command1x[2] = (index - 8) + 1;	/* start from 1 */
      send_commandx (command1x);
      receive_fb ();
      return;
    }

  if (index >= 11 && index <= 12)
    {				/* DAP SHARP/SOFT */
      command19x[2] = index - 11;
      send_commandx (command19x);
      receive_fb ();
      return;
    }

  if (index >= 13 && index <= 15)
    {				/* IF FIL 1/2/3 */
      command6x[2] = index - 13;
      send_commandx (command6x);
      receive_fb ();
      return;
    }

  if (index >= 16 && index <= 18)
    {				/* PRE-AMP OFF/1/2 */
      command9x[2] = index - 16;
      send_commandx (command9x);
      receive_fb ();
      return;
    }

  if (index >= 19 && index <= 20)
    {				/* PRE-AMP OFF/1/2 */
      command12x[1] = (index - 19) * 0x20;
      send_commandx (command12x);
      receive_fb ();
      return;
    }

  if (index >= 21 && index <= 23)
    {				/* AGC F/M/S */
      command16x[2] = (index - 21) + 1;	/* start from 1 */
      send_commandx (command16x);
      receive_fb ();
      return;
    }

  if (index >= 24 && index <= 25)
    {				/* ANT 1/2 */
      command14x[1] = index - 24;
      send_commandx (command14x);
      receive_fb ();
      return;
    }

  if (index >= 26 && index <= 28)
    {				/* BKIN OFF/SEMI/FULL */
      command3x[2] = index - 26;
      send_commandx (command3x);
      receive_fb ();
      return;
    }

}
