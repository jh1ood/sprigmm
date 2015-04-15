/*
 * mydefine.h
 *
 *  Created on: Apr 9, 2015
 *      Author: user1
 */
#ifndef MYDEFINE_H
#define MYDEFINE_H

#define DEBUG
#define NO_MARKER
#define NFFT                    2048
#define WINDOW_XSIZE            1920
#define WINDOW_YSIZE             500
#define AREA1_XSIZE               99
#define AREA1_YSIZE               50
#define WATERFALL_XSIZE         1900
#define WATERFALL_YSIZE          512
#define WATERFALL_XOFFSET          0
#define WATERFALL_YOFFSET          0
#define WAVEFORM_LEN             128
#define BAUDRATE                B19200
#define TIMEOUT_VALUE           100
#define END_OF_COMMAND          0xfd

#include <asoundlib.h>
#include <fftw3.h>

extern int fd;
extern unsigned int rate;
extern unsigned int channels;	/* count of channels */
extern int byte_per_sample;	/* 16 bit format */
extern unsigned int buffer_time;	/* ring buffer length in us */
extern unsigned int period_time;	/* period time in us */
extern int resample;		/* disable resample */
extern int period_event;	/* produce poll event after each period */
extern double audio_signal[NFFT];
extern double audio_signal_ffted[NFFT];
extern double fft_window[NFFT];
extern int cw_pitch;
extern int iwater;
extern int nsamples;
extern double bin_size, waterfall_scale_x;
extern double amax, amin;
extern long int ifreq_in_hz;
extern int s_meter;
extern int operating_mode;	/* CW=03, CW-REV=07, LSB=00, USB=01 */
extern int dsp_filter;		/* FIL1=01, FIL2=02, FIL3=03 */
extern snd_pcm_sframes_t buffer_size;
extern snd_pcm_sframes_t period_size;
extern snd_pcm_t *handle;
extern snd_pcm_hw_params_t *hwparams;
extern snd_pcm_sw_params_t *swparams;
extern fftw_complex *in;
extern fftw_complex *out;
extern fftw_plan p;
extern int flag_togo1, flag_togo2;

int send_command(unsigned char *partial_command);
int receive_fb();
void myfunc(int);
void set_freq(long int ifreq_in_hz);
void myclock();
int colormap_r(double);
int colormap_g(double);
int colormap_b(double);

#endif
