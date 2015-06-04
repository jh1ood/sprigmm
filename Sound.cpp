/*
 * Sound.cpp
 *
 *  Created on: Apr 15, 2015
 *      Author: user1
 */

#include "Sound.h"
#include "mydefine.h"
#include <fftw3.h>
#include <iostream>
#include <string>
#include <cmath>
#include <time.h>
#include <sys/time.h>
#include <ctime>
#include <iomanip>
using namespace std;

//extern Sound *mysound[];
extern struct timeval t0;

void asound_async_callback0_wrapper(snd_async_handler_t * ahandler) {
//	mysound[0]->asound_async_callback(ahandler);
}

void asound_async_callback1_wrapper(snd_async_handler_t * ahandler) {
//	mysound[1]->asound_async_callback(ahandler);
}

Sound::Sound(char *s, int n) : sound_device(s), channels(n) {

	cout << "Sound constructor is called." << endl;

	int err = 0;
	string myid;

	if(0) {
	} else if(channels == 1) {
		myid = "Sound::Sound(0): ";
		rate        = 32000;
		buffer_size = 32 * 1024;
		period_size =  8 * 1024;
	} else if(channels == 2) {
		myid = "Sound::Sound(1)";
		rate        = 48000;
		buffer_size = 96 * 1024 * 10;
		period_size =  2 * 1024;
	} else {
		cout << "Sound::Sound: ERROR, channels (" << channels << ") should be 1 or 2." << endl;
		exit(EXIT_FAILURE);
	}

	cout << myid << "sound_device = " << sound_device
			<< ", rate = " << rate
			<< ", channels = " << channels
			<< ", buffer_size = " << buffer_size
			<< ", period_size = " << period_size
			<< endl;

	samples            = new signed short[period_size * channels * 2];
	audio_signal       = new double      [period_size * channels * 2];

	cout << myid << "sound_device setup begin.. sound_device = " << sound_device << endl;

	snd_pcm_hw_params_alloca(&hwparams);
	snd_pcm_sw_params_alloca(&swparams);

	if ((err = snd_pcm_open(&handle, sound_device, SND_PCM_STREAM_CAPTURE, 0)) < 0) {
		cout << myid << "snd_pcm_open() error. " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if ((err = asound_set_hwparams(handle, hwparams)) < 0) {
		cout << myid << "setting of hwparams failed. " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if ((err = asound_set_swparams(handle, swparams)) < 0) {
		cout << myid << "setting of swparams failed. " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

	if(0) {
	} else if(channels == 1) {
		if ((err = snd_async_add_pcm_handler(&ahandler, handle, asound_async_callback0_wrapper, samples)) < 0) {
			cout << myid << "snd_async_add_pcm_handler failed. " << snd_strerror(err) << endl;
			exit(EXIT_FAILURE);
		}
	} else if(channels == 2) {
		if ((err = snd_async_add_pcm_handler(&ahandler, handle, asound_async_callback1_wrapper, samples)) < 0) {
			cout << myid << "snd_async_add_pcm_handler failed. " << snd_strerror(err) << endl;
			exit(EXIT_FAILURE);
		}
	}

	if ((err = snd_pcm_prepare(handle)) < 0) {
		cout << myid << "snd_pcm_prepare error: " << snd_strerror(err) << endl;
		exit(EXIT_FAILURE);
	}

//	if (snd_pcm_state(handle) == SND_PCM_STATE_PREPARED) {
//		if ((err = snd_pcm_start(handle)) < 0) {
//			cout << myid << "pcm_start error: " << snd_strerror(err) << endl;
//			exit(EXIT_FAILURE);
//		}
//	}

	cout << "Sound constructor end.." << endl;
}

void Sound::Sound_go() {

	int err = 0;
	string myid;

	if(0) {
	} else if(channels == 1) {
		myid = "Sound::Sound_go(0): ";
	} else if(channels == 2) {
		myid = "Sound::Sound_go(1): ";
	} else {
		cout << "Sound::Sound_go: ERROR, channels (" << channels << ") should be 1 or 2." << endl;
		exit(EXIT_FAILURE);
	}
	cout << myid << "is called." << endl;

	if (snd_pcm_state(handle) == SND_PCM_STATE_PREPARED) {
		if ((err = snd_pcm_start(handle)) < 0) {
			cout << myid << "pcm_start error: " << snd_strerror(err) << endl;
			exit(EXIT_FAILURE);
		}
	}
}

Sound::~Sound() {
	string myid;

	if(0) {
	} else if(channels == 1) {
		myid = "Sound::~Sound(1): ";
	} else if(channels == 2) {
		myid = "Sound::~Sound(2): ";
	}

	cout << myid << "destructor is called." << endl;

	delete[] samples;
	delete[] audio_signal;

}

void Sound::asound_async_callback(snd_async_handler_t * ahandler) {
	string myid;
	static int icount[3] = {0, 0, 0}; /* [0] is dummy */
	static struct timeval t1[3], t1b4[3], t9[3];

	gettimeofday(&t1[channels], NULL);

	if(0) {
	} else if(channels == 1) {
		myid = "Sound::asound_async_callback(1): ";
	} else if(channels == 2) {
		myid = "Sound::asound_async_callback(2): ";
	}

	cout << myid << "icount[" << channels << "] = " << icount[channels]++ << endl;

	snd_pcm_t    *handle  = snd_async_handler_get_pcm(ahandler);
	signed short *samples = (signed short*) snd_async_handler_get_callback_private(ahandler);

	avail = snd_pcm_avail_update(handle);
	cout << myid << "avail = " << avail << endl;

	if (avail == -EPIPE) {    /* under-run */
		cout << myid << "underrun occurred, trying to recover now .." << endl;
		int err = snd_pcm_prepare(handle);
		if (err < 0) {
			cout << myid << "can not recover from underrun: " << snd_strerror(err) << endl;
		}
		cout << myid << "return here because underrun occurred." << endl;
		return;
	}

	while (avail >= (snd_pcm_sframes_t) period_size) {
		frames_actually_read = snd_pcm_readi(handle, samples, period_size);
		cout << myid << "frames_actually_read = " << frames_actually_read << endl;

		if (frames_actually_read < 0) {
			cout << myid << "snd_pcm_readi error: " << snd_strerror(frames_actually_read) << endl;
			exit(EXIT_FAILURE);
		}
		if (frames_actually_read != (int) period_size) {
			cout << myid << "frames_actually_read (" << frames_actually_read
					<< ") does not match the period_size (" << period_size << endl;
			exit(EXIT_FAILURE);
		}

		/* copy samples into audio_signal */
		for (int i = 0; i < (int) (period_size * channels); i++) {
			audio_signal[i] = samples[i];
		}
		avail = snd_pcm_avail_update(handle);
		cout << myid << "in the while loop, avail = " << avail << endl;
	}

	gettimeofday(&t9[channels], NULL);
	cout_gettimeofday_diff(myid+"elapsed:  ", t0            , t1[channels]);
	cout_gettimeofday_diff(myid+"interval: ", t1b4[channels], t1[channels]);
	cout_gettimeofday_diff(myid+"duration: ", t1  [channels], t9[channels]);
	t1b4[channels] = t1[channels];

}

bool Sound::asound_myread() {
	string myid;
	static int icount[3] = {0, 0, 0}; /* [0] is dummy */
	static struct timeval t1[3], t1b4[3], t9[3];

	gettimeofday(&t1[channels], NULL);

	if(0) {
	} else if(channels == 1) {
		myid = "Sound::asound_myread(1): ";
	} else if(channels == 2) {
		myid = "Sound::asound_myread(2): ";
	}

	cout << myid << "icount[" << channels << "] = " << icount[channels]++ << endl;

//	snd_pcm_t    *handle  = snd_async_handler_get_pcm(ahandler);
//	signed short *samples = (signed short*) snd_async_handler_get_callback_private(ahandler);

	avail = snd_pcm_avail_update(handle);
	cout << myid << "avail = " << avail << endl;

	if (avail == -EPIPE) {    /* under-run */
		cout << myid << "underrun occurred, trying to recover now .." << endl;
//		int err = snd_pcm_prepare(handle);
		int err = snd_pcm_recover(handle, -EPIPE, 0);
		if (err < 0) {
			cout << myid << "can not recover from underrun: " << snd_strerror(err) << endl;
		}
		cout << myid << "return here because underrun occurred." << endl;
//		return false;
	}

	int loop_count = 0;
	while (avail >= (snd_pcm_sframes_t) period_size) {
		frames_actually_read = snd_pcm_readi(handle, samples, period_size);
		cout << myid << "loop_count = " << loop_count++ << ", frames_actually_read = " << frames_actually_read << endl;

		if (frames_actually_read < 0) {
			cout << myid << "snd_pcm_readi error: " << snd_strerror(frames_actually_read) << endl;
			exit(EXIT_FAILURE);
		}
		if (frames_actually_read != (int) period_size) {
			cout << myid << "frames_actually_read (" << frames_actually_read
					<< ") does not match the period_size (" << period_size << endl;
			exit(EXIT_FAILURE);
		}

		/* copy samples into audio_signal */
		for (int i = 0; i < (int) (period_size * channels); i++) {
			audio_signal[i] = samples[i];
		}
		avail = snd_pcm_avail_update(handle);
		cout << myid << "in the while loop, avail = " << avail << endl;
	}

	gettimeofday(&t9[channels], NULL);
	cout_gettimeofday_diff(myid+"elapsed:  ", t0            , t1[channels]);
	cout_gettimeofday_diff(myid+"interval: ", t1b4[channels], t1[channels]);
	cout_gettimeofday_diff(myid+"duration: ", t1  [channels], t9[channels]);
	t1b4[channels] = t1[channels];

	if(loop_count > 0) {
		return true;
	} else {
		return false;
	}

}


int Sound::asound_set_hwparams(snd_pcm_t * handle, snd_pcm_hw_params_t * params) {
	unsigned int rrate;
	int err;
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
	err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16);

	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Sample format not available for playback."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set the count of channels */
	err = snd_pcm_hw_params_set_channels(handle, params, channels);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: channels = " << channels
				<< " is not available." << snd_strerror(err)
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
