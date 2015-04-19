/*
 * Sound.h
 *
 *  Created on: Apr 15, 2015
 *      Author: user1
 */

#ifndef SOUND_H_
#define SOUND_H_

#define NFFT 2048

#include <fftw3.h>
#include <asoundlib.h>
#include <string>
using namespace std;

class Sound {
  public:
    Sound(char *, const char *, const char *);
     virtual ~ Sound();
  public:
    int asound_set_hwparams(snd_pcm_t * handle,
			    snd_pcm_hw_params_t * hwparams);
    int asound_set_swparams(snd_pcm_t * handle,
			    snd_pcm_sw_params_t * swparams);
    int asound_async_loop(snd_pcm_t * handle, signed short *samples);
    void asound_async_callback(snd_async_handler_t * ahandler);

    snd_pcm_sframes_t buffer_size;
    snd_pcm_sframes_t period_size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_sw_params_t *swparams;
    snd_async_handler_t *ahandler;

    char *sound_device;
    unsigned int rate;
    unsigned int channels;
    int nsamples;
    int byte_per_sample = 2;	/* 16 bit format */
    unsigned int buffer_time = 500000;	/* ring buffer length in us */
    unsigned int period_time = 128000;	/* period time in us */
    int resample = 0;		/* disable resample */
    int period_event = 0;	/* produce poll event after each period */

    int nfft = 2048;
    double bin_size;
    signed short samples[99999];
    double audio_signal[99999];
    double audio_signal_ffted[99999];
    double fft_window[99999];
    fftw_complex *in, *out;
    fftw_plan p;
    double amax, amin;

    int cw_pitch = 600;
    int iwater = 0;
//    int flag_togo1 = 0;
//    int flag_togo2 = 0;
//    int flag_togo3 = 0;
//    int flag_togo4 = 0;
};

#endif				/* SOUND_H_ */
