/*
 * Sound.cpp
 */

#include "Sound.h"
#include <iostream>
#include <cmath>
#include <chrono>
using namespace std;

Sound::~Sound() {
	cout << "Sound::~Sound() destructor.." << endl;
}

int Sound::asound_init() {
	int err;

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

	cout << "Sound::asound_init(): done" << endl;

	return 0;
}

int Sound::asound_read() {
	int err;

	//	static int    count  =   0;
	//	static double phase1 = 0.0;
	//	static double phase2 = 0.0;
	//	static double phase3 = 0.0;
	//	static double phase4 = 0.0;

	avail = snd_pcm_avail_update(handle);
	cout << "Sound::asound_read(): avail = " << avail << endl;

	while (avail == -EPIPE) {    /* under-run */
		cout << "Sound::asound_read(): -EPIPE error (overrun for capture) occurred, trying to recover now .." << endl;

		err = snd_pcm_prepare(handle);
		cout << "Sound::asound_read(): snd_pcm_prepare returns with = " << err << endl;
		if(err > 0) {
			cout << "Sound::asound_read(): can not recover from overrun, snd_pcm_prepare failed." << endl;
		}
		avail = snd_pcm_avail_update(handle);
		cout << "Sound::asound_read(): now avail = " << avail << endl;

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

	int loop_count = 0;
	while (avail >= (snd_pcm_sframes_t) period_size) {
		loop_count++;
		frames_actually_read = snd_pcm_readi(handle, samples, period_size);

		if (frames_actually_read < 0) { /* this sometimes happens */
			cout << "Sound::asound_read(): snd_pcm_readi error: " << snd_strerror(frames_actually_read) << endl;
			cout << "Sound::asound_read(): snd_pcm_readi error: frames_actually_read = " << frames_actually_read << endl;

			if(frames_actually_read == -EPIPE) {
				err = snd_pcm_prepare(handle);
				cout << "Sound::asound_read(): snd_pcm_prepare returns with = " << err << endl;
				if(err > 0) {
					cout << "Sound::asound_read(): can not recover from overrun, snd_pcm_prepare failed." << endl;
				}
				avail = snd_pcm_avail_update(handle);
				cout << "Sound::asound_read(): now avail = " << avail << endl;

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
			} else {
				cout << "error not -EPIPE occurred" << endl;
				exit(EXIT_FAILURE);
			}
		}

		if (frames_actually_read != (int) period_size) {
			cout << "Sound::asound_read(): frames_actually_read (" << frames_actually_read
					<< ") does not match the period_size (" << period_size << endl;
			exit(EXIT_FAILURE);
		}

		/* copy samples into audio_signal */

		for (int i = 0; i < (int) (period_size * channels); i++) {
			*signal_end++ = samples[i];
			//			*signal_end++ = 16384.0 * ( sin(phase1) + 0.0*sin(phase2+0.1234) + 0.0*sin(phase3+0.5678) + 0.0*sin(phase4+0.101) );
			//			phase1 += 0.3; phase2 += 0.05; phase3 += 0.13; phase4 += 0.15;
		}

		avail = snd_pcm_avail_update(handle);
	}

	return loop_count;
}

int Sound::asound_fftcopy() {

	if(signal_end - signal_start >= nfft*channels) { /* this should always be true */
		auto p = signal_start;
		switch (channels) {
		case 1:
			for (int i = 0; i < nfft; i++) {
				in[i][0] = *p++ * audio_window[i];
				in[i][1] = 0.0; /* no imaginary part */
			}
			break;
		case 2:
			for (int i = 0; i < nfft; i++) {
				in[i][1] = *p++ * audio_window[i]; /* reverse I and Q */
				in[i][0] = *p++ * audio_window[i]; /* reverse I and Q */
			}
			break;
		default:
			exit(1);
		}
		return 0;
	} else { /* should never happen */
		cout << "Sound::asound_fftcopy((): error " << endl;
		exit(1);
	}
}

int Sound::asound_set_hwparams() {
	unsigned int rrate;
	int err;

	/* choose all parameters */
	err = snd_pcm_hw_params_any(handle, hwparams);
	cout << "snd_pcm_hw_params_any: err = " << err << endl;
	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Broken configuration for playback: no configurations available."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set hardware resampling disabled */
	err = snd_pcm_hw_params_set_rate_resample(handle, hwparams, resample);
	cout << "snd_pcm_hw_params_set_rate_resample: err = " << err << endl;
	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Resampling setup failed for playback."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set the interleaved read/write format */
	err = snd_pcm_hw_params_set_access(handle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
	cout << "snd_pcm_hw_params_set_access: err = " << err << endl;
	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Access type not available for playback."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set the sample format */
	err = snd_pcm_hw_params_set_format(handle, hwparams, SND_PCM_FORMAT_S16);
	cout << "snd_pcm_hw_params_set_format: err = " << err << endl;
	if (err < 0) {
		cout
		<< "Sound::asound_set_hwparams: Sample format not available for playback."
		<< snd_strerror(err) << endl;
		return err;
	}

	/* set the count of channels */
	err = snd_pcm_hw_params_set_channels(handle, hwparams, channels);
	cout << "snd_pcm_hw_params_set_channels: err = " << err << endl;
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: channels = " << channels
				<< " is not available." << snd_strerror(err)
				<< endl;
		return err;
	}

	/* set the stream rate */
	rrate = rate;
	err = snd_pcm_hw_params_set_rate_near(handle, hwparams, &rrate, 0);
	cout << "snd_pcm_hw_params_set_rate_near: err = " << err << endl;
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
	buffer_size_org = buffer_size; /* save the original value */
	err = snd_pcm_hw_params_set_buffer_size_near(handle, hwparams, &buffer_size);
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: snd_pcm_hw_params_set_buffer_size_near, error = " << err << endl;
		return err;
	} else {
		cout << "Sound::asound_set_hwparams: snd_pcm_hw_params_set_buffer_size_near, error = " << err
				<< " ,buffer_size_org = " << buffer_size_org << " , buffer_size = " << buffer_size << endl;
	}

	/* get the buffer size to check */
	err = snd_pcm_hw_params_get_buffer_size(hwparams, &buffer_size_get);
	if (err < 0) {
		printf("Unable to get buffer size: %s\n", snd_strerror(err));
		return err;
	}
	cout << "snd_pcm_hw_params_get_buffer_size: err = " << err << " , buffer_size_get = " << buffer_size_get << endl;
	if(buffer_size_get != buffer_size) {
		cout << "snd_pcm_hw_params_get_buffer_size: buffer_size does not match!" << endl;
		return err;
	}

	/* set the period size */
	period_size_org = period_size;
	err = snd_pcm_hw_params_set_period_size_near(handle, hwparams, &period_size, NULL);
	cout << "snd_pcm_hw_params_set_period_size_near: err = " << err << endl;
	if (err < 0) {
		cout << "Sound::asound_set_hwparams: period size error = " << err << endl;
		return err;
	} else {
		cout << "Sound::asound_set_hwparams: snd_pcm_hw_params_set_period_size_near, error = " << err
				<< " ,period_size_org = " << period_size_org << " , period_size = " << period_size << endl;
	}

	/* get the period size to check */
	err = snd_pcm_hw_params_get_period_size(hwparams, &period_size_get, &sub_unit_direction);
	if (err < 0) {
		printf("Unable to get period size: %s\n", snd_strerror(err));
		return err;
	}
	cout << "snd_pcm_hw_params_get_period_size: err = " << err << " , period_size_get = " << period_size_get
			<< " , sub_unit_direction = " << sub_unit_direction << endl;
	if(period_size_get != period_size) {
		cout << "snd_pcm_hw_params_get_period_size: period_size does not match!" << endl;
		return err;
	}

	/* write the parameters to device */
	err = snd_pcm_hw_params(handle, hwparams);
	cout << "snd_pcm_hw_params: err = " << err << endl;
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

	/* get the current swparams */
	err = snd_pcm_sw_params_current(handle, swparams);
	cout << "snd_pcm_sw_params_current: err = " << err << endl;
	if (err < 0) {
		cout
		<< "Sound::asound_set_swparams: Unable to determine current swparams: "
		<< snd_strerror(err) << endl;
		return err;
	}

	/* start the transfer when the buffer is half full, or some data becomes available */
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, period_size*channels);
	cout << "snd_pcm_sw_params_set_start_threshold: err = " << err << endl;
	if (err < 0) {
		cout
		<< "Sound::asound_set_swparams: Unable to set start threshold mode: "
		<< snd_strerror(err) << endl;
		return err;
	}

	/* allow the transfer when at least period_size samples can be processed */
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, period_size*channels);
	cout << "snd_pcm_sw_params_set_avaail_min: err = " << err << endl;
	if (err < 0) {
		cout
		<< "Sound::asound_set_swparams: Unable to set avail min: "
		<< snd_strerror(err) << endl;
		return err;
	}

	/* write the parameters to the device */
	err = snd_pcm_sw_params(handle, swparams);
	cout << "snd_pcm_sw_params: err = " << err << endl;
	if (err < 0) {
		cout
		<< "Sound::asound_set_swparams: Unable to set sw params: "
		<< snd_strerror(err) << endl;
		return err;
	}

	return 0;
}
