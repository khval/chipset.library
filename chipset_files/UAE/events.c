
#include <stdint.h>
#include <stdbool.h>

#include "sysconfig.h"
#include "sysdeps.h"

#include "options.h"
#include "UAE/events.h"

#include <proto/dos.h>

void events_schedule (void)
{
	uint16_t mintime = ~0L;
	unsigned long int eventtime;

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
			Printf("mintime use blitter %ld < %ld\n", eventtime, mintime );
			mintime = eventtime;
		}
	}

	nextevent = mintime + cycles;
	Printf("nextevent %ld\n", nextevent );

	event_unlock();
}

void do_cycles_slow (uint32_t cycles_to_add)
{
	int delta_nextevent = 0;

	if (cycles_to_add == 0) return;

	event_lock();

	delta_nextevent = nextevent - cycles;

	if (~0L != delta_nextevent)
	{
		Printf("%s:%ld --  nextevent  %ld -  cycles_to_add: %ld\n",__FUNCTION__,__LINE__, nextevent , cycles_to_add);
		Printf("hsync: %s, cia: %s: blitter: %s\n",
			eventtab[ev_hsync].active ? "active" : "disabled",
			eventtab[ev_cia].active ? "active" : "disabled",
			eventtab[ev_blitter].active ? "active" : "disabled");
	}

	if (cycles_to_add >= delta_nextevent )
	{ 
		cycles_to_add -= delta_nextevent;
		cycles += delta_nextevent;
		delta_nextevent = 0;
	}
	else 
	{
		delta_nextevent -= cycles_to_add;
	}

	if (delta_nextevent == 0)
	{
		Printf("We got something to do\n{\n");

		/* HSYNC */
		if(eventtab[ev_hsync].active && eventtab[ev_hsync].evtime == cycles)
		{
			Printf("Do HSYNC\n");

			(*eventtab[ev_hsync].handler)();
		}

		/* AUDIO */
#if 0
		if(eventtab[ev_audio].active && eventtab[ev_audio].evtime == cycles)
		{
			(*eventtab[ev_audio].handler)();
		}
#endif

		/* CIA */
		if(eventtab[ev_cia].active && eventtab[ev_cia].evtime == cycles)
		{
			Printf("Do CIA\n");

			(*eventtab[ev_cia].handler)();
		}

		Printf("eventtab[ev_blitter].evtime: %ld == cycles: %ld\n", eventtab[ev_blitter].evtime , cycles);

		/* blitter */
		if(eventtab[ev_blitter].active && eventtab[ev_blitter].evtime == cycles)
		{
			Printf("Do bliter\n");
			(*eventtab[ev_blitter].handler)();
		}
			
		Printf("}\n");

		Delay(5);
	}


	cycles += cycles_to_add;

	event_unlock();

	events_schedule();
}

