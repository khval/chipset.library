/* :ts=4
    Copyright (C) 2020  Kjetil Hvalstrand

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
    USA
 */


#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "proto/exec.h"
#include "proto/dos.h"

#include "uae/cia.h"
#include "uae/events.h"

uint32_t cycles = 0;
struct ev eventtab[ev_max];
uint32_t nextevent =0 ;
int custom_bank = 0;



BOOL lock_init();
void lock_cleanup();

//CIA_hsync_handler();
//CIA_vsync_handler();

// Fcolor = 4.43361825 MHz (PAL color carrier frequency)
// CPU clock = Fcolor * 1.6  = 7.0937892 Mhz
// CIA Clock = cpu clock / 10 = 709.37892 Khz

// CIA clock is 1 / 70938,92 = 1,4096 us
// 1 / 50Hz = every 20ms, we need to trigger vsync.


int htick = 0;
bool pause = false;


// to trigger more often we set _50hz to 20Hz.


// 1/ 709378.92 = 1,4 us
// 1/ 50 is  20 ms period 

#define _50Hz_period_us 20000.0f

double cia_time_us = 100000.0f / 70938.92f;
double _50Hz_period_us_cnt = 0;

// 240 scanlines for PAL screen
// 224 scanline for NTSC.

#define hsync_period_us (20000.0f / 240.0f)
double hsync_period_us_cnt = 0;

void do_cia()
{
	// We call this like every 10 cpu cycles, (Amiga500)
	cycles += 10;	

	_50Hz_period_us_cnt += cia_time_us;
	hsync_period_us_cnt += cia_time_us;

	printf("hsync_period_us_cnt: %0.3f\n", hsync_period_us_cnt);

	CIA_handler();	// this function process cpu cycles

	//	and trigger CIA_hsync_handler every 50 tick.

	if (hsync_period_us_cnt >= hsync_period_us)
	{
		printf("trigger hsync\n");
		CIA_hsync_handler();
		hsync_period_us_cnt -= hsync_period_us;
	}

	if (_50Hz_period_us_cnt >= _50Hz_period_us) 
	{	
		printf("trigger vsync\n");
		CIA_vsync_handler();
		pause = true;

		_50Hz_period_us_cnt -= _50Hz_period_us;	// 20 ms.
	}
} 


#define run_for (3*50)

void do_pause()
{
	if (pause)	
	{
		getchar();
		pause = false;
	}
}

extern int ciaatodon;
extern int ciabtodon;

int main()
{
	int r;
	void *stderr_backup = stderr;
	stderr = stdout;

	if (lock_init() == false)
	{
		lock_cleanup();
		return 0;
	}

	printf("cia_time_us: %0.3f\n",cia_time_us);
	getchar();
	CIA_reset();

	// enable ciaatod

	WriteCIAA( 0xF , 0x00 );
	WriteCIAA( 0x8 , 0x00 );

	printf("ciaatod: %s\n", ciaatodon ? "On" : "Off");
	printf("ciabtod: %s\n", ciabtodon ? "On" : "Off");


	for (r=0;r<run_for;r++)
	{
		printf("event: %d\n",r);
		do_cia();
		dumpcia();
		do_pause();
	}

	stderr = stderr_backup;

	lock_cleanup();

	return 0;
}

void INTREQ_0(uint32_t xx)
{

}
