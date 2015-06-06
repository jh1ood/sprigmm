/*
 * Usbsound.cpp
 *
 *  Created on: Jun 4, 2015
 *      Author: user1
 */

#include "Usbsound.h"
#include <time.h>
#include <sys/time.h>
#include <ctime>
extern struct timeval t0;
void cout_gettimeofday_diff(string, timeval, timeval);

Usbsound::Usbsound() {
	cout << myid << "default constructor begin.." << endl;
}

Usbsound::Usbsound(char* s) : sound_device{s} {

	cout << myid << "constructor with char* begin.." << endl;
	cout << myid << "sound_device = " << sound_device << endl;
	cout << myid << "constructor with char* end.." << endl;

}

Usbsound::~Usbsound() {
	cout << "usbsound destructor"  << endl;
}

int Usbsound::asound_set_hwparams(snd_pcm_t * handle, snd_pcm_hw_params_t * params) {
	unsigned int rrate;
	int err;
	cout << "Usbsound::asound_set_hwparams: begin... \n";

	/* choose all parameters */
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		cout
		<< "Usbsound::asound_set_hwparams: Broken configuration for playback: no configurations available."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set hardware resampling disabled */
	err = snd_pcm_hw_params_set_rate_resample(handle, params, resample);
	if (err < 0) {
		cout
		<< "Usbsound::asound_set_hwparams: Resampling setup failed for playback."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set the interleaved read/write format */
	err = snd_pcm_hw_params_set_access(handle, params,
			SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		cout
		<< "Usbsound::asound_set_hwparams: Access type not available for playback."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set the sample format */
	err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16);

	if (err < 0) {
		cout
		<< "Usbsound::asound_set_hwparams: Sample format not available for playback."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set the count of channels */
	err = snd_pcm_hw_params_set_channels(handle, params, channels);
	if (err < 0) {
		cout << "Usbsound::asound_set_hwparams: channels = " << channels
				<< " is not available." << snd_strerror(err)
				<< endl;
		return err;
	}

	/* set the stream rate */
	rrate = rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
	if (err < 0) {
		cout << "Usbsound::asound_set_hwparams: Rate " << rate
				<< " is not available for playbacks." << snd_strerror(err)
				<< endl;
		return err;
	}
	if (rrate != rate) {
		cout << "Usbsound::asound_set_hwparams: Rate does not match. Requested "
				<< rate << ", but get " << err << endl;
		return -EINVAL;
	}

	/* set the buffer size */
	err = snd_pcm_hw_params_set_buffer_size_near(handle, params, &buffer_size);
	if (err < 0) {
		cout << "Usbsound::asound_set_hwparams: buffer size error = " << err << endl;
		return err;
	}

	/* set the period size */
	snd_pcm_hw_params_set_period_size_near(handle, params, &period_size, NULL);
	if (err < 0) {
		cout << "Usbsound::asound_set_hwparams: period size error = " << err << endl;
		return err;
	}

	/* write the parameters to device */
	err = snd_pcm_hw_params(handle, params);
	if (err < 0) {
		cout
		<< "Usbsound::asound_set_hwparams: Unable to set hw params for playback: "
		<< snd_strerror(err) << endl;
		return err;
	}

	return 0;
}

int Usbsound::asound_set_swparams(snd_pcm_t * handle, snd_pcm_sw_params_t * swparams) {
	int err;
	cout << "Usbsound::asound_set_swparams: begin... \n";

	/* get the current swparams */
	err = snd_pcm_sw_params_current(handle, swparams);
	if (err < 0) {
		cout
		<< "Usbsound::asound_set_swparams: Unable to determine current swparams: "
		<< snd_strerror(err) << endl;
		return err;
	}

	/* start the transfer when the buffer is half full */
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, period_size * 2);
	if (err < 0) {
		cout
		<< "Usbsound::asound_set_swparams: Unable to set start threshold mode: "
		<< snd_strerror(err) << endl;
		return err;
	}

	/* allow the transfer when at least period_size samples can be processed */
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size * 2);
	if (err < 0) {
		cout
		<< "Usbsound::asound_set_swparams: Unable to set avail min: "
		<< snd_strerror(err) << endl;
		return err;
	}

	/* write the parameters to the device */
	err = snd_pcm_sw_params(handle, swparams);
	if (err < 0) {
		cout
		<< "Usbsound::asound_set_swparams: Unable to set sw params: "
		<< snd_strerror(err) << endl;
		return err;
	}

	return 0;
}

void Usbsound::Usbsound_go() {

	int err = 0;
	string myid;

	if(0) {
	} else if(channels == 1) {
		myid = "Sound::Sound_go(mono): ";
	} else if(channels == 2) {
		myid = "Sound::Sound_go(iq): ";
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

bool Usbsound::asound_myread() {
	string myid;
	static int icount[3] = {0, 0, 0}; /* [0] is dummy */
	static struct timeval t1[3], t1b4[3], t9[3];

	gettimeofday(&t1[channels], NULL);

	if(0) {
	} else if(channels == 1) {
		myid = "Sound::asound_myread(mono): ";
	} else if(channels == 2) {
		myid = "Sound::asound_myread(iq): ";
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
