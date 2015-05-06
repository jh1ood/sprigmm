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
	rate         = atoi(r);
	channels     = atoi(c);

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

	if (0) {
	} else if (channels == 1) {
		buffer_size = 64 * 1024;
		period_size = 16 * 1024;
		nfft = 8192;
	} else if (channels == 2) {
		buffer_size = 64 * 1024;
		period_size =  4 * 1024;
		nfft = 2048;
	}

	bin_size = rate / (double) nfft;
	cout << "Sound::Sound: nfft = " << nfft << ", bin_size = " << bin_size << endl;

	for (int i = 0; i < nfft; i++) {
		fft_window[i] = 0.54 - 0.46 * cos(2.0 * M_PI * i / (double) nfft);
	}

	in  = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft);
	out = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft);
	p = fftw_plan_dft_1d(nfft, in, out, FFTW_FORWARD, FFTW_MEASURE);
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

	snd_pcm_t    *handle  = snd_async_handler_get_pcm(ahandler);
	signed short *samples = (signed short*) snd_async_handler_get_callback_private(ahandler);

	avail = snd_pcm_avail_update(handle);
	cout << "asound_async_callback: avail = " << avail << endl;

	if (avail == -EPIPE) {    /* under-run */
    		cout << "asound_asysnc_callback: underrun occured, trying to recover now .." << endl;
            int err = snd_pcm_prepare(handle);
            if (err < 0) {
            	cout << "asound_asysnc_callback: can not recover from underrun: " << snd_strerror(err) << endl;
            }
	}

	while (avail >= (snd_pcm_sframes_t) period_size) {
		frames_actually_read = snd_pcm_readi(handle, samples, period_size);
		cout << "asound_async_callback: frames_actually_read = " << frames_actually_read << endl;

		if (frames_actually_read < 0) {
			cout << "asound_async_callback: snd_pcm_readi error: " << snd_strerror(frames_actually_read) << endl;
			exit(EXIT_FAILURE);
		}
		if (frames_actually_read != (int) period_size) {
			cout << "asound_async_callback: frames_actually_read (" << frames_actually_read
				 << ") does not match the period_size (" << period_size << endl;
			exit(EXIT_FAILURE);
		}

		/*** begin audio signal processing ***/
		audio_signal_max = -32768;
		if (0) {
		} else if (channels == 1) {
			for (int i = 0; i < nfft; i++) {
				audio_signal[i] = samples[i];
				if (audio_signal_max < audio_signal[i])
					audio_signal_max = audio_signal[i];
			}
		} else if (channels == 2) { /* for my Soft66LC4 only */
			dc0 = 0.0;
			dc1 = 0.0;
			for (int i = 0; i < nfft * 2; i += 2) {
				dc0 += samples[i];
				dc1 += samples[i + 1];
			}
			dc0 /= (double) nfft;
			dc1 /= (double) nfft;
//			dc0 = 0.0; dc1 = 0.0; /* ignore DC offset cancellation */
			for (int i = 0; i < nfft * 2; i += 2) {
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
		for (int i = 0; i < nfft; i++) {
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

		for (int i = 0; i < nfft; i++) {
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
		/*** end audio signal processing */

		avail = snd_pcm_avail_update(handle);
		cout << "asound_async_callback: avail again = " << avail << endl;
	}

/* the followings should be in the above while loop?? */

}

int Sound::asound_set_hwparams(snd_pcm_t * handle, snd_pcm_hw_params_t * params) {
	unsigned int rrate;
	int err;

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
	if(channels == 10) { /* for PrismSound Orpheus only */
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

	/* set the buffer size */
	err = snd_pcm_hw_params_set_buffer_size_near(handle, params, &buffer_size);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: buffer size error = " << err << endl;
		return err;
	}

	/* set the period size */
	snd_pcm_hw_params_set_period_size_near(handle, params, &period_size, NULL);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: period size error = " << err << endl;
		return err;
	}

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

	/* start the transfer when the buffer is half full */
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, buffer_size / 2);
	if (err < 0) {
		cout
				<< "Sound::asound_set_swparams: Unable to set start threshold mode for playback: "
				<< snd_strerror(err) << endl;
		return err;
	}

	/* allow the transfer when at least period_size samples can be processed */
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size * 2);
	if (err < 0) {
		cout
				<< "Sound::asound_set_swparams: Unable to set avail min for playback: "
				<< snd_strerror(err) << endl;
		return err;
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
