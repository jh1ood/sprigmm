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
	int nfft                   {0};  /* for fftw3 */
	double *audio_signal {nullptr};
	double *in_real      {nullptr};  /* for IC-7410 only */
	fftw_complex *in;                /* for Soft66LC4 only */
	fftw_complex *out;
	fftw_plan     plan;

	int    spectrum_x         {1200};
	int    spectrum_y          { 50};
	int    waterfall_x        {1200};
	int    waterfall_y         {200};
	double bin_size            {0.0};
	double amax                {9.0}; /* for psuedcolor */
	double amin                {1.0};
	int    timervalue          { 50}; /* on_timeout() */

	snd_pcm_uframes_t   buffer_size          {0};
	snd_pcm_uframes_t   period_size          {0};
	snd_pcm_sframes_t   avail                {0};
	snd_pcm_sframes_t   frames_actually_read {0};
	snd_pcm_t           *handle   {nullptr};
	snd_pcm_hw_params_t *hwparams {nullptr};
	snd_pcm_sw_params_t *swparams {nullptr};
	snd_async_handler_t *ahandler {nullptr};
	signed short *samples         {nullptr};
	char*         sound_device    {nullptr};
	const int byte_per_sample  {2};	/* 16 bit format */
	const int resample         {0};	/* disable resample */
	const int period_event     {0};
	unsigned int channels      {0};
	unsigned int rate          {0};

};

#endif /* ALSAPARAMS_H_ */
