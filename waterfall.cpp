#include "mydefine.h"
#include "waterfall.h"
#include "Sound.h"
#include <cairomm/context.h>
#include <gdkmm/general.h>	// set_source_pixbuf()
#include <glibmm/fileutils.h>
#include <cmath>
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <cairo.h>
#include <glibmm/main.h>
#include <asoundlib.h>
#include <fftw3.h>
#include <time.h>
#include <sys/time.h>
#include <ctime>

using namespace std;
extern Sound *mysound[];
extern struct timeval t0;

void set_freq(long int ifreq_in_hz);
int colormap_r(double);
int colormap_g(double);
int colormap_b(double);

Waterfall::Waterfall() {

	std::cout << "Waterfall constructor is called." << std::endl;
	string myid ="Waterfall::Waterfall: ";

	set_size_request(WATERFALL_XSIZE + WATERFALL_XOFFSET, WATERFALL_YSIZE + WATERFALL_YOFFSET + WATERFALL_ZSIZE);
	m_image = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, WATERFALL_XSIZE, WATERFALL_YSIZE + WATERFALL_ZSIZE);

	guint8* p = m_image->get_pixels();

	nfft[0] = 4 * 1024; /* IC-7410 */
	nfft[1] = 2 * 1024; /* Soft66LC4 */

	for(int i=0;i<2;i++) {
		bin_size[i] = (double) mysound[i]->rate / (double) nfft[i];
		cout << myid << "i = " << i << ", nfft = " << nfft[i] << ", rate =" << mysound[i]->rate << ", bin_size = " << bin_size[i] << endl;
		fft_window        [i] = new double [nfft[i]];
		audio_signal_ffted[i] = new double [nfft[i]];
		for (int j = 0; j < nfft[i]; j++) {
			fft_window[i][j] = 0.54 - 0.46 * cos(2.0 * M_PI * j / (double) nfft[i]);
		}
		/* plan: FFTW_ESTIMATE, FFTW_MEASURE, FFTW_PATIENT, FFTW_EXHAUSTIVE */
		if(0) {
		} else if(i == 0) {
			in_real = new double[nfft[i]];
			out [i] = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * ( nfft[i]/2 + 1 )*2 );
			plan[i] = fftw_plan_dft_r2c_1d(nfft[i], in_real, out[i], FFTW_MEASURE);
		} else if(i == 1) {
			in      = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft[i]);
			out [i] = (fftw_complex *) fftw_malloc(sizeof(fftw_complex) * nfft[i]);
			plan[i] = fftw_plan_dft_1d    (nfft[i], in     , out[i], FFTW_FORWARD, FFTW_MEASURE);
		}
	}

	/* IC-7410 spectrum area */
	for (int j = 0; j < WATERFALL_ZSIZE; j++) {
		for (int i = 0; i < WATERFALL_XSIZE; i++) {
			*p++ = 0;
			*p++ = (int) ( 255.0 * (double)i / (double) WATERFALL_XSIZE );
			*p++ = 0;
		}
	}

	/* IC-7410 waterfall area */
	for (int j = 0; j < WATERFALL_YSIZE/2; j++) {
		for (int i = 0; i < WATERFALL_XSIZE; i++) {
			*p++ = i % 256;
			*p++ = j % 256;
			*p++ = 0;
		}
	}

	/* Soft66LC4 marker area */
	for (int j = 0; j < 5; j++) {
		for (int i = 0; i < WATERFALL_XSIZE; i++) {
			int tmp = 128 + 127 * ( ( (int) ( abs(i-WATERFALL_XSIZE/2) * bin_size[1] / 5000.0) ) % 2);
			*p++ = tmp;
			*p++ = tmp;
			*p++ = tmp;
		}
	}

	/* Soft66LC4 waterfall area */
	for (int j = 0; j < WATERFALL_YSIZE/2 - 5; j++) {
		for (int i = 0; i < WATERFALL_XSIZE; i++) {
			*p++ = 0;
			*p++ = i % 256;
			*p++ = j % 256;
		}
	}

	cout << myid << "going to signal_timeout" << endl;
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &Waterfall::on_timeout), 160 );
	cout << myid << "returned from signal_timeout" << endl;
#ifndef GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
	// actually, this ifndef part is not necessary.
	cout << myid << "should not occur." << endl;
	signal_draw().connect(sigc::mem_fun(*this, &Waterfall::on_draw), false);
#endif				//GLIBMM_DEFAULT_SIGNAL_HANDLERS_ENABLED
	cout << myid << "going to add_events" << endl;
	add_events(	Gdk::BUTTON_PRESS_MASK );
	cout << myid << "returned from add_events" << endl;

	mysound[0]->Sound_go(); /* start audio device */
	mysound[1]->Sound_go(); /* start audio device */

	std::cout << "Waterfall constructor end.." << std::endl;

}

Waterfall::~Waterfall() {
	std::cout << "Waterfall destructor is called." << std::endl;
}

bool Waterfall::on_draw(const Cairo::RefPtr<Cairo::Context> &cr) {
	static int icountx = 0;
	static struct timeval t1, t1b4, t9;
	gettimeofday(&t1, NULL);
	string myid = "Waterfall::on_draw: ";
	guint8 *p = nullptr;

	cout << myid << "icountx = " << icountx++ << endl;
	int rowstride = m_image->get_rowstride();

	bool ready[2] = {false, false};
	for(int irep=0;irep<2;irep++) {
		ready[irep] = mysound[irep]->asound_myread();
		cout << myid << "(" << irep << ") ";
		if(ready[irep]) {
			cout << " ready" << endl;
		} else {
			cout << " not ready" << endl;
		}

		// shift down pixbuf
		if(irep == 0) {
			for (int i = WATERFALL_YSIZE/2 + WATERFALL_ZSIZE- 1; i > 0 + WATERFALL_ZSIZE; i--) {
				p = m_image->get_pixels() + i * rowstride;
				for (int j = 0; j < WATERFALL_XSIZE * 3; j++) {
					*p = *(p - WATERFALL_XSIZE * 3);
					p++;
				}
			}
		} else if(irep == 1) {
			for (int i = WATERFALL_YSIZE + WATERFALL_ZSIZE- 1; i > WATERFALL_YSIZE/2 + 5 + WATERFALL_ZSIZE; i--) {
				p = m_image->get_pixels() + i * rowstride;
				for (int j = 0; j < WATERFALL_XSIZE * 3; j++) {
					*p = *(p - WATERFALL_XSIZE * 3);
					p++;
				}
			}
		}

		/* audio signal FFT */
		if(irep == 0) {
			for (int i = 0; i < nfft[irep]; i++) {
				in_real[i] = fft_window[irep][i] * mysound[irep]->audio_signal[i];
			}
		} else if(irep == 1) {
			for (int i = 0; i < nfft[irep]; i++) { /* I and Q reversed */
				in[i][1] = fft_window[irep][i] * mysound[irep]->audio_signal[2 * i];
				in[i][0] = fft_window[irep][i] * mysound[irep]->audio_signal[2 * i + 1];
			}
		}
		fftw_execute(plan[irep]);

		/* log10 and normalize */

		amax[irep] = 14.0 - irep * 1.0;
		amin[irep] =  7.0;
		for (int i = 0; i < nfft[irep]; i++) {
			double val;
			val = out[irep][i][0] * out[irep][i][0] + out[irep][i][1] * out[irep][i][1];
			if (val < pow(10.0, amin[irep])) {
				audio_signal_ffted[irep][i] = 0.0;
			} else if (val > pow(10.0, amax[irep])) {
				audio_signal_ffted[irep][i] = 1.0;
			} else {
				audio_signal_ffted[irep][i] = (log10(val) - amin[irep]) / (amax[irep] - amin[irep]);
			}
		}

		// write one line for IC-7410
		if(irep == 0) {
			p = m_image->get_pixels() + rowstride * WATERFALL_ZSIZE;
			for (int i = 0; i < WATERFALL_XSIZE; i++) {
					double tmp = audio_signal_ffted[irep][i];
					*p++ = colormap_r(tmp);
					*p++ = colormap_g(tmp);
					*p++ = colormap_b(tmp);
			}
		}

		// write one line for Soft66LC4
		if(irep == 1) {
			p = m_image->get_pixels() + rowstride * (WATERFALL_YSIZE/2 + WATERFALL_ZSIZE + 5);
			for (int i = 0; i < WATERFALL_XSIZE; i++) {
				double tmp = audio_signal_ffted[irep][(i + WATERFALL_XSIZE/2) % WATERFALL_XSIZE];
				*p++ = colormap_r(tmp);
				*p++ = colormap_g(tmp);
				*p++ = colormap_b(tmp);
			}
			/* Soft66LC4 marker area */
			p = m_image->get_pixels() + rowstride * (WATERFALL_YSIZE/2 + WATERFALL_ZSIZE);
			for (int j = 0; j < 5; j++) {
				for (int i = 0; i < WATERFALL_XSIZE; i++) {
					int tmp = 128 + 127 * ( ( (int) ( abs(i-WATERFALL_XSIZE/2) * bin_size[1] / 5000.0) ) % 2);
					if( abs( (i-WATERFALL_XSIZE/2) * bin_size[1] + 7020000 - ifreq_in_hz) < 200.0) {
						tmp = 0;
					}
					*p++ = tmp;
					*p++ = tmp;
					*p++ = tmp;
				}
			}
		}

	} /* end of sound device loop */

	Gdk::Cairo::set_source_pixbuf(cr, m_image, WATERFALL_XOFFSET, WATERFALL_YOFFSET);
	cr->paint();
	cr->stroke();

	cr->save();
	cr->set_source_rgba(0.2, 0.9, 0.9, 1.0);
	cr->move_to  (0.0, 40.0 * (1.0 - audio_signal_ffted[0][0]) + 5.0 + 0.0);
	for (int i = 0; i < WATERFALL_XSIZE; i++) {
		cr->line_to(i, 40.0 * (1.0 - audio_signal_ffted[0][i]) + 5.0 + 0.0);
	}
	cr->stroke();
	cr->restore();

	int itone = cw_pitch / bin_size[0];
	cr->save();
	cr->set_source_rgba(0.9, 0.9, 0.0, 0.8);
	cr->move_to(itone, 50.0);
	cr->line_to(itone, 40.0);
	cr->stroke();
	cr->restore();

	gettimeofday(&t9, NULL);
	cout_gettimeofday_diff(myid+"             elapsed:  ", t0  , t1);
	cout_gettimeofday_diff(myid+"             interval: ", t1b4, t1);
	cout_gettimeofday_diff(myid+"             duration: ", t1  , t9);
	t1b4 = t1;

	return true;
}

bool Waterfall::on_timeout() {
	static int icount = 0;

	static struct timeval t1, t1b4, t9;
	gettimeofday(&t1, NULL);
	string myid = "Waterfall::on_timeout: ";

	cout << myid << "icountw = " << icount++ <<  endl;

	Glib::RefPtr<Gdk::Window> win = get_window();
	if (win) {
		cout << "Waterfall::on_timeout: get_window OK" << endl;
		cout << "Waterfall::on_timeout: width  " << get_allocation().get_width() << endl;
		cout << "Waterfall::on_timeout: height " << get_allocation().get_height() << endl;
		win->invalidate(false);
	} else {
		cout << "Waterfall::on_timeout: get_window NG" << endl;
	}

	gettimeofday(&t9, NULL);
	cout_gettimeofday_diff(myid+"          elapsed:  ", t0  , t1);
	cout_gettimeofday_diff(myid+"          interval: ", t1b4, t1);
	cout_gettimeofday_diff(myid+"          duration: ", t1  , t9);
	t1b4 = t1;

	return true;
}

bool Waterfall::on_button_press_event(GdkEventButton * event) {

	x_press = event->x;
	y_press = event->y;

	std::cout << "Waterfall::on_button_press_event:  x_press = " << x_press
			  << ", y_press = "     << y_press << endl;

	if (y_press <= WATERFALL_YSIZE/2 + WATERFALL_ZSIZE) { /* IC-7410 area */
		if(operating_mode == 3) { /* CW is LSB */
			ifreq_in_hz -= x_press * bin_size[0] - cw_pitch;
		} else if(operating_mode == 7) { /* CW-R is USB */
			ifreq_in_hz += x_press * bin_size[0] - cw_pitch;
		} else {
			;
		}
		set_freq(ifreq_in_hz);
	}

	if (y_press >= WATERFALL_YSIZE/2 + WATERFALL_ZSIZE) { /* Soft66LC4 area */
		ifreq_in_hz = 7020000 + (x_press - (WATERFALL_XSIZE/2) ) * bin_size[1];
		set_freq(ifreq_in_hz);
	}

	return true;
}
