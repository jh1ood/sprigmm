#include "radiobuttons.h"
#include "drawingarea.h"
#include "MyWindow.h"
#include "Scales.h"
#include <iostream>
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include <asoundlib.h>
#include <fftw3.h>

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

int fd = -1;
unsigned int rate = 32000;	/* stream rate */
unsigned int channels = 1;	/* count of channels */
int byte_per_sample = 2;	/* 16 bit format */
unsigned int buffer_time = 500000;	/* ring buffer length in us */
unsigned int period_time = 128000;	/* period time in us */
int resample = 0;	/* disable resample */
int period_event = 0;	/* produce poll event after each period */
double audio_signal[NFFT];
double audio_signal_ffted[NFFT];
double fft_window[NFFT];
int cw_pitch = 600;
int iwater = 0;
int nsamples;
double bin_size, waterfall_scale_x;
double amax = 14.0, amin = 7.0;
long int ifreq_in_hz = 7026000;
int s_meter;
int operating_mode=3;	/* CW=03, CW-REV=07, LSB=00, USB=01 */
int dsp_filter=1;		/* FIL1=01, FIL2=02, FIL3=03 */
snd_pcm_sframes_t buffer_size;
snd_pcm_sframes_t period_size;
snd_pcm_t *handle;
snd_pcm_hw_params_t *hwparams;
snd_pcm_sw_params_t *swparams;
double *in;
fftw_complex *out;
fftw_plan p;

void rig_init_serial (char *);
void rig_init_sound  (char *);

int
main (int argc, char *argv[])
{
  if (argc != 3)
    {
      std::cout << "Usage example: " << argv[0] << " /dev/ttyUSB0 hw:2,0 \n";
      std::
	cout <<
	" --> try % ls -l /dev/ttyUSB*, and % arecord -l to know these parameters.\n";
      return false;
    }
  std::
    cout << "serial_port = " << argv[1] << ", sound_device = " << argv[2] <<
    std::endl;
  rig_init_serial (argv[1]);
  rig_init_sound  (argv[2]);

  argc = 1;			/* just for the next line */
  Glib::RefPtr < Gtk::Application > app =
    Gtk::Application::create (argc, argv, "org.gtkmm.example");

  MyWindow win;
  win.set_title ("IC-7410 Rig Control Program (C++ version)");
  win.set_default_size (50, 50);
  win.set_border_width (5);
  win.show_all_children ();

  return app->run (win);
}
