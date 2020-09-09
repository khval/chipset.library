
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "uade/cia.h"
#include "uade/events.h"

#define __USE_INLINE__

#include <proto/exec.h>
#include <proto/dos.h>

unsigned long int cycles = 0;
struct ev eventtab[ev_max];
unsigned long int nextevent =0 ;
int custom_bank = 0;


//CIA_hsync_handler();
//CIA_vsync_handler();

// Fcolor = 4.43361825 MHz (PAL color carrier frequency)
// CPU clock = Fcolor * 1.6  = 7.0937892 Mhz
// CIA Clock = cpu clock / 10 = 709.37892 Khz

// CIA clock is 1 / 70938,92 = 1,4096 us
// 1 / 50Hz = every 20ms, we need to trigger vsync.

int htick = 0;

// to trigger more often we set _50hz to 20Hz.


// 1/ 709378.92 = 1,4 us
// 1/ 50 is  20 ms period 

// #define _50Hz_period_us 10.0f

#define _50Hz_period_us 20000.0f

float cia_time_us = 100000.0f / 70938.92f;
double _50Hz_period_us_cnt = 0;

// 240 scanlines for PAL screen
// 224 scanline for NTSC.

#define hsync_period_us (20000.0f / 240.0f)
double hsync_period_us_cnt = 0;
struct timeval start, end;

void do_cia( float delta_us )
{
	_50Hz_period_us_cnt += delta_us;
	hsync_period_us_cnt += delta_us;

	CIA_handler();	// this function process cpu cycles

	while (hsync_period_us_cnt >= hsync_period_us)
	{
		CIA_hsync_handler();
		hsync_period_us_cnt -= hsync_period_us;
	}

	if (_50Hz_period_us_cnt >= _50Hz_period_us) 
	{	
		CIA_vsync_handler();
		_50Hz_period_us_cnt -= _50Hz_period_us;	// 20 ms.
	}
} 

extern bool expunge_tasks;

	// 20 ms /  20000 us is a frame, 
	// if must have a tigger for etch 240 rows then 83 us should be fine.

uint32_t cia_latency_us = 83;

#define setTimerIO()									\
	TimerIO->Request.io_Command = TR_ADDREQUEST;		\
	TimerIO->Time.Seconds=0;							\
	TimerIO->Time.Microseconds = cia_latency_us;						\
	DoIO(TimerIO);

struct MsgPort			*TimerMP = NULL;
struct TimeRequest		*TimerIO = NULL;
int 					timer_device_open = 0;

extern void init_timer_device();
extern void close_timer_device();

extern struct _Library *libBase_debug;

void cia_process_fn ()
{
	int microseconds = NULL;
	int count;

	Printf("init timer.device...\n");

	init_timer_device();

	Printf("ready...\n");

	do
	{
		setTimerIO();
		gettimeofday(&end,NULL);

		microseconds += (double) ((end.tv_usec - start.tv_usec) + (end.tv_sec - start.tv_sec) * 1000000);
		count = (int) (microseconds / cia_time_us);
		microseconds-= cia_time_us * (double) count; 

		// 1 cia time = 10 cpu cycles, (Amiga500)
		cycles+=(10 *count);

		do_cia(cia_time_us * count);
		start = end;
 
//		Printf("libcount: %ld, expunge: %ld\n", ((struct Library *)libBase_debug)->lib_OpenCnt, expunge_tasks);

	} while (  expunge_tasks == false );

	Printf("trying to close\n");

	close_timer_device();
}

