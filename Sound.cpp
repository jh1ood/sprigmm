/*
 * Sound.cpp
 *
 *  Created on: Apr 15, 2015
 *      Author: user1
 */

#include "Sound.h"
#include <fftw3.h>
#include <iostream>
#include <string>
#include <cmath>
using namespace std;

//void async_callback(snd_async_handler_t * ahandler);
//int async_loop(snd_pcm_t * handle, signed short *samples);
//void asound_async_callback(snd_async_handler_t * ahandler);
//void asound_async_callback2(snd_async_handler_t * ahandler);

//extern double audio_signal[NFFT];

extern Sound *mysound1;
extern Sound *mysound2;

void asound_callback1_wrapper(snd_async_handler_t * ahandler) {
	mysound1->asound_async_callback(ahandler);
}

void asound_callback2_wrapper(snd_async_handler_t * ahandler) {
	mysound2->asound_async_callback(ahandler);
}

Sound::Sound(char *s, const char *r, const char *c) {

	sound_device = s;
	rate = atoi(r);
	channels = atoi(c);

	cout << "Sound::Sound: sound_device = " << sound_device << ", rate = "
			<< rate << ", channels = " << channels << endl;
	int err = 0;

	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);

	if ((err = snd_pcm_open(&handle, sound_device, SND_PCM_STREAM_CAPTURE, 0))
			< 0) {
		cout << "Sound::Sound: snd_pcm_open() error. " << snd_strerror(err)
				<< endl;
		exit(EXIT_FAILURE);
	}

	if ((err = asound_set_hwparams(handle, hwparams)) < 0) {
		cout << "Sound::Sound: setting of hwparams failed. "
				<< snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if ((err = asound_set_swparams(handle, swparams)) < 0) {
		cout << "Sound::Sound: setting of swparams failed. "
				<< snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	nsamples = period_size * channels * byte_per_sample;
	cout << "Sound::Sound: nsamples = " << nsamples << endl;
	cout << "Sound::Sound: samples  = " << samples << endl;

	if (0) {
	} else if (channels == 1) {
		err = snd_async_add_pcm_handler(&ahandler, handle,
				asound_callback1_wrapper, samples); /* callback */
	} else if (channels == 2) {
		err = snd_async_add_pcm_handler(&ahandler, handle,
				asound_callback2_wrapper, samples); /* callback */
	}

	if (err < 0) {
		cout << "Sound::Sound: Unable to register async handler. \n";
		exit(EXIT_FAILURE);
	}

	if (snd_pcm_state(handle) == SND_PCM_STATE_PREPARED) {
		err = snd_pcm_start(handle);
		if (err < 0) {
			cout << "Sound::Sound: Start error: " << snd_strerror(err) << endl;
			exit(EXIT_FAILURE);
		}
	}

	bin_size = rate / (double) NFFT;
	for (int i = 0; i < NFFT; i++) {
		fft_window[i] = 0.54 - 0.46 * cos(2.0 * M_PI * i / (double) NFFT);
	}

	in = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * NFFT);
	out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * NFFT);
	p = fftw_plan_dft_1d(NFFT, in, out, FFTW_FORWARD, FFTW_MEASURE);

	cout << "Sound::Sound: end.. \n";

}

Sound::~Sound() {
}

extern int flag_togo1, flag_togo2, flag_togo3, flag_togo4;

void Sound::asound_async_callback(snd_async_handler_t * ahandler) {
	static int icount1 = 0, icount2 = 0;

	if (0) {
	} else if (channels == 1) {
		flag_togo1 = 1; /* to activate DrawArea::on_draw()  */
		flag_togo2 = 1; /* to activate Waterfall::on_draw() */
		cout << "asound_async_callback: icount1 = " << icount1++ << endl;
	} else if (channels == 2) {
		flag_togo3 = 1; /* to activate DrawArea::on_draw()  */
		flag_togo4 = 1; /* to activate Waterfall::on_draw() */
		cout << "asound_async_callback: icount2 = " << icount2++ << endl;
	}

	snd_pcm_t *handle = snd_async_handler_get_pcm(ahandler);
	signed short *samples = snd_async_handler_get_callback_private(ahandler);

	cout << "asound_async_callback: samples     = " << samples << endl;

	snd_pcm_sframes_t avail;
	int err;

	avail = snd_pcm_avail_update(handle);
	cout << "asound_async_callback: avail       = " << avail << endl;
	cout << "asound_async_callback: period_size = " << period_size << endl;

	while (avail >= period_size) {
		err = snd_pcm_readi(handle, samples, period_size);
		cout << "asound_async_callback: err         = " << err << endl;

		if (err < 0) {
			fprintf(stderr, "Write error: %s\n", snd_strerror(err));
			exit(EXIT_FAILURE);
		}
		if (err != period_size) {
			fprintf(stderr, "Write error: written %i expected %li\n", err,
					period_size);
			exit(EXIT_FAILURE);
		}

		avail = snd_pcm_avail_update(handle);
		cout << "asound_async_callback: avail again = " << avail << endl;
	}

	audio_signal_max = -32768;
	if (0) {
	} else if (channels == 1) {
		for (int i = 0; i < NFFT; i++) { /* NFFT=period_size */
			audio_signal[i] = samples[i];
			if (audio_signal_max < audio_signal[i])
				audio_signal_max = audio_signal[i];
		}
	} else if (channels == 2) { /* for my Soft66LC4 only */
		dc0 = 0.0;
		dc1 = 0.0;
		for (int i = 0; i < NFFT * 2; i += 2) {
			dc0 += samples[i];
			dc1 += samples[i + 1];
		}
		dc0 /= (double) NFFT;
		dc1 /= (double) NFFT;
		dc0 = 0.0; dc1 = 0.0; /* ignore DC offset cancellation */
		for (int i = 0; i < NFFT * 2; i += 2) {
			double i1 = samples[i]     - dc0; /* DC offset */
			double q1 = samples[i + 1] - dc1;
			double i2 = i1;
			double q2 = -0.32258 * i1 + 1.1443 * q1; /* gain and phase correction */
			double i3 = q2; /* swap IQ */
			double q3 = i2;
			audio_signal[i] = i3;
			audio_signal[i + 1] = q3;
			if (audio_signal_max < audio_signal[i])
				audio_signal_max = audio_signal[i];
			if (audio_signal_max < audio_signal[i+1])
				audio_signal_max = audio_signal[i+1];
		}
		cout << "asound_async_callback: dc0 = " << dc0 << ", dc1 = " << dc1 << endl;
	}
	cout << "asound_async_callback: audio_signal_max = " << audio_signal_max
			<< " for channels = " << channels << endl;

	/* audio signal FFT */
	for (int i = 0; i < NFFT; i++) {
		if (0) {
		} else if (channels == 1) {
			in[i][0] = fft_window[i] * audio_signal[i];
			in[i][1] = 0.0;
		} else if (channels == 2) {
			in[i][0] = fft_window[i] * audio_signal[2 * i];
			in[i][1] = fft_window[i] * audio_signal[2 * i + 1];
		}
	}

	fftw_execute(p);

	/* log10 and normalize */

	if (channels == 1) {
		amax = 14.0;
		amin = 7.0;
	} else if (channels == 2) {
		amax = 12.0;
		amin = 7.0;
	}

	for (int i = 0; i < NFFT; i++) {
		double val;
		val = out[i][0] * out[i][0] + out[i][1] * out[i][1];
		if (val < pow(10.0, amin)) {
			audio_signal_ffted[i] = 0.0;
		} else if (val > pow(10.0, amax)) {
			audio_signal_ffted[i] = 1.0;
		} else {
			audio_signal_ffted[i] = (log10(val) - amin) / (amax - amin);
		}
	}
	cout << "asound_async_callback: done fftw, etc. \n";

	cout << "asound_async_callback: end.." << endl;
}

int Sound::asound_set_hwparams(snd_pcm_t * handle,
		snd_pcm_hw_params_t * params) {
	unsigned int rrate;
	snd_pcm_uframes_t size;
	int err, dir;
	cout << "Sound::asound_set_hwparams: begin... \n";

	/* choose all parameters */
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		cout
				<< "Sound::asound_set_hwparams: Broken configuration for playback: no configurations available."
				<< snd_strerror(err) << endl;
		return err;
	}

	/* set hardware resampling disabled */
	err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
	if (err < 0) {
		cout
				<< "Sound::asound_set_hwparams: Resampling setup failed for playback."
				<< snd_strerror(err) << endl;
		return err;
	}

	/* set the interleaved read/write format */
	err = snd_pcm_hw_params_set_access(handle, params,
			SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		cout
				<< "Sound::asound_set_hwparams: Access type not available for playback."
				<< snd_strerror(err) << endl;
		return err;
	}

	/* set the sample format */
	if(channels == 10) {
		err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_LE);

	} else {
		err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16);
	}

	if (err < 0) {
		cout
				<< "Sound::asound_set_hwparams: Sample format not available for playback."
				<< snd_strerror(err) << endl;
		return err;
	}

	/* set the count of channels */
	err = snd_pcm_hw_params_set_channels(handle, params, channels);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: Channels count " << channels
				<< " is not available for playbacks." << snd_strerror(err)
				<< endl;
		return err;
	}

	/* set the stream rate */
	rrate = rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: Rate " << rate
				<< " is not available for playbacks." << snd_strerror(err)
				<< endl;
		return err;
	}
	if (rrate != rate) {
		cout << "Sound::asound_set_hwparams: Rate does not match. Requested "
				<< rate << ", but get " << err << endl;
		return -EINVAL;
	}

	/* set the buffer time */
	err = snd_pcm_hw_params_set_buffer_time_near(handle, params, &buffer_time,
			&dir);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: Unable to set buffer time "
				<< buffer_time << "i for playback: " << snd_strerror(err)
				<< endl;
		return err;
	}
	cout << "Sound::asound_set_hwparams: buffer_time = " << buffer_time
			<< ", dir   = " << dir << endl;

	err = snd_pcm_hw_params_get_buffer_size(params, &size);
	if (err < 0) {
		cout
				<< "Sound::asound_set_hwparams: Unable to get buffer size for playback: "
				<< snd_strerror(err) << endl;
		return err;
	}
	buffer_size = size;
	cout << "Sound::asound_set_hwparams: buffer_size = " << buffer_size << endl;

	/* set the period time */
	err = snd_pcm_hw_params_set_period_time_near(handle, params, &period_time,
			&dir);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: Unable to set period time "
				<< period_time << " for playback: " << snd_strerror(err)
				<< endl;
		return err;
	}
	cout << "Sound::asound_set_hwparams: period_time = " << period_time
			<< " , dir   = " << dir << endl;

	err = snd_pcm_hw_params_get_period_size(params, &size, &dir);
	if (err < 0) {
		cout
				<< "Sound::asound_set_hwparams: Unable to get period size for playback: "
				<< snd_strerror(err) << endl;
		return err;
	}

	period_size = size;
	cout << "Sound::asound_set_hwparams: period_size = " << period_size
			<< ", dir = " << dir << endl;

//	if (period_size < NFFT * channels) {
//		cout << "Sound::asound_set_hwparams: period_size = " << period_sizekk
//				<< ", but less than NFFT (" << NFFT << ") times channels ("
//				<< channels << ")." << endl;
//		exit(1);
//	}

	/* write the parameters to device */
	err = snd_pcm_hw_params(handle, params);
	if (err < 0) {
		cout
				<< "Sound::asound_set_hwparams: Unable to set hw params for playback: "
				<< snd_strerror(err) << endl;
		return err;
	}

	return 0;
}

int Sound::asound_set_swparams(snd_pcm_t * handle,
		snd_pcm_sw_params_t * swparams) {
	int err;
	cout << "Sound::asound_set_swparams: begin... \n";

	/* get the current swparams */
	err = snd_pcm_sw_params_current(handle, swparams);
	if (err < 0) {
		cout
				<< "Sound::asound_set_swparams: Unable to determine current swparams for playback: "
				<< snd_strerror(err) << endl;
		return err;
	}

	/* start the transfer when the buffer is almost full: */
	/* (buffer_size / avail_min) * avail_min */
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams,
			(buffer_size / period_size) * period_size);
	if (err < 0) {
		cout
				<< "Sound::asound_set_swparams: Unable to set start threshold mode for playback: "
				<< snd_strerror(err) << endl;
		return err;
	}

	/* allow the transfer when at least period_size samples can be processed */
	/* or disable this mechanism when period event is enabled (aka interrupt like style processing) */
	err = snd_pcm_sw_params_set_avail_min(handle, swparams,
			period_event ? buffer_size : period_size);
	if (err < 0) {
		cout
				<< "Sound::asound_set_swparams: Unable to set avail min for playback: "
				<< snd_strerror(err) << endl;
		return err;
	}

	/* enable period events when requested */
	if (period_event) {
		err = snd_pcm_sw_params_set_period_event(handle, swparams, 1);
		if (err < 0) {
			cout << "Sound::asound_set_swparams: UUnable to set period event: "
					<< snd_strerror(err) << endl;
			return err;
		}
	}

	/* write the parameters to the playback device */
	err = snd_pcm_sw_params(handle, swparams);
	if (err < 0) {
		cout
				<< "Sound::asound_set_swparams: Unable to set sw params for playback: "
				<< snd_strerror(err) << endl;
		return err;
	}

	return 0;
}
