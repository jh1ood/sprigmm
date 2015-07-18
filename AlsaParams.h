/*
 * AlsaParams.h
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#ifndef ALSAPARAMS_H_
#define ALSAPARAMS_H_

#include <asoundlib.h>
#include <fftw3.h>

struct AlsaParams  {
public:
	double *audio_signal {nullptr};
	double *in_real      {nullptr};  /* for IC-7410 only */
	fftw_complex *in;                /* for Soft66LC4 only */
	fftw_complex *out;
	fftw_plan     plan;

	signed short *samples      {nullptr};
	char         *sound_device {nullptr};
	char         *tty_device   {nullptr};
	const int byte_per_sample  {2};	/* 16 bit format */
	const int resample         {0};	/* disable resample */
	const int period_event     {0};	/* produce poll event after each period */
	unsigned int channels      {0};
	unsigned int rate          {0};
	int nfft                   {0};
	double bin_size          {0.0};
	int spectrum_x             {0};
	int spectrum_y             {0};
	int waterfall_x            {0};
	int waterfall_y            {0};
	double amax              {0.0};
	double amin              {0.0};

	snd_pcm_uframes_t   buffer_size {0};
	snd_pcm_uframes_t   period_size {0};
	snd_pcm_sframes_t   avail {0};
	snd_pcm_sframes_t   frames_actually_read {0};
	snd_pcm_t           *handle   {nullptr};
	snd_pcm_hw_params_t *hwparams {nullptr};
	snd_pcm_sw_params_t *swparams {nullptr};
	snd_async_handler_t *ahandler {nullptr};

	static int  ic7410_frequency;
	static int  soft66_frequency;
	static bool ic7410_changed;
	static bool soft66_changed;

	int fd            {-1};
//	int frequency      {0};
	int cw_pitch     {600};
	int s_meter        {0};
	int operating_mode {3};		/* CW=03, CW-REV=07, LSB=00, USB=01 */
	int dsp_filter     {1};		/* FIL1=01, FIL2=02, FIL3=03 */

};

#endif /* ALSAPARAMS_H_ */
