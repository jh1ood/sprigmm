/* common2.cpp */
#define NO_DEBUG
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
#define M_PI                    3.141592653589793

#include <iostream>
#include <asoundlib.h>
#include <fftw3.h>

void send_command (unsigned char *partial_command);
void receive_fb ();


static unsigned int rate = 32000;	/* stream rate */
static unsigned int channels = 1;	/* count of channels */
static int byte_per_sample = 2;	/* 16 bit format */
static unsigned int buffer_time = 500000;	/* ring buffer length in us */
static unsigned int period_time = 128000;	/* period time in us */
static int resample = 0;	/* disable resample */
static int period_event = 0;	/* produce poll event after each period */
static double audio_signal[NFFT];
static double audio_signal_ffted[NFFT];
static double fft_window[NFFT];
static int cw_pitch = 600;
static int iwater = 0;
static int nsamples;
static double bin_size, waterfall_scale_x;
static double amax = 14.0, amin = 7.0;

long int ifreq_in_hz = 7026000;
int s_meter;
extern int operating_mode;	/* CW=03, CW-REV=07, LSB=00, USB=01 */
extern int dsp_filter;		/* FIL1=01, FIL2=02, FIL3=03 */
// int fd = -1;

double *in;
fftw_complex *out;
fftw_plan p;

static snd_pcm_sframes_t buffer_size;
static snd_pcm_sframes_t period_size;

void set_freq (long int ifreq_in_hz);

static int
set_hwparams (snd_pcm_t * handle, snd_pcm_hw_params_t * params)
{
  unsigned int rrate;
  snd_pcm_uframes_t size;
  int err, dir;
  std::cout << "set_hwparams:    begin... \n";

  /* choose all parameters */
  err = snd_pcm_hw_params_any (handle, params);
  if (err < 0)
    {
      fprintf (stderr,
	       "Broken configuration for playback: no configurations available: %s\n",
	       snd_strerror (err));
      return err;
    }

  /* set hardware resampling disabled */
  err = snd_pcm_hw_params_set_rate_resample (handle, params, resample);
  if (err < 0)
    {
      fprintf (stderr, "Resampling setup failed for playback: %s\n",
	       snd_strerror (err));
      return err;
    }

  /* set the interleaved read/write format */
  err =
    snd_pcm_hw_params_set_access (handle, params,
				  SND_PCM_ACCESS_RW_INTERLEAVED);
  if (err < 0)
    {
      fprintf (stderr, "Access type not available for playback: %s\n",
	       snd_strerror (err));
      return err;
    }

  /* set the sample format */
  err = snd_pcm_hw_params_set_format (handle, params, SND_PCM_FORMAT_S16);
  if (err < 0)
    {
      fprintf (stderr, "Sample format not available for playback: %s\n",
	       snd_strerror (err));
      return err;
    }

  /* set the count of channels */
  err = snd_pcm_hw_params_set_channels (handle, params, channels);
  if (err < 0)
    {
      fprintf (stderr,
	       "Channels count (%i) not available for playbacks: %s\n",
	       channels, snd_strerror (err));
      return err;
    }

  /* set the stream rate */
  rrate = rate;
  err = snd_pcm_hw_params_set_rate_near (handle, params, &rrate, 0);
  if (err < 0)
    {
      fprintf (stderr, "Rate %iHz not available for playback: %s\n", rate,
	       snd_strerror (err));
      return err;
    }
  if (rrate != rate)
    {
      fprintf (stderr, "Rate doesn't match (requested %iHz, get %iHz)\n",
	       rate, err);
      return -EINVAL;
    }

  /* set the buffer time */
  err =
    snd_pcm_hw_params_set_buffer_time_near (handle, params, &buffer_time,
					    &dir);
  if (err < 0)
    {
      fprintf (stderr, "Unable to set buffer time %i for playback: %s\n",
	       buffer_time, snd_strerror (err));
      return err;
    }
  fprintf (stderr, "buffer_time = %8d, dir   = %d \n", buffer_time, dir);

  err = snd_pcm_hw_params_get_buffer_size (params, &size);
  if (err < 0)
    {
      fprintf (stderr, "Unable to get buffer size for playback: %s\n",
	       snd_strerror (err));
      return err;
    }
  buffer_size = size;
  fprintf (stderr, "buffer_size = %8d             \n", (int) buffer_size);

  /* set the period time */
  err =
    snd_pcm_hw_params_set_period_time_near (handle, params, &period_time,
					    &dir);
  if (err < 0)
    {
      fprintf (stderr, "Unable to set period time %i for playback: %s\n",
	       period_time, snd_strerror (err));
      return err;
    }
  fprintf (stderr, "period_time = %8d, dir   = %d \n", period_time, dir);

  err = snd_pcm_hw_params_get_period_size (params, &size, &dir);
  if (err < 0)
    {
      fprintf (stderr, "Unable to get period size for playback: %s\n",
	       snd_strerror (err));
      return err;
    }
  period_size = size;
  fprintf (stderr, "period_size = %8d, dir   = %d \n", (int) period_size,
	   dir);

  if (period_size < NFFT)
    {
      fprintf (stderr,
	       "error: period_size = %8d, but less than NFFT  = %d \n",
	       (int) period_size, NFFT);
      exit (1);
    }

  /* write the parameters to device */
  err = snd_pcm_hw_params (handle, params);
  if (err < 0)
    {
      fprintf (stderr, "Unable to set hw params for playback: %s\n",
	       snd_strerror (err));
      return err;
    }

  return 0;
}

static int
set_swparams (snd_pcm_t * handle, snd_pcm_sw_params_t * swparams)
{
  int err;

  /* get the current swparams */
  err = snd_pcm_sw_params_current (handle, swparams);
  if (err < 0)
    {
      fprintf (stderr,
	       "Unable to determine current swparams for playback: %s\n",
	       snd_strerror (err));
      return err;
    }

  /* start the transfer when the buffer is almost full: */
  /* (buffer_size / avail_min) * avail_min */
  err =
    snd_pcm_sw_params_set_start_threshold (handle, swparams,
					   (buffer_size / period_size) *
					   period_size);
  if (err < 0)
    {
      fprintf (stderr,
	       "Unable to set start threshold mode for playback: %s\n",
	       snd_strerror (err));
      return err;
    }

  /* allow the transfer when at least period_size samples can be processed */
  /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
  err =
    snd_pcm_sw_params_set_avail_min (handle, swparams,
				     period_event ? buffer_size :
				     period_size);
  if (err < 0)
    {
      fprintf (stderr, "Unable to set avail min for playback: %s\n",
	       snd_strerror (err));
      return err;
    }

  /* enable period events when requested */
  if (period_event)
    {
      err = snd_pcm_sw_params_set_period_event (handle, swparams, 1);
      if (err < 0)
	{
	  fprintf (stderr, "Unable to set period event: %s\n",
		   snd_strerror (err));
	  return err;
	}
    }

  /* write the parameters to the playback device */
  err = snd_pcm_sw_params (handle, swparams);
  if (err < 0)
    {
      fprintf (stderr, "Unable to set sw params for playback: %s\n",
	       snd_strerror (err));
      return err;
    }
  return 0;
}

int
rig_init_sound (char *sound_device)
{

  snd_pcm_t *handle;
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  int err = 0;

  std::cout << "rig_init_sound:  begin ... \n";

  snd_pcm_hw_params_alloca (&hwparams);
  snd_pcm_sw_params_alloca (&swparams);

  std::cout << " rate = " << rate << " Hz, " << channels << " channel(s). \n";

  if ((err =
       snd_pcm_open (&handle, sound_device, SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
      fprintf (stderr, "Capture open error: %s\n", snd_strerror (err));
      return 0;
    }

  if ((err = set_hwparams (handle, hwparams)) < 0)
    {
      fprintf (stderr, "Setting of hwparams failed: %s\n",
	       snd_strerror (err));
      exit (EXIT_FAILURE);
    }

  if ((err = set_swparams (handle, swparams)) < 0)
    {
      fprintf (stderr, "Setting of swparams failed: %s\n",
	       snd_strerror (err));
      exit (EXIT_FAILURE);
    }

  return 0;
}

void
set_cw_speed (int wpm)
{
  static unsigned char command1[5] = { 0x14, 0x0c, 0x00, 0x32, 0xfd };
  int iii, i100, i10, i1;

  if (wpm < 6)
    wpm = 6;
  if (wpm > 48)
    wpm = 48;
  iii = 255 * (wpm - 6) / (48 - 6);
  i100 = iii / 100;
  i10 = (iii - 100 * i100) / 10;
  i1 = iii % 10;
//  fprintf(stderr, "wpm changed %d %d %d %d \n", wpm, i100, i10, i1);
  command1[2] = i100;
  command1[3] = 16 * i10 + i1;
  send_command (command1);
  receive_fb ();
}

void
set_tx_power (int txp)
{
  static unsigned char command1[5] = { 0x14, 0x0a, 0x00, 0x32, 0xfd };
  int iii, i100, i10, i1;

  if (txp < 2)
    txp = 2;
  if (txp > 100)
    txp = 100;
  iii = 255.0 * (txp - 2) / 100.0;
  i100 = iii / 100;
  i10 = (iii - 100 * i100) / 10;
  i1 = iii % 10;
//  fprintf(stderr, "txpower changed %d %d %d %d \n", txpower, i100, i10, i1);
  command1[2] = i100;
  command1[3] = 16 * i10 + i1;
  send_command (command1);
  receive_fb ();
}


#if 0

#define NO_DEBUG
#define NO_MARKER
#define NFFT                    4096
#define WINDOW_XSIZE            1320
#define WINDOW_YSIZE             500
#define AREA1_XSIZE               99void
set_tx_power (int txpower)
{
  static unsigned char command1[5] = { 0x14, 0x0a, 0x00, 0x32, 0xfd };
  int iii, i100, i10, i1;

  if (txpower < 2)
    txpower = 2;
  if (txpower > 100)
    txpower = 100;
  iii = 255.0 * (txpower - 2) / 100.0;
  i100 = iii / 100;
  i10 = (iii - 100 * i100) / 10;
  i1 = iii % 10;
//  fprintf(stderr, "txpower changed %d %d %d %d \n", txpower, i100, i10, i1);
  command1[2] = i100;
  command1[3] = 16 * i10 + i1;
  send_command (command1);
  receive_fb ();
}

#define AREA1_YSIZE               50
#define WATERFALL_XSIZE          512
#define WATERFALL_YSIZE          768
#define WAVEFORM_LEN             128
#define BAUDRATE                B19200
#define TIMEOUT_VALUE           100
#define END_OF_COMMAND          0xfd
#define M_PI                    3.141592653589793

#include "asoundlib.h"
#include <cairo.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <gtk/gtk.h>
#include <math.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fftw3.h>
#include <complex.h>
void
set_tx_power (int txpower)
{
  static unsigned char command1[5] = { 0x14, 0x0a, 0x00, 0x32, 0xfd };
  int iii, i100, i10, i1;

  if (txpower < 2)
    txpower = 2;
  if (txpower > 100)
    txpower = 100;
  iii = 255.0 * (txpower - 2) / 100.0;
  i100 = iii / 100;
  i10 = (iii - 100 * i100) / 10;
  i1 = iii % 10;
//  fprintf(stderr, "txpower changed %d %d %d %d \n", txpower, i100, i10, i1);
  command1[2] = i100;
  command1[3] = 16 * i10 + i1;
  send_command (command1);
  receive_fb ();
}

static char myrig[256] = "/dev/ttyUSB0";
static char device[256] = "hw:2,0";	/* sound capture device */
static unsigned int rate = 32000;	/* stream rate */
static unsigned int channels = 1;	/* count of channels */
static int byte_per_sample = 2;	/* 16 bit format */
static unsigned int buffer_time = 500000;	/* ring buffer length in us */
static unsigned int period_time = 128000;	/* period time in us */
static int resample = 0;	/* disable resample */
static int period_event = 0;	/* produce poll event after each period */
static double audio_signal[NFFT];
static double audio_signal_ffted[NFFT];
static double fft_window[NFFT];
static int cw_pitch = 600;
static int iwater = 0;
static int nsamples;
static double bin_size, waterfall_scale_x;
static double amax = 14.0, amin = 7.0;

long int ifreq_in_hz = 7026000;
int s_meter;
int operating_mode = 3;		/* CW=03, CW-REV=07, LSB=00, USB=01 */
int dsp_filter = 1;		/* FIL1=01, FIL2=02, FIL3=03 */
int fd = -1;

GtkWidget *window1, *window2;
GtkWidget *button2dim[100][100];

double *in;
fftw_complex *out;
fftw_plan p;

static snd_pcm_sframes_t buffer_size;
static snd_pcm_sframes_t period_size;

void set_freq (long int ifreq_in_hz);
gboolean timeout (gpointer data);
gboolean timeout2 (gpointer data);

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

/*
  fprintf(stderr, "res=%4d \n",res);
  for(int i=0;i<res;i++) {
    fprintf(stderr, "myresponse[%2d]=[%02x] \n", i, myresponse[i]);
  }
*/
  return res;
}

int
colormap_r (double tmp)
{
  double val;
  if (tmp < 0.50)
    {
      val = 0.0;
    }
  else if (tmp > 0.75)
    {
      val = 1.0;
    }
  else
    {
      val = 4.0 * tmp - 2.0;
    }
  return (int) (255.0 * val);
}

int
colormap_g (double tmp)
{
  double val;
  if (tmp < 0.25)
    {
      val = 4.0 * tmp;
    }
  else if (tmp > 0.75)
    {
      val = -4.0 * tmp + 4.0;
    }
  else
    {
      val = 1.0;
    }
  return (int) (255.0 * val);
}

int
colormap_b (double tmp)
{
  double val;
  if (tmp < 0.25)
    {
      val = 1.0;
    }
  else if (tmp > 0.50)
    {
      val = 0.0;
    }
  else
    {
      val = -4.0 * tmp + 2.0;
    }
  return (int) (255.0 * val);
}

static gint
cb_expose2 (GtkWidget * widget, GdkEventExpose * event, gpointer user_data)
{
  GdkPixbuf *pixbuf = (GdkPixbuf *) user_data;
  GdkPixbuf *background;
  GdkPixmap *pixmap;
  int w, h, n, rowstride;
  guchar *p, *q;

  w = gdk_pixbuf_get_width (pixbuf);
  h = gdk_pixbuf_get_height (pixbuf);
  n = gdk_pixbuf_get_n_channels (pixbuf);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  if (iwater < 0 || n != 3 || rowstride != 3 * w)
    {
      fprintf (stderr, "cb_expse: w=%d, h=%d, n=%d, rowstride=%d \n", w, h, n,
	       rowstride);
    }

  p = gdk_pixbuf_get_pixels (pixbuf) + (iwater) * rowstride;
  q = gdk_pixbuf_get_pixels (pixbuf) + (iwater + WATERFALL_YSIZE) * rowstride;
  for (int j = 0; j < WATERFALL_XSIZE; j++)
    {
      double tmp = audio_signal_ffted[j];
      *p++ = *q++ = colormap_r (tmp);
      *p++ = *q++ = colormap_g (tmp);
      *p++ = *q++ = colormap_b (tmp);
    }

  background =
    gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, WATERFALL_XSIZE,
		    WATERFALL_YSIZE);

  double scale_x, scale_y, offset_x, offset_y;
  int dest_x, dest_y, dest_width, dest_height;

  if (operating_mode == 0x03 || operating_mode == 0x07)
    {
      waterfall_scale_x = 2.0;
    }void
	set_tx_power (int txpower)
	{
	  static unsigned char command1[5] = { 0x14, 0x0a, 0x00, 0x32, 0xfd };
	  int iii, i100, i10, i1;

	  if (txpower < 2)
	    txpower = 2;
	  if (txpower > 100)
	    txpower = 100;
	  iii = 255.0 * (txpower - 2) / 100.0;
	  i100 = iii / 100;
	  i10 = (iii - 100 * i100) / 10;
	  i1 = iii % 10;
	//  fprintf(stderr, "txpower changed %d %d %d %d \n", txpower, i100, i10, i1);
	  command1[2] = i100;
	  command1[3] = 16 * i10 + i1;
	  send_command (command1);
	  receive_fb ();
	}

  else if (operating_mode == 0x00 || operating_mode == 0x01)
    {
      waterfall_scale_x = 1.0;
    }
  else
    {
      waterfall_scale_x = 1.0;
    }
  scale_x = waterfall_scale_x;
  scale_y = 1.0;
  offset_x = 0;
  offset_y = -(iwater + 1);
  dest_x = 0;
  dest_y = 0;
  dest_width = w;
  dest_height = WATERFALL_YSIZE;
  gdk_pixbuf_composite (pixbuf, background, dest_x, dest_y, dest_width,
			dest_height, offset_x, offset_y, scale_x, scale_y,
			GDK_INTERP_BILINEAR, 255);

  iwater++;
  if (iwater >= WATERFALL_YSIZE)
    iwater = 0;

  gdk_pixbuf_render_pixmap_and_mask (background, &pixmap, NULL, 255);
  gdk_window_set_back_pixmap (widget->window, pixmap, FALSE);
  gdk_window_clear (widget->window);

  g_object_unref (background);
  g_object_unref (pixmap);

  return TRUE;
}

static int
set_hwparams (snd_pcm_t * handle, snd_pcm_hw_params_t * params)
{
  unsigned int rrate;
  snd_pcm_uframes_t size;
  int err, dir;

  /* choose all parameters */
  err = snd_pcm_hw_params_any (handle, params);
  if (err < 0)
    {
      fprintf (stderr,
	       "Broken configuration for playback: no configurations available: %s\n",
	       snd_strerror (err));
      return err;
    }

  /* set hardware resampling disabled */
  err = snd_pcm_hw_params_set_rate_resample (handle, params, resample);
  if (err < 0)
    {
      fprintf (stderr, "Resampling setup failed for playback: %s\n",
	       snd_strerror (err));
      return err;
    }

  /* set the interleaved read/write format */
  err =
    snd_pcm_hw_params_set_access (handle, params,
				  SND_PCM_ACCESS_RW_INTERLEAVED);
  if (err < 0)
    {
      fprintf (stderr, "Access type not available for playback: %s\n",
	       snd_strerror (err));
      return err;
    }

  /* set the sample format */
  err = snd_pcm_hw_params_set_format (handle, params, SND_PCM_FORMAT_S16);
  if (err < 0)
    {
      fprintf (stderr, "Sample format not available for playback: %s\n",
	       snd_strerror (err));
      return err;
    }

  /* set the count of channels */
  err = snd_pcm_hw_params_set_channels (handle, params, channels);
  if (err < 0)
    {
      fprintf (stderr,
	       "Channels count (%i) not available for playbacks: %s\n",
	       channels, snd_strerror (err));
      return err;
    }

  /* set the stream rate */
  rrate = rate;
  err = snd_pcm_hw_params_set_rate_near (handle, params, &rrate, 0);
  if (err < 0)
    {
      fprintf (stderr, "Rate %iHz not available for playback: %s\n", rate,
	       snd_strerror (err));
      return err;
    }
  if (rrate != rate)
    {
      fprintf (stderr, "Rate doesn't match (requested %iHz, get %iHz)\n",
	       rate, err);
      return -EINVAL;
    }

  /* set the buffer time */
  err =
    snd_pcm_hw_params_set_buffer_time_near (handle, params, &buffer_time,
					    &dir);
  if (err < 0)void
		  set_tx_power (int txpower)
		  {
		    static unsigned char command1[5] = { 0x14, 0x0a, 0x00, 0x32, 0xfd };
		    int iii, i100, i10, i1;

		    if (txpower < 2)
		      txpower = 2;
		    if (txpower > 100)
		      txpower = 100;
		    iii = 255.0 * (txpower - 2) / 100.0;
		    i100 = iii / 100;
		    i10 = (iii - 100 * i100) / 10;
		    i1 = iii % 10;
		  //  fprintf(stderr, "txpower changed %d %d %d %d \n", txpower, i100, i10, i1);
		    command1[2] = i100;
		    command1[3] = 16 * i10 + i1;
		    send_command (command1);
		    receive_fb ();
		  }

    {
      fprintf (stderr, "Unable to set buffer time %i for playback: %s\n",
	       buffer_time, snd_strerror (err));
      return err;
    }
  fprintf (stderr, "buffer_time = %8d, dir   = %d \n", buffer_time, dir);

  err = snd_pcm_hw_params_get_buffer_size (params, &size);
  if (err < 0)
    {
      fprintf (stderr, "Unable to get buffer size for playback: %s\n",
	       snd_strerror (err));
      return err;
    }
  buffer_size = size;
  fprintf (stderr, "buffer_size = %8d             \n", (int) buffer_size);

  /* set the period time */
  err =
    snd_pcm_hw_params_set_period_time_near (handle, params, &period_time,
					    &dir);
  if (err < 0)
    {
      fprintf (stderr, "Unable to set period time %i for playback: %s\n",
	       period_time, snd_strerror (err));
      return err;void
      set_tx_power (int txpower)
      {
        static unsigned char command1[5] = { 0x14, 0x0a, 0x00, 0x32, 0xfd };
        int iii, i100, i10, i1;

        if (txpower < 2)
          txpower = 2;
        if (txpower > 100)
          txpower = 100;
        iii = 255.0 * (txpower - 2) / 100.0;
        i100 = iii / 100;
        i10 = (iii - 100 * i100) / 10;
        i1 = iii % 10;
      //  fprintf(stderr, "txpower changed %d %d %d %d \n", txpower, i100, i10, i1);
        command1[2] = i100;
        command1[3] = 16 * i10 + i1;
        send_command (command1);
        receive_fb ();
      }

    }
  fprintf (stderr, "period_time = %8d, dir   = %d \n", period_time, dir);

  err = snd_pcm_hw_params_get_period_size (params, &size, &dir);
  if (err < 0)
    {
      fprintf (stderr, "Unable to get period size for playback: %s\n",
	       snd_strerror (err));
      return err;
    }
  period_size = size;
  fprintf (stderr, "period_size = %8d, dir   = %d \n", (int) period_size,
	   dir);

  if (period_size < NFFT)
    {
      fprintf (stderr,
	       "error: period_size = %8d, but less than NFFT  = %d \n",
	       (int) period_size, NFFT);
      exit (1);
    }

  /* write the parameters to device */
  err = snd_pcm_hw_params (handle, params);
  if (err < 0)
    {
      fprintf (stderr, "Unable to set hw params for playback: %s\n",
	       snd_strerror (err));
      return err;
    }

  return 0;
}

static int
set_swparams (snd_pcm_t * handle, snd_pcm_sw_params_t * swparams)
{
  int err;

  /* get the current swparams */
  err = snd_pcm_sw_params_current (handle, swparams);
  if (err < 0)
    {
      fprintf (stderr,
	       "Unable to determine current swparams for playback: %s\n",
	       snd_strerror (err));
      return err;
    }

  /* start the transfer when the buffer is almost full: */
  /* (buffer_size / avail_min) * avail_min */
  err =
    snd_pcm_sw_params_set_start_threshold (handle, swparams,
					   (buffer_size / period_size) *
					   period_size);
  if (err < 0)
    {
      fprintf (stderr,
	       "Unable to set start threshold mode for playback: %s\n",
	       snd_strerror (err));
      return err;
    }

  /* allow the transfer when at least period_size samples can be processed */
  /* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
  err =
    snd_pcm_sw_params_set_avail_min (handle, swparams,
				     period_event ? buffer_size :
				     period_size);
  if (err < 0)
    {
      fprintf (stderr, "Unable to set avail min for playback: %s\n",
	       snd_strerror (err));
      return err;
    }

  /* enable period events when requested */
  if (period_event)
    {
      err = snd_pcm_sw_params_set_period_event (handle, swparams, 1);
      if (err < 0)
	{
	  fprintf (stderr, "Unable to set period event: %s\n",
		   snd_strerror (err));
	  return err;
	}
    }

  /* write the parameters to the playback device */
  err = snd_pcm_sw_params (handle, swparams);
  if (err < 0)
    {
      fprintf (stderr, "Unable to set sw params for playback: %s\n",
	       snd_strerror (err));
      return err;
    }
  return 0;
}

static void
async_callback (snd_async_handler_t * ahandler)
{
  snd_pcm_t *handle = snd_async_handler_get_pcm (ahandler);
  signed short *samples = snd_async_handler_get_callback_private (ahandler);
  snd_pcm_sframes_t avail;
  int err;
//  static int icount = 0;

  avail = snd_pcm_avail_update (handle);

  while (avail >= period_size)
    {
      err = snd_pcm_readi (handle, samples, period_size);
      if (err < 0)
	{
	  fprintf (stderr, "Write error: %s\n", snd_strerror (err));
	  exit (EXIT_FAILURE);
	}
      if (err != period_size)
	{
	  fprintf (stderr, "Write error: written %i expected %li\n", err,
		   period_size);
	  exit (EXIT_FAILURE);
	}

      for (int i = 0; i < NFFT; i++)
	{			/* NFFT=period_size */
	  audio_signal[i] = samples[i];
#ifdef MARKER
	  audio_signal[i] +=
	    16384.0 * (0.25 * sin (2.0 * 3.14 * 600.0 * (double) i / rate)
		       + 0.25 * sin (2.0 * 3.14 * 500.0 * (double) i / rate)
		       + 0.25 * sin (2.0 * 3.14 * 450.0 * (double) i / rate)
		       + 0.25 * sin (2.0 * 3.14 * 750.0 * (double) i / rate));
#endif

	}
//      fprintf(stderr, "async_callback: icount = %12d, iwater = %12d \n", icount++, iwater);

/* audio signal FFT */

      for (int i = 0; i < NFFT; i++)
	{
	  in[i] = fft_window[i] * audio_signal[i];
	}

      fftw_execute (p);

/* log10 and normalize */

      for (int i = 0; i < NFFT / 4; i++)
	{
	  double val;
	  val = out[i][0] * out[i][0] + out[i][1] * out[i][1];
	  if (val < pow (10.0, amin))
	    {
	      audio_signal_ffted[i] = 0.0;
	    }
	  else if (val > pow (10.0, amax))
	    {
	      audio_signal_ffted[i] = 1.0;
	    }
	  else
	    {
	      audio_signal_ffted[i] = (log10 (val) - amin) / (amax - amin);
	    }
	}
      timeout2 (NULL);
      avail = snd_pcm_avail_update (handle);
    }
}

static int
async_loop (snd_pcm_t * handle, signed short *samples)
{
  snd_async_handler_t *ahandler;
  int err;

  err =
    snd_async_add_pcm_handler (&ahandler, handle, async_callback, samples);
  if (err < 0)
    {
      fprintf (stderr, "Unable to register async handler\n");
      exit (EXIT_FAILURE);
    }

  if (snd_pcm_state (handle) == SND_PCM_STATE_PREPARED)
    {
      err = snd_pcm_start (handle);
      if (err < 0)
	{
	  fprintf (stderr, "Start error: %s\n", snd_strerror (err));
	  exit (EXIT_FAILURE);
	}
    }

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

gboolean
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
      return FALSE;
    }

  return TRUE;
}

gboolean
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

/*
  for(int i=0;i<n_command;i++) {
    fprintf(stderr, "command[%2d] = [%02x] \n", i, command[i]);
  }
*/

  write (fd, command, n_command);
  n_echoback = myread (echoback);	/* get echo back */

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
      return FALSE;
    }

  return TRUE;
}

static gboolean
cb_mouse_event (GtkWidget * widget, GdkEventScroll * event, gpointer data)
{
  double clicked_baseband_freq, freq_delta;
  int ix, index;

//  fprintf(stderr, "mouse scroll event at (%d, %d) with direction = %d \n", (int) event->x, (int) event->y, event->direction);
  ix = (int) event->x;

/* mouse event in the waterfall window */

  if (g_strcmp0 ((char *) data, "Waterfall") == 0)
    {
      if (ix > 0 && ix < WATERFALL_XSIZE)
	{
	  clicked_baseband_freq = (ix / waterfall_scale_x) * bin_size;
	  freq_delta = clicked_baseband_freq - cw_pitch;
	  if (operating_mode == 0x03 || operating_mode == 0x00)
	    {			/* CW or LSB */
	      ifreq_in_hz -= freq_delta;
	    }
	  if (operating_mode == 0x07 || operating_mode == 0x01)
	    {			/* CW-R or USB */
	      ifreq_in_hz += freq_delta;
	    }
//    fprintf (stderr, "Waterfall: clicked_baseband_freq = %f, freq_delta = %f, new frequency is %ld \n", clicked_baseband_freq, freq_delta, ifreq_in_hz);
	  set_freq (ifreq_in_hz);
	  return FALSE;
	}
    }

/* mouse event in the main window */
/* mouse event on frequency display   */
/* cairo_move_to(cr,  4.0+28.0*i,0);  // kHz chars */
/* cairo_move_to(cr,157.0+28.0*i,0);  //  Hz chars */

  if (g_strcmp0 ((char *) data, "Main") == 0)
    {
      if (ix >= 4 && ix <= 4 + 5 * 28)
	{
	  index = 7 - (ix - 4) / 28;
	}
      else if (ix >= 157 && ix <= 157 + 3 * 28)
	{
	  index = 2 - (ix - 157) / 28;
	}
      else
	{
	  index = -1;
	}

      if (index < 0)
	{
	  freq_delta = 0.0;
	}
      else
	{
	  freq_delta = pow (10.0, (double) index);
	}

      if ((event->direction) == 0)
	{
	  freq_delta = -freq_delta;
	}

      if ((ifreq_in_hz + freq_delta >= 0)
	  && (ifreq_in_hz + freq_delta < 60000000))
	{
	  ifreq_in_hz += freq_delta;
	}

      set_freq (ifreq_in_hz);
    }
  return FALSE;
}

gboolean
timeout (gpointer data)
{
  GtkWidget *widget = GTK_WIDGET (data);
  if (!widget->window)
    return TRUE;
  gtk_widget_queue_draw (widget);
  return TRUE;
}

gboolean
timeout2 (gpointer data)
{
//  static int icount = 0;
//  fprintf(stderr,"timeout2: icount = %12d \n", icount++);
  GtkWidget *widget1 = GTK_WIDGET (window1);
  GtkWidget *widget2 = GTK_WIDGET (window2);
  if (!widget1->window)
    return TRUE;
  if (!widget2->window)
    return TRUE;
  gtk_widget_queue_draw (widget1);
  gtk_widget_queue_draw (widget2);
  return TRUE;
}

gboolean
configure (GtkWidget * widget, GdkEventConfigure * event, gpointer data)
{
  fprintf (stderr, "configure event at %s \n", (char *) data);
  return TRUE;
}

gboolean
cb_expose1 (GtkWidget * widget, GdkEventExpose * event, gpointer data)
{
  int x, y, width, height;

  x = event->area.x;
  y = event->area.y;
  width = event->area.width;
  height = event->area.height;

  cairo_t *cr = gdk_cairo_create (widget->window);
  cairo_pattern_t *pattern;

  cairo_rectangle (cr, x, y, width, height);
  cairo_clip (cr);

/* s-meter */

  double x0 = 350;
  double y0 = height * 0.8;
  double ss = height * 0.65;
  double th = (2.0 * 3.14 / 360.0) * (180.0 + 30.0 + 120.0 * s_meter / 255.0);
  double xx = x0 + ss * cos (th);
  double yy = y0 + ss * sin (th);

  pattern =
    cairo_pattern_create_linear (x0 - ss, 0.1 * height, x0 - ss,
				 0.9 * height);
  cairo_pattern_add_color_stop_rgb (pattern, 0.0, 0.2, 0.2, 1.0);
  cairo_pattern_add_color_stop_rgb (pattern, 1.0, 0.8, 0.8, 1.0);
  cairo_rectangle (cr, x0 - ss, 0.1 * height, 2.0 * ss, 0.8 * height);
  cairo_set_source (cr, pattern);
  cairo_fill_preserve (cr);

  cairo_set_source_rgb (cr, 0.0, 0.1, 1.0);
  cairo_stroke (cr);

  cairo_set_source_rgb (cr, 0xdc / 255.0, 0x14 / 255.0, 0x3c / 255.0);
  cairo_set_line_width (cr, 4.0);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
  cairo_move_to (cr, x0, y0);
  cairo_line_to (cr, xx, yy);
  cairo_stroke (cr);

/* audio signal waveform */

  x0 = 400;
  cairo_set_source_rgb (cr, 0x22 / 255.0, 0x8b / 255.0, 0x22 / 255.0);
  cairo_set_line_width (cr, 2.0);
  for (int i = 0; i < WAVEFORM_LEN; i++)
    {
      cairo_line_to (cr, x0 + i,
		     height * 0.5 + height * 0.4 * audio_signal[2 * i +
								NFFT / 2] /
		     32768.0);
    }
  cairo_stroke (cr);

  cairo_set_source_rgb (cr, 0x80 / 255.0, 0x80 / 255.0, 0x80 / 255.0);
  cairo_set_line_width (cr, 1.0);
  cairo_move_to (cr, x0, height * 0.5 + 0.5);
  cairo_line_to (cr, x0 + WAVEFORM_LEN, height * 0.5 + 0.5);
  cairo_stroke (cr);

/* spectrum bar graph*/

  x0 = 580;
  int hlen;
  double fmin = 300.0;
  double fmax = 900.0;
  int ibinmin = fmin / bin_size;
  int ibinmax = fmax / bin_size;

  cairo_set_source_rgb (cr, 0x00 / 255.0, 0x00 / 255.0, 0x00 / 255.0);
  cairo_move_to (cr, x0 + ((int) (cw_pitch / bin_size) - ibinmin) * 2 + 0.5,
		 height - 9);
  cairo_line_to (cr, x0 + ((int) (cw_pitch / bin_size) - ibinmin) * 2 + 0.5,
		 3);
  cairo_stroke (cr);

  for (int ifreq = 450; ifreq <= 750; ifreq += 50)
    {
      cairo_move_to (cr, x0 + ((int) (ifreq / bin_size) - ibinmin) * 2 + 0.5,
		     height - 3);
      cairo_line_to (cr, x0 + ((int) (ifreq / bin_size) - ibinmin) * 2 + 0.5,
		     height - 6);
      cairo_stroke (cr);
    }

  cairo_set_source_rgb (cr, 0xff / 255.0, 0x00 / 255.0, 0x00 / 255.0);
  cairo_set_line_width (cr, 1.0);
  for (int i = ibinmin; i < ibinmax; i++)
    {
      hlen = (height - 6) * audio_signal_ffted[i];
      cairo_line_to (cr, x0 + (i - ibinmin) * 2, height - 3 - hlen);
    }
  cairo_stroke (cr);

/* frequency display */

  char string[128];
  cairo_set_source_rgb (cr, 0.0, 0.1, 1.0);
  cairo_select_font_face (cr, "FreeSans", CAIRO_FONT_SLANT_NORMAL,
			  CAIRO_FONT_WEIGHT_NORMAL);

  cairo_set_font_size (cr, 50.0);
  if (ifreq_in_hz >= 10000000)
    {				/* higher than 10MHz */
      sprintf (string, "%9.3f", ((double) ifreq_in_hz) / 1000.0);
      cairo_move_to (cr, 4.0, height - 6.0);
    }
  else
    {
      sprintf (string, "%8.3f", ((double) ifreq_in_hz) / 1000.0);
      cairo_move_to (cr, 32.0, height - 6.0);
    }
  cairo_show_text (cr, string);

  cairo_set_font_size (cr, 20.0);
  cairo_move_to (cr, 250.0, height - 6.0);
  cairo_show_text (cr, "kHz");

/*
        cairo_set_line_width(cr,1.0);
        cairo_set_source_rgb(cr,0.5,1.0,0.5);
        for(int i=0;i<6;i++) {
                cairo_move_to(cr,4.0+28.0*i,0);
                cairo_line_to(cr,4.0+28.0*i,height);
                cairo_stroke(cr);
        }
        for(int i=0;i<4;i++) {
                cairo_move_to(cr,157.0+28.0*i,0);
                cairo_line_to(cr,157.0+28.0*i,height);
                cairo_stroke(cr);
        }
*/

  cairo_set_source_rgb (cr, 0, 0, 0);
  cairo_destroy (cr);

  return FALSE;
}

void
serial_init (void)
{
  struct termios tio;
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
}

void
set_freq (long int ifreq_in_hz)
{
  fprintf (stderr, "freq set to %12.3f [kHz] \n",
	   (double) ifreq_in_hz / 1000.0);
  static unsigned char command1[7] =
    { 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfd };
  long int ifreq_wrk;
  int idigit[8];

  ifreq_wrk = ifreq_in_hz;

  for (int i = 0; i < 8; i++)
    {
      idigit[i] = ifreq_wrk % 10;
      ifreq_wrk /= 10;
    }
  command1[1] = 16 * idigit[1] + idigit[0];
  command1[2] = 16 * idigit[3] + idigit[2];
  command1[3] = 16 * idigit[5] + idigit[4];
  command1[4] = 16 * idigit[7] + idigit[6];
  send_command (command1);
  receive_fb ();
}

void
set_tx_power (int txpower)
{
  static unsigned char command1[5] = { 0x14, 0x0a, 0x00, 0x32, 0xfd };
  int iii, i100, i10, i1;

  if (txpower < 2)
    txpower = 2;
  if (txpower > 100)
    txpower = 100;
  iii = 255.0 * (txpower - 2) / 100.0;
  i100 = iii / 100;
  i10 = (iii - 100 * i100) / 10;
  i1 = iii % 10;
//  fprintf(stderr, "txpower changed %d %d %d %d \n", txpower, i100, i10, i1);
  command1[2] = i100;
  command1[3] = 16 * i10 + i1;
  send_command (command1);
  receive_fb ();
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
set_cw_pitch (int pitch)
{
  static unsigned char command1[5] = { 0x14, 0x09, 0x00, 0x32, 0xfd };
  int iii, i100, i10, i1;

  if (pitch < 300)
    pitch = 300;
  if (pitch > 900)
    pitch = 900;
  cw_pitch = pitch;
  iii = 128 + 127.0 * (pitch - 600) / 300.0;
  i100 = iii / 100;
  i10 = (iii - 100 * i100) / 10;
  i1 = iii % 10;
//  fprintf(stderr, "pitch changed %d %d %d %d \n", pitch, i100, i10, i1);
  command1[2] = i100;
  command1[3] = 16 * i10 + i1;
  send_command (command1);
  receive_fb ();
}

void
set_cw_speed (int wpm)
{
  static unsigned char command1[5] = { 0x14, 0x0c, 0x00, 0x32, 0xfd };
  int iii, i100, i10, i1;

  if (wpm < 6)
    wpm = 6;
  if (wpm > 48)
    wpm = 48;
  iii = 255 * (wpm - 6) / (48 - 6);
  i100 = iii / 100;
  i10 = (iii - 100 * i100) / 10;
  i1 = iii % 10;
//  fprintf(stderr, "wpm changed %d %d %d %d \n", wpm, i100, i10, i1);
  command1[2] = i100;
  command1[3] = 16 * i10 + i1;
  send_command (command1);
  receive_fb ();
}

void
scale_value_changed (GtkWidget * scale, gpointer data)
{
  static unsigned char command[5] = { 0x14, 0x00, 0x00, 0x00, 0xfd };
  int val, i100, i10, i1;

  val = gtk_range_get_value (GTK_RANGE (scale));

  if (g_strcmp0 ((char *) data, "W1") == 0)
    {
      amin = val;
      return;
    }
  else if (g_strcmp0 ((char *) data, "W2") == 0)
    {
      amax = val;
      return;
    }

  if (g_strcmp0 ((char *) data, "AF Gain") == 0)
    {
      command[1] = 0x01;
    }
  if (g_strcmp0 ((char *) data, "RF Gain") == 0)
    {
      command[1] = 0x02;
    }
  if (g_strcmp0 ((char *) data, "NR") == 0)
    {
      command[1] = 0x06;
    }
  if (g_strcmp0 ((char *) data, "PBT1") == 0)
    {
      command[1] = 0x07;
    }
  if (g_strcmp0 ((char *) data, "PBT2") == 0)
    {
      command[1] = 0x08;
    }
  if (g_strcmp0 ((char *) data, "Notch") == 0)
    {
      command[1] = 0x0d;
    }
  if (g_strcmp0 ((char *) data, "NB") == 0)
    {
      command[1] = 0x12;
    }
  fprintf (stderr, "command[5] = %02x \n", command[5]);

  i100 = val / 100;
  i10 = (val - 100 * i100) / 10;
  i1 = val % 10;
  command[2] = i100;
  command[3] = 16 * i10 + i1;

  send_command (command);
  receive_fb ();
  return;

}

void
callback3 (GtkWidget * widget, gpointer data)
{
  char s[100];
  int n;
  double freq_in_khz;

  n = strlen ((char *) data);
  strncpy (s, (char *) data, strlen ((char *) data) - 3);
  s[n - 3] = '\0';
  freq_in_khz = atof (s);
  fprintf (stderr, "callback3: %s was toggled %d [%s] %f \n", (char *) data,
	   n, s, freq_in_khz);
  ifreq_in_hz = freq_in_khz * 1000.0;
  set_freq (ifreq_in_hz);
}

gboolean
myclock (gpointer data)
{
  static unsigned char command1[2] = { 0x03, 0xfd };
  static unsigned char command2[3] = { 0x15, 0x02, 0xfd };
  static unsigned char command3[2] = { 0x04, 0xfd };
  char string[256];
  unsigned char buf[255];
  int res;

/* read operating mode, response in char[5]-char[6] */

  send_command (command3);
  res = myread (buf);
  if (res != 8)
    {
      fprintf (stderr, "operating mode response is wrong! \n");
    }
  operating_mode = buf[5];
  dsp_filter = buf[6];
  if (operating_mode == 0x03)
    {				/* CW */
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button2dim[0][0]),
				    TRUE);
    }
  if (operating_mode == 0x07)
    {				/* CW-R */
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button2dim[0][1]),
				    TRUE);
    }
  if (operating_mode == 0x00)
    {				/* LSB */
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button2dim[0][2]),
				    TRUE);
    }
  if (operating_mode == 0x01)
    {				/* USB */
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button2dim[0][3]),
				    TRUE);
    }
  if (dsp_filter == 0x01)
    {				/* FIL1 */
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button2dim[6][0]),
				    TRUE);
    }
  if (dsp_filter == 0x02)
    {				/* FIL2 */
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button2dim[6][1]),
				    TRUE);
    }
  if (dsp_filter == 0x03)
    {				/* FIL3 */
      gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button2dim[6][2]),
				    TRUE);
    }

/* freq response in char[8]-char[5] */

  send_command (command1);
  res = myread (buf);

#ifdef DEBUG
  unsigned char *s;
  fprintf (stderr, "response for frequncy read, res = %2d : ", res);
  s = buf;
  for (int i = 0; i < res; i++)
    {
      fprintf (stderr, "[%02x] ", *s++);
    }
  fprintf (stderr, "\n");
#endif

  if (res != 11)
    {
      fprintf (stderr, "frequency response is wrong! \n");
    }
  sprintf (string, "%02x%02x%02x%02x", buf[8], buf[7], buf[6], buf[5]);
  ifreq_in_hz = atoi (string);

/* S-meter response in char[6]-char[5] */

  send_command (command2);
  res = myread (buf);
  if (res != 9)
    {
      fprintf (stderr, "S-meter response is wrong! \n");
    }

  sprintf (string, "%02x%02x", buf[6], buf[7]);
  s_meter = atoi (string);

  return TRUE;
}

void
callback2 (GtkWidget * widget, gpointer data)
{
  static unsigned char command3[4] = { 0x16, 0x47, 0x00, 0xfd };	/* BKIN OFF  */
  static unsigned char command4[4] = { 0x16, 0x47, 0x01, 0xfd };	/* BKIN ON   */
  static unsigned char command5[4] = { 0x16, 0x47, 0x02, 0xfd };	/* BKIN FULL */
  static unsigned char command6[4] = { 0x16, 0x55, 0x00, 0xfd };	/* IF FIL1 */
//static unsigned char command7 [4] = { 0x16, 0x55, 0x01, 0xfd };       /* if fil2 */
  static unsigned char command8[4] = { 0x16, 0x55, 0x02, 0xfd };	/* IF FIL3 */
  static unsigned char command19[4] = { 0x16, 0x56, 0x00, 0xfd };	/* DSP SHARP */
  static unsigned char command20[4] = { 0x16, 0x56, 0x01, 0xfd };	/* DSP SOFT */
  static unsigned char command9[4] = { 0x16, 0x02, 0x00, 0xfd };	/* PRE-AMP OFF */
  static unsigned char command10[4] = { 0x16, 0x02, 0x01, 0xfd };	/* PRE-AMP 1  */
  static unsigned char command11[4] = { 0x16, 0x02, 0x02, 0xfd };	/* PRE-AMP 2  */
  static unsigned char command12[3] = { 0x11, 0x00, 0xfd };	/* ATT OFF  */
  static unsigned char command13[3] = { 0x11, 0x20, 0xfd };	/* ATT 20dB */
  static unsigned char command14[3] = { 0x12, 0x00, 0xfd };	/* ANT 1 */
  static unsigned char command15[3] = { 0x12, 0x01, 0xfd };	/* ANT 2 */
  static unsigned char command16[4] = { 0x16, 0x12, 0x01, 0xfd };	/* AGC FAST  */
  static unsigned char command17[4] = { 0x16, 0x12, 0x02, 0xfd };	/* AGC FAST  */
  static unsigned char command18[4] = { 0x16, 0x12, 0x03, 0xfd };	/* AGC SLOW  */
  int wpm, pitch, txpower;

//  fprintf(stderr, "callback2: %s was toggled \n", (char *) data);

  if (GTK_TOGGLE_BUTTON (widget)->active)
    {
//    fprintf(stderr, "active \n");
    }
  else
    {
//    fprintf(stderr, "not active, do nothing and return FALSE \n");
      return;
    }

  if (g_strcmp0 ((char *) data, "CW") == 0)
    {
      operating_mode = 3;
      set_operating_mode ();
    }

  if (g_strcmp0 ((char *) data, "CW-REV") == 0)
    {
      operating_mode = 7;
      set_operating_mode ();
    }

  if (g_strcmp0 ((char *) data, "LSB") == 0)
    {
      operating_mode = 0;
      set_operating_mode ();
    }

  if (g_strcmp0 ((char *) data, "USB") == 0)
    {
      operating_mode = 1;
      set_operating_mode ();
    }

  if (g_strcmp0 ((char *) data, "DSP FIL1") == 0)
    {
      dsp_filter = 1;
      set_operating_mode ();
    }

  if (g_strcmp0 ((char *) data, "DSP FIL2") == 0)
    {
      dsp_filter = 2;
      set_operating_mode ();
    }

  if (g_strcmp0 ((char *) data, "DSP FIL3") == 0)
    {
      dsp_filter = 3;
      set_operating_mode ();
    }

  if (g_strcmp0 ((char *) data, "BKIN OFF") == 0)
    {
      send_command (command3);
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "BKIN ON") == 0)
    {
      send_command (command4);
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "BKIN FULL") == 0)
    {
      send_command (command5);
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "IF FIL1") == 0)
    {
      fprintf (stderr, "IF FIL1 \n");
      send_command (command6);
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "if fil2") == 0)
    {
      fprintf (stderr, "IF FIL2 is not implemented. \n");
    }

  if (g_strcmp0 ((char *) data, "IF FIL3") == 0)
    {
      fprintf (stderr, "IF FIL3 \n");
      send_command (command8);
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "DSP SHARP") == 0)
    {
      fprintf (stderr, "DSP SHARP \n");
      send_command (command19);
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "DSP SOFT") == 0)
    {
      fprintf (stderr, "DSP SOFT \n");
      send_command (command20);
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "BOTH OFF") == 0)
    {
      fprintf (stderr, "PRE-AMP OFF \n");
      send_command (command9);	/* PRE-AMP OFF */
      receive_fb ();
      send_command (command12);	/* ATT OFF */
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "PRE-AMP 1") == 0)
    {
      fprintf (stderr, "PRE-AMP 1 \n");
      send_command (command10);
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "PRE-AMP 2") == 0)
    {
      fprintf (stderr, "PRE-AMP 2 \n");
      send_command (command11);
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "ATT 20dB") == 0)
    {
      fprintf (stderr, "ATT 20dB \n");
      send_command (command13);
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "AGC FAST") == 0)
    {
      fprintf (stderr, "AGC FIRST \n");
      send_command (command16);
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "AGC MID") == 0)
    {
      fprintf (stderr, "AGC MID \n");
      send_command (command17);
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "AGC SLOW") == 0)
    {
      fprintf (stderr, "AGC SLOW \n");
      send_command (command18);
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "ANT 1") == 0)
    {
      fprintf (stderr, "ANT 1 \n");
      send_command (command14);
      receive_fb ();
    }

  if (g_strcmp0 ((char *) data, "ANT 2") == 0)
    {
      fprintf (stderr, "ANT 2 \n");
      send_command (command15);
      receive_fb ();
    }

  if (g_str_has_suffix ((char *) data, "Hz")
      && !g_str_has_suffix ((char *) data, "kHz"))
    {
      pitch = atoi (g_strndup ((char *) data, 3));
      set_cw_pitch (pitch);
      fprintf (stderr, "CW pitch set to %d \n", pitch);
    }

  if (g_str_has_suffix ((char *) data, "wpm"))
    {
      wpm = atoi (g_strndup ((char *) data, 2));
      set_cw_speed (wpm);
      fprintf (stderr, "Key Speed set to %d \n", wpm);
    }

  if (g_str_has_suffix ((char *) data, " W"))
    {
      int n = mystrlen (data);
      txpower = atoi (g_strndup ((char *) data, n - 3));	/* delete " W"+EOC */
      set_tx_power (txpower);
      fprintf (stderr, "TX power set to %d \n", txpower);
    }

  char s[100];
  int n;
  double freq_in_khz;
  if (g_str_has_suffix ((char *) data, "kHz"))
    {
      n = strlen ((char *) data);
      strncpy (s, (char *) data, strlen ((char *) data) - 3);
      s[n - 3] = '\0';
      freq_in_khz = atof (s);
      fprintf (stderr, "freq set to  %f \n", freq_in_khz);
      ifreq_in_hz = freq_in_khz * 1000.0;
      set_freq (ifreq_in_hz);
    }

  return;
}

gint
delete_event (GtkWidget * widget, GdkEvent * event, gpointer data)
{
  fprintf (stderr, "delete_event at %s \n", (char *) data);
  gtk_main_quit ();
  return (FALSE);
}

int
main (int argc, char *argv[])
{

  struct vscale
  {
    double value_min;
    double value_max;
    double value_ini;
    char *vscale_name;
  };

  int nvscale = 9;
  struct vscale my_vscale[128] = {
    {0.0, 255.0, 32.0, "AF Gain"},
    {0.0, 255.0, 255.0, "RF Gain"},
    {0.0, 255.0, 16.0, "NR"},
    {0.0, 255.0, 128.0, "PBT1"},
    {0.0, 255.0, 128.0, "PBT2"},
    {0.0, 255.0, 128.0, "Notch"},
    {0.0, 255.0, 16.0, "NB"},
    {4.0, 10.0, 7.0, "W1"},
    {11.0, 17.0, 14.0, "W2"},
  };

  struct radio_button
  {
    int nbutton;
    char name[32][128];
  };

  int ngroup = 12;
  struct radio_button my_radio_buttons[128] = {
    {4, {"CW", "CW-REV", "LSB", "USB"}},
    {7, {"450Hz", "500Hz", "550Hz", "600Hz", "650Hz", "700Hz", "750Hz"}},
    {7,
     {"10 wpm", "15 wpm", "20 wpm", "25 wpm", "30 wpm", "35 wpm", "40 wpm"}},
    {3, {"BKIN OFF", "BKIN ON", "BKIN FULL"}},
    {6, {"2 W", "5 W", "10 W", "20 W", "50 W", "100 W"}},
    {3, {"IF FIL1", "if fil2", "IF FIL3"}},
    {3, {"DSP FIL1", "DSP FIL2", "DSP FIL3"}},
    {2, {"DSP SHARP", "DSP SOFT"}},
    {4, {"PRE-AMP 1", "PRE-AMP 2", "ATT 20dB", "BOTH OFF"}},
    {3, {"AGC FAST", "AGC MID", "AGC SLOW"}},
    {2, {"ANT 1", "ANT 2"}},
    {8,
     {" 3501.234kHz", " 7026.000kHz", "10118.000kHz", "14058.000kHz",
      "18085.000kHz", "21058.000kHz", "24908.000kHz", "28058.000kHz"}},
  };

  struct termios oldtio;

  struct mybutton
  {
    char name[100];
    int left_attach;
    int right_attach;
    int top_attach;
    int bottom_attach;
  };

  GtkWidget *box1, *box2, *box3;
  GtkWidget *hsep, *vsep;
  GtkWidget *labels[100];
  GtkObject *adj[100];
  GtkWidget *scale[100];
  GdkPixbuf *pixbuf;

  snd_pcm_t *handle;
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  signed short *samples;
  int err;

  if (argc != 3)
    {
      fprintf (stderr, "Usage %s /dev/ttyUSB0 hw:2,0 \n", argv[0]);
      fprintf (stderr,
	       " try ls -l /dev/ttyUSB*, and arecord -l to know these parameters.\n");
      return -1;
    }
  fprintf (stderr, "serial device is [%s], audio capture device is [%s] \n",
	   argv[1], argv[2]);
  strcpy (myrig, argv[1]);
  strcpy (device, argv[2]);

  bin_size = rate / (double) NFFT;
  for (int i = 0; i < NFFT; i++)
    {
      fft_window[i] = 0.54 - 0.46 * cos (2.0 * M_PI * i / (double) NFFT);
    }

  in = malloc (sizeof (double) * NFFT);
  out = (fftw_complex *) fftw_malloc (sizeof (fftw_complex) * (NFFT / 2 + 1));
  p = fftw_plan_dft_r2c_1d (NFFT, in, out, FFTW_MEASURE);

  snd_pcm_hw_params_alloca (&hwparams);
  snd_pcm_sw_params_alloca (&swparams);

  fprintf (stderr, "audio capture device = %s, rate = %i Hz, %i channels\n",
	   device, rate, channels);

  if ((err = snd_pcm_open (&handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
      fprintf (stderr, "Capture open error: %s\n", snd_strerror (err));
      return 0;
    }

  if ((err = set_hwparams (handle, hwparams)) < 0)
    {
      fprintf (stderr, "Setting of hwparams failed: %s\n",
	       snd_strerror (err));
      exit (EXIT_FAILURE);
    }
  if ((err = set_swparams (handle, swparams)) < 0)
    {
      fprintf (stderr, "Setting of swparams failed: %s\n",
	       snd_strerror (err));
      exit (EXIT_FAILURE);
    }

  nsamples = period_size * channels * byte_per_sample;
  fprintf (stderr, "nsamples = %d \n", nsamples);
  samples = malloc (nsamples);
  if (samples == NULL)
    {
      fprintf (stderr, "No enough memory\n");
      exit (EXIT_FAILURE);
    }

  err = async_loop (handle, samples);

/*
        if (err < 0) fprintf(stderr,"Transfer failed: %s\n", snd_strerror(err));

        free(samples);
        snd_pcm_close(handle);
*/

  gtk_init (&argc, &argv);

  fd = open (myrig, O_RDWR | O_NOCTTY);
  if (fd < 0)
    {
      fprintf (stderr, "Error: can not open %s \n", myrig);
      return (-1);
    }
  tcgetattr (fd, &oldtio);
  serial_init ();

  window1 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  window2 = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window1), "IC-7410 Rig Control (Main)");
  gtk_window_set_title (GTK_WINDOW (window2),
			"IC-7410 Rig Control (Waterfall)");
  gtk_widget_set_size_request (window1, WINDOW_XSIZE, WINDOW_YSIZE);
  gtk_widget_set_size_request (window2, WATERFALL_XSIZE, WATERFALL_YSIZE);
  g_signal_connect (GTK_OBJECT (window1), "delete_event",
		    G_CALLBACK (delete_event), (gpointer) "Main");
  g_signal_connect (GTK_OBJECT (window2), "delete_event",
		    G_CALLBACK (delete_event), (gpointer) "Waterfall");
  gtk_container_set_border_width (GTK_CONTAINER (window1), 5);
  gtk_container_set_border_width (GTK_CONTAINER (window2), 2);

  box1 = gtk_vbox_new (FALSE, 5);
  box2 = gtk_hbox_new (FALSE, 5);
//  gtk_widget_set_size_request (box2, 720, 200);

/* radio buttons */

  GtkWidget *hbox_radio, *vbox_radio;
  hbox_radio = gtk_hbox_new (FALSE, 5);
  vsep = gtk_vseparator_new ();
  gtk_box_pack_start (GTK_BOX (hbox_radio), vsep, FALSE, FALSE, 5);
  for (int j = 0; j < ngroup; j++)
    {
      vbox_radio = gtk_vbox_new (FALSE, 5);
      for (int i = 0; i < my_radio_buttons[j].nbutton; i++)
	{
	  if (i == 0)
	    {
	      button2dim[j][i] =
		gtk_radio_button_new_with_label (NULL,
						 my_radio_buttons[j].name[i]);
	    }
	  else
	    {
	      button2dim[j][i] =
		gtk_radio_button_new_with_label (gtk_radio_button_get_group
						 (GTK_RADIO_BUTTON
						  (button2dim[j][0])),
						 my_radio_buttons[j].name[i]);
	    }
	  g_signal_connect (GTK_OBJECT (button2dim[j][i]), "toggled",
			    G_CALLBACK (callback2),
			    (gpointer) my_radio_buttons[j].name[i]);
	  gtk_box_pack_start (GTK_BOX (vbox_radio), button2dim[j][i], FALSE,
			      FALSE, 0);
	  gtk_widget_show (button2dim[j][i]);
	}
      gtk_box_pack_start (GTK_BOX (hbox_radio), vbox_radio, FALSE, FALSE, 0);
      vsep = gtk_vseparator_new ();
      gtk_box_pack_start (GTK_BOX (hbox_radio), vsep, FALSE, FALSE, 5);
    }

/* vscale */

  for (int i = 0; i < nvscale; i++)
    {
      box3 = gtk_vbox_new (FALSE, 5);
      labels[i] = gtk_label_new_with_mnemonic (my_vscale[i].vscale_name);
      adj[i] =
	gtk_adjustment_new (my_vscale[i].value_ini, my_vscale[i].value_min,
			    my_vscale[i].value_max, 5.0, 1.0, 0.0);
      scale[i] = gtk_vscale_new (GTK_ADJUSTMENT (adj[i]));
      gtk_scale_set_value_pos (GTK_SCALE (scale[i]), GTK_POS_TOP);
      gtk_scale_set_digits (GTK_SCALE (scale[i]), 0);

      g_signal_connect (GTK_OBJECT (scale[i]), "value_changed",
			G_CALLBACK (scale_value_changed),
			(gpointer) my_vscale[i].vscale_name);

      gtk_box_pack_start (GTK_BOX (box3), scale[i], TRUE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (box3), labels[i], TRUE, TRUE, 0);
      gtk_box_pack_start (GTK_BOX (box2), box3, FALSE, FALSE, 20);
      if (i == 6)
	{
	  vsep = gtk_vseparator_new ();
	  gtk_box_pack_start (GTK_BOX (box2), vsep, FALSE, FALSE, 20);
	}
    }

/********************/

  GtkWidget *drawing_area1;	/* frequency, S-meter, waveform, bar graph */
  GtkWidget *drawing_area2;	/* waterfall */
  drawing_area1 = gtk_drawing_area_new ();
  drawing_area2 = gtk_drawing_area_new ();
  gtk_widget_set_size_request (drawing_area1, AREA1_XSIZE, AREA1_YSIZE);
  gtk_widget_set_size_request (drawing_area2, WATERFALL_XSIZE,
			       WATERFALL_YSIZE);
  const GdkColor background_color1 = { 0, 0x4700, 0xce00, 0xfa00 };
  const GdkColor background_color2 = { 0, 0xf700, 0xce00, 0x4a00 };
  gtk_widget_modify_bg (drawing_area1, GTK_STATE_NORMAL, &background_color1);
  gtk_widget_modify_bg (drawing_area2, GTK_STATE_NORMAL, &background_color2);

  gtk_box_pack_start (GTK_BOX (box1), drawing_area1, FALSE, FALSE, 0);
  hsep = gtk_hseparator_new ();
  gtk_box_pack_start (GTK_BOX (box1), hsep, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box1), hbox_radio, FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (box1), box2, TRUE, TRUE, 0);

/*** waterfall by pixbuf ***/

  pixbuf =
    gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, WATERFALL_XSIZE,
		    2 * WATERFALL_YSIZE);
  int pixbuf_width = gdk_pixbuf_get_width (pixbuf);
  int pixbuf_height = gdk_pixbuf_get_height (pixbuf);
  fprintf (stderr, "pixbuf width=%d, hegiht=%d \n", pixbuf_width,
	   pixbuf_height);

/**********/

  gtk_container_add (GTK_CONTAINER (window1), box1);
  gtk_container_add (GTK_CONTAINER (window2), drawing_area2);
  gtk_widget_show_all (window1);
  gtk_widget_show_all (window2);
  gtk_widget_add_events (drawing_area1, GDK_SCROLL_MASK);
  gtk_widget_add_events (drawing_area2, GDK_SCROLL_MASK);

  g_signal_connect (G_OBJECT (drawing_area1), "configure_event",
		    G_CALLBACK (configure), (gpointer) "drawing_area1");
  g_signal_connect (G_OBJECT (drawing_area2), "configure_event",
		    G_CALLBACK (configure), (gpointer) "drawing_area2");
  g_signal_connect (G_OBJECT (drawing_area1), "expose_event",
		    G_CALLBACK (cb_expose1), (gpointer) "drawing_area1");
  g_signal_connect (G_OBJECT (drawing_area2), "expose-event",
		    G_CALLBACK (cb_expose2), (gpointer) pixbuf);
  g_signal_connect (G_OBJECT (drawing_area1), "scroll-event",
		    G_CALLBACK (cb_mouse_event), (gpointer) "Main");
  g_signal_connect (G_OBJECT (drawing_area2), "scroll-event",
		    G_CALLBACK (cb_mouse_event), (gpointer) "Waterfall");

//  g_timeout_add (TIMEOUT_VALUE, timeout, window1);
//  g_timeout_add (TIMEOUT_VALUE, timeout, window2);
//  g_timeout_add (TIMEOUT_VALUE, timeout2, NULL);
  g_timeout_add (TIMEOUT_VALUE, (GSourceFunc) myclock, (gpointer) "abc");

  gtk_main ();

  fftw_destroy_plan (p);
  fftw_free (in);
  fftw_free (out);

  return 0;
}

#endif
