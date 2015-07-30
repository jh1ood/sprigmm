/*
 * Sound.cpp
 *
 *  Created on: Jul 4, 2015
 *      Author: user1
 */

#include "Sound.h"
#include <iostream>
using namespace std;

Sound::~Sound() {
	cout << "Sound::~Sound() detructor.." << endl;
}

int Sound::asound_init() {
	int err;

	cout << "Sound::asound_init(): channels = " << channels << endl;

	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);

	if ((err = snd_pcm_open(&handle, sound_device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		cout  << "snd_pcm_open() error. " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if ((err = asound_set_hwparams()) < 0) {
		cout  << "setting of hwparams failed. " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if ((err = asound_set_swparams()) < 0) {
		cout  << "setting of swparams failed. " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if ((err = snd_pcm_prepare(handle)) < 0) {
		cout  << "snd_pcm_prepare error: " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if (snd_pcm_state(handle) == SND_PCM_STATE_PREPARED) {
		if ((err = snd_pcm_start(handle)) < 0) {
			cout  << "pcm_start error: " << snd_strerror(err) << endl;
			exit(EXIT_FAILURE);
		}
	} else {
		cout  << "snd_pcm_state is not PREPARED" << endl;
		exit(EXIT_FAILURE);
	}

	return 0;
}

int Sound::asound_read() {
	static int count = 0;

	avail = snd_pcm_avail_update(handle);
	cout << "Sound::asound_read(): count = " << count++ << ", avail = " << avail << endl;

	if (avail == -EPIPE) {    /* under-run */
		cout << "Sound::asound_read(): -EPIPE error (overrun for capture) occurred, trying to recover now .." << endl;
		int err = snd_pcm_recover(handle, -EPIPE, 0);
		if (err < 0) {
			cout << "Sound::asound_read(): can not recover from -EPIPE error: " << snd_strerror(err) << endl;
		}
		avail = snd_pcm_avail_update(handle);
		cout << "Sound::asound_read(): avail after snd_pcm_recover() = " << avail << endl;

//		return 0;
	}

	int loop_count = 0;
	while (avail >= (snd_pcm_sframes_t) period_size) {
		frames_actually_read = snd_pcm_readi(handle, samples, period_size);
		cout << "Sound::asound_read(): loop_count = " << loop_count++ << ", frames_actually_read = " << frames_actually_read << endl;

		if (frames_actually_read < 0) {
			cout << "Sound::asound_read(): snd_pcm_readi error: " << snd_strerror(frames_actually_read) << endl;
			exit(EXIT_FAILURE);
		}
		if (frames_actually_read != (int) period_size) {
			cout << "Sound::asound_read(): frames_actually_read (" << frames_actually_read
					<< ") does not match the period_size (" << period_size << endl;
			exit(EXIT_FAILURE);
		}

		/* copy samples into audio_signal */
		for (int i = 0; i < (int) (period_size * channels); i++) {
			audio_signal[i] = samples[i];
		}
		avail = snd_pcm_avail_update(handle);
		cout << "Sound::asound_read(): " << "in the while loop, avail = " << avail << endl;
	}

	return loop_count;
}

int Sound::asound_set_hwparams() {
	unsigned int rrate;
	int err;
	cout << "Sound::asound_set_hwparams() begin... \n"
	     << "  channels = " << channels << endl;

	/* choose all parameters */
	err = snd_pcm_hw_params_any(handle, hwparams);
	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Broken configuration for playback: no configurations available."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set hardware resampling disabled */
	err = snd_pcm_hw_params_set_rate_resample(handle, hwparams, resample);
	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Resampling setup failed for playback."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set the interleaved read/write format */
	err = snd_pcm_hw_params_set_access(handle, hwparams,
			SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Access type not available for playback."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set the sample format */
	err = snd_pcm_hw_params_set_format(handle, hwparams, SND_PCM_FORMAT_S16);

	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Sample format not available for playback."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set the count of channels */
	err = snd_pcm_hw_params_set_channels(handle, hwparams, channels);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: channels = " << channels
				<< " is not available." << snd_strerror(err)
				<< endl;
		return err;
	}

	/* set the stream rate */
	rrate = rate;
	err = snd_pcm_hw_params_set_rate_near(handle, hwparams, &rrate, 0);
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
	err = snd_pcm_hw_params_set_buffer_size_near(handle, hwparams, &buffer_size);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: buffer size error = " << err << endl;
		return err;
	}

	/* set the period size */
	snd_pcm_hw_params_set_period_size_near(handle, hwparams, &period_size, NULL);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: period size error = " << err << endl;
		return err;
	}

	/* write the parameters to device */
	err = snd_pcm_hw_params(handle, hwparams);
	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Unable to set hw params for playback: "
		<< snd_strerror(err) << endl;
		return err;
	}

	return 0;
}

int Sound::asound_set_swparams() {
	int err;
	cout << "Sound::asound_set_swparams() begin... \n"
	     << "  channels = " << channels << endl;

	/* get the current swparams */
	err = snd_pcm_sw_params_current(handle, swparams);
	if (err < 0) {
		cout
		<< "Sound::asound_set_swparams: Unable to determine current swparams: "
		<< snd_strerror(err) << endl;
		return err;
	}

	/* start the transfer when the buffer is half full */
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, period_size * 2);
	if (err < 0) {
		cout
		<< "Sound::asound_set_swparams: Unable to set start threshold mode: "
		<< snd_strerror(err) << endl;
		return err;
	}

	/* allow the transfer when at least period_size samples can be processed */
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size * 2);
	if (err < 0) {
		cout
		<< "Sound::asound_set_swparams: Unable to set avail min: "
		<< snd_strerror(err) << endl;
		return err;
	}

	/* write the parameters to the device */
	err = snd_pcm_sw_params(handle, swparams);
	if (err < 0) {
		cout
		<< "Sound::asound_set_swparams: Unable to set sw params: "
		<< snd_strerror(err) << endl;
		return err;
	}

	return 0;
}
