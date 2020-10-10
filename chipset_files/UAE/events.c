
#include <stdint.h>
#include <stdbool.h>

#include "sysconfig.h"
#include "sysdeps.h"

#include "options.h"
#include "UAE/events.h"

#include <proto/dos.h>

extern void event_lock();
extern void event_unlock();

void events_schedule (void)
{
	uint32_t mintime = ~0L;
	uint32_t eventtime;

	event_lock();

	/* HSYNC */
	if(eventtab[ev_hsync].active)
	{
		eventtime = eventtab[ev_hsync].evtime - cycles;
		if (eventtime < mintime) 
		{
			mintime = eventtime;
			Printf("mintime use hsync %ld < %ld\n", eventtime, mintime );
		}
	}

	/* AUDIO */
	#if 0
	if(eventtab[ev_audio].active)
	{
		eventtime = eventtab[ev_audio].evtime - cycles;
		if (eventtime < mintime)
		{
			mintime = eventtime;
			Printf("mintime use audio %ld < %ld\n", eventtime, mintime );
		}
	}
	#endif

	/* CIA */
	if(eventtab[ev_cia].active)
	{
		eventtime = eventtab[ev_cia].evtime - cycles;
		if (eventtime < mintime)
		{
			 mintime = eventtime;
			Printf("mintime use cia %ld < %ld\n", eventtime, mintime );
		}
	}

	/* blitter */
	if(eventtab[ev_blitter].active)
	{
		eventtime = eventtab[ev_blitter].evtime - cycles;
		if (eventtime < mintime) 
		{
			Printf("mintime use blitter %08lx < %08lx\n", eventtime, mintime );
			mintime = eventtime;
		}
	}

	nextevent = mintime + cycles;

	printf("nextevent %08x\n", nextevent );

	event_unlock();
}

void do_cycles_slow (uint32_t cycles_to_add)
{
	uint32_t delta_nextevent = 0;

	if (cycles_to_add == 0) return;

	event_lock();
	delta_nextevent = nextevent - cycles;

	if (cycles_to_add > delta_nextevent )
	{ 

		cycles_to_add -= delta_nextevent;
		cycles += delta_nextevent;

		/* HSYNC */
		if(eventtab[ev_hsync].active && eventtab[ev_hsync].evtime == cycles)
		{
			(*eventtab[ev_hsync].handler)();
		}

		/* CIA */
		if(eventtab[ev_cia].active && eventtab[ev_cia].evtime == cycles)
		{
			(*eventtab[ev_cia].handler)();
		}

		/* blitter */
		if(eventtab[ev_blitter].active && eventtab[ev_blitter].evtime == cycles)
		{
			(*eventtab[ev_blitter].handler)();
		}
	}

	cycles += cycles_to_add;
	event_unlock();

	events_schedule();
}

