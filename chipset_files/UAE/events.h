 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Events
  * These are best for low-frequency events. Having too many of them,
  * or using them for events that occur too frequently, can cause massive
  * slowdown.
  *
  * Copyright 1995-1998 Bernd Schmidt
  */

extern void reset_frame_rate_hack (void);
extern int rpt_available;

extern uint32_t cycles;
extern uint32_t nextevent, is_lastline;
extern unsigned long int sample_evtime;
typedef void (*evfunc)(void);

struct ev
{
    int active;
    unsigned long int evtime, oldcycles;
    evfunc handler;
};

enum {
    ev_hsync, ev_copper, ev_cia,
    ev_blitter, ev_diskblk, ev_diskindex,
    ev_max
};

extern struct ev eventtab[ev_max];

extern void events_schedule (void);

extern void do_cycles_slow (uint32_t cycles_to_add);

#define do_cycles do_cycles_slow
