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

  static unsigned char command3 [4] = { 0x16, 0x47, 0x00, 0xfd };       /* BKIN OFF  */
  static unsigned char command4 [4] = { 0x16, 0x47, 0x01, 0xfd };       /* BKIN ON   */
  static unsigned char command5 [4] = { 0x16, 0x47, 0x02, 0xfd };       /* BKIN FULL */
  static unsigned char command6 [4] = { 0x16, 0x55, 0x00, 0xfd };       /* IF FIL1 */
//static unsigned char command7 [4] = { 0x16, 0x55, 0x01, 0xfd };       /* if fil2 */
  static unsigned char command8 [4] = { 0x16, 0x55, 0x02, 0xfd };       /* IF FIL3 */
  static unsigned char command9 [4] = { 0x16, 0x02, 0x00, 0xfd };       /* PRE-AMP OFF */
  static unsigned char command10[4] = { 0x16, 0x02, 0x01, 0xfd };       /* PRE-AMP 1  */
  static unsigned char command11[4] = { 0x16, 0x02, 0x02, 0xfd };       /* PRE-AMP 2  */
  static unsigned char command12[3] = { 0x11, 0x00, 0xfd };             /* ATT OFF  */
  static unsigned char command13[3] = { 0x11, 0x20, 0xfd };             /* ATT 20dB */
  static unsigned char command14[3] = { 0x12, 0x00, 0xfd };             /* ANT 1 */
  static unsigned char command15[3] = { 0x12, 0x01, 0xfd };             /* ANT 2 */
  static unsigned char command16[4] = { 0x16, 0x12, 0x01, 0xfd };       /* AGC FAST  */
  static unsigned char command17[4] = { 0x16, 0x12, 0x02, 0xfd };       /* AGC FAST  */
  static unsigned char command18[4] = { 0x16, 0x12, 0x03, 0xfd };       /* AGC SLOW  */
  static unsigned char command19[4] = { 0x16, 0x56, 0x00, 0xfd };       /* DSP SHARP */
  static unsigned char command20[4] = { 0x16, 0x56, 0x01, 0xfd };       /* DSP SOFT */
  static vector<unsigned char> command19x = { 0x16, 0x56, 0x00, 0xfd };       /* DSP SHARP */
  static vector<unsigned char> command20x = { 0x16, 0x56, 0x01, 0xfd };       /* DSP SOFT */

int fd;
int operating_mode = 3, dsp_filter = 1;


int rig_init_serial (char* serial_port) {
  struct termios tio;

  std::cout << "rig_init_serial: begin ... \n";

  fd = open (serial_port, O_RDWR | O_NOCTTY);
  if (fd < 0) {
    std::cout << "error: can not open myrig. \n";
    return 1;
  }

  memset (&tio, 0, sizeof (tio));
  tio.c_cflag     = CS8 | CLOCAL | CREAD;
  tio.c_cc[VEOL]  = 0xfd;               /* IC-7410 postamble */
  tio.c_lflag     = 0;          /* non canonical mode */
  tio.c_cc[VTIME] = 0;          /* non canonical mode */
  tio.c_cc[VMIN]  = 1;          /* non canonical mode */

  tio.c_iflag     = IGNPAR | ICRNL;
  cfsetispeed (&tio, BAUDRATE);
  cfsetospeed (&tio, BAUDRATE);
  tcsetattr (fd, TCSANOW, &tio);

  return 0;
}


int mystrlen (unsigned char *string) {
  unsigned char *t;
  for (t = string; *t != END_OF_COMMAND; t++) { ; }
  return (t - string) + 1;  /* +1 to include EOC */
}

int mystrcmp (unsigned char *string1, unsigned char *string2) {
  unsigned char *t1;
  unsigned char *t2;

  for (t1 = string1, t2 = string2; *t1 == *t2 && *t1 != END_OF_COMMAND; t1++, t2++) { ; }
  return *t1 - *t2;
}

int myread( unsigned char *myresponse ) {
  int res = 0;
  unsigned char mybuf[256], *p;

  p = myresponse;
  do {
    int t = read(fd, mybuf, 1);
  if(t != 1) {
   fprintf(stderr, "error in read \n");
   return -1;
    }
  *p++ = mybuf[0];
  res++;
  } while (mybuf[0] != 0xfd);

 return res;
}

int receive_fb (void) {
  unsigned char response[256];
  unsigned char fb_message[6] = { 0xfe, 0xfe, 0xe0, 0x80, 0xfb, 0xfd };
  int n;

  n = myread (response); /* get echo back */

  if (mystrcmp (response, fb_message) != 0) {
    fprintf(stderr, "*** error *** not a FB message. ");
    for (int i = 0; i < n; i++) {
      fprintf(stderr, "%02x ", response[i]);
    }
    fprintf(stderr, "\n");
    return false;
  }

  return true;
}

int send_command (unsigned char *partial_command) {
  int n_partial, n_command;
  int n_echoback;
  unsigned char command [256] = {0xfe, 0xfe, 0x80, 0xe0}; /* preamble */
  unsigned char echoback[256];

  n_partial = mystrlen (partial_command);
  n_command = n_partial + 4; /* add preamble(4) */
  for(int i=0;i<n_partial;i++) {
    command[4+i] = partial_command[i];
  }
  command[n_command-1] = 0xfd;

  write (fd, command, n_command);
  n_echoback = myread(echoback);

#ifdef DEBUG
  unsigned char *s;
  s = command;
  fprintf(stderr, "send_command: n_command  = %2d, command  = ", n_command);
  for (int i = 0; i < n_command; i++) {
        fprintf(stderr, "[%02x] ", *s++);
  }
  fprintf(stderr, "\n");
  fprintf(stderr, "              n_echoback = %2d, echoback = ", n_echoback);
  s = echoback;
  for (int i = 0; i < n_echoback; i++) {
        fprintf(stderr, "[%02x] ", *s++);
  }
  fprintf(stderr, "\n");
#endif

  if ((n_echoback != n_command) || (mystrcmp (command, echoback) != 0)) {
    fprintf(stderr, "              *** error *** echoback does not much. \n");
    return false;
  }

  return true;
}

int send_commandx (vector<unsigned char> &partial_command) {
  int n_partial, n_command;
  int n_echoback;
  unsigned char command [256] = {0xfe, 0xfe, 0x80, 0xe0}; /* preamble */
  unsigned char echoback[256];

  n_partial = partial_command.size();
  cout << "send_commandx: n_partial = " << n_partial << endl;

  n_command = 4 + n_partial; /* add preamble(4) */
  for(int i=0;i<n_partial;i++) {
    command[4+i] = partial_command.at(i);
  }
  command[n_command-1] = 0xfd;

  write (fd, command, n_command);
  n_echoback = myread(echoback);

#ifdef DEBUG
  unsigned char *s;
  s = command;
  fprintf(stderr, "send_command: n_command  = %2d, command  = ", n_command);
  for (int i = 0; i < n_command; i++) {
        fprintf(stderr, "[%02x] ", *s++);
  }
  fprintf(stderr, "\n");
  fprintf(stderr, "              n_echoback = %2d, echoback = ", n_echoback);
  s = echoback;
  for (int i = 0; i < n_echoback; i++) {
        fprintf(stderr, "[%02x] ", *s++);
  }
  fprintf(stderr, "\n");
#endif

  if ((n_echoback != n_command) || (mystrcmp (command, echoback) != 0)) {
    fprintf(stderr, "              *** error *** echoback does not much. \n");
    return false;
  }

  return true;
}

void set_operating_mode (void)
{
  static unsigned char command1[4] = { 0x06, 0x03, 0x01, 0xfd };

  command1[1] = operating_mode;
  command1[2] = dsp_filter;;
  send_command (command1);
  receive_fb ();
}

void set_operating_modex (void)
{
  vector<unsigned char> command1 = { 0x06, 0x03, 0x01, 0xfd };

  command1[1] = operating_mode;
  command1[2] = dsp_filter;
  send_commandx (command1);
  receive_fb ();
}

void myfunc (int index) {
  std::cout << "myfunc: " << index << std::endl;
  if(index >= 1 && index <= 7) {
    switch (index) {
      case 1:
      operating_mode = 0x03;
      break;
      case 2:
      operating_mode = 0x07;
      break;
      case 3:
      operating_mode = 0x00;
      break;
      case 4:
      operating_mode = 0x01;
      break;
      case 5:
      dsp_filter     = 0x01;
      break;
      case 6:
      dsp_filter     = 0x02;
      break;
      case 7:
      dsp_filter     = 0x03;
      break;
    }
    set_operating_modex();
  }

  if(index >= 8 && index <= 9) {
    switch (index) {
      case 8:
    send_commandx(command19x);
    receive_fb();
    break;
      case 9:
    send_commandx(command20x);
    receive_fb();
    break;
  }
  }
}

