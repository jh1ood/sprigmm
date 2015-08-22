/*
 * AlsaParams.h
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#ifndef ALSAPARAMS_H_
#define ALSAPARAMS_H_

#include <asoundlib.h>
#include <cmath>
#include <chrono>
#include <fftw3.h>

struct AlsaParams  {
public:
	double *audio_signal {nullptr}; /* valid data in [start,end) */
	double *signal_start {nullptr};
	double *signal_end   {nullptr};
	double *audio_window {nullptr};
	static constexpr int audio_signal_buffer_size {1024*1024};
	int nfft                   {0}; /* for fftw3 */
	int loop_count             {0};

	double fft_forward_ratio {1.0}; /* (0, 1] the value 1.0 is for non-overlap */
	fftw_complex *in;
	fftw_complex *out;
	fftw_plan     plan;
	int    waveform_x        {0};
	int    waveform_y        {0};
	int    spectrum_x        {0};
	int    spectrum_y        {0};
	int    waterfall_x       {0};
	int    waterfall_y       {0};
	double bin_size        {0.0};
	double amax            {0.0}; /* waterfall pseudo color */
	double amin            {0.0}; /* waterfall pseudo color */
	int    timer_value       {0}; /* on_timeout() */
//	int    timer_value2      {0}; /* on_timeout2() */
	double timer_margin    {1.0}; /* reduce timer_value by this factor */
	snd_pcm_uframes_t   buffer_size          {0};
	snd_pcm_uframes_t   buffer_size_org      {0};
	snd_pcm_uframes_t   buffer_size_get      {0};
	snd_pcm_uframes_t   period_size          {0};
	snd_pcm_uframes_t   period_size_org      {0};
	snd_pcm_uframes_t   period_size_get      {0};
	int                 sub_unit_direction   {0};
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
