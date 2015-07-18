/*
 * Mydefine.h
 *
 *  Created on: Jul 12, 2015
 *      Author: user1
 */

/*
 * Mydefine.h
 *
 *  Created on: Apr 9, 2015
 *      Author: user1
 */
#ifndef MYDEFINE_H
#define MYDEFINE_H

#define DEBUG
#define NO_MARKER
#define NFFT                    2048
#define WINDOW_XSIZE            2058
#define WINDOW_YSIZE             900
#define AREA1_XSIZE               99
#define AREA1_YSIZE               50
#define WATERFALL_XSIZE         2048
#define WATERFALL0_XSIZE         480
#define WATERFALL_YSIZE          256
#define WATERFALL_ZSIZE           50
#define WATERFALL_WSIZE           10
#define WATERFALL_XOFFSET          0
#define WATERFALL_YOFFSET          0
#define WAVEFORM_LEN             128
#define BAUDRATE              B19200
#define TIMEOUT_VALUE            100
#define END_OF_COMMAND          0xfd

#include <asoundlib.h>
#include <gtkmm.h>
#include <fftw3.h>
#include <iostream>
#include <string>
#include <time.h>
#include <sys/time.h>
#include <ctime>
#include <iomanip>
using namespace std;

int send_command(unsigned char *partial_command);
int receive_fb();
void myfunc(int);
int rig_init_serial(char *);
void set_ic7410_freq(long int ifreq_in_hz);
void myclock();
int colormap_r(double);
int colormap_g(double);
int colormap_b(double);

void cout_gettimeofday_diff(string, timeval, timeval);

#endif



