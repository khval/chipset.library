 /*
  * UAE - The Un*x Amiga Emulator
  *
  * CIA chip support
  *
  * Copyright 1995 Bernd Schmidt, Alessandro Bissacco
  * Copyright 1996, 1997 Stefan Reinauer, Christian Schmitt
  * Copyright 2020 Kjetil Hvalstrand
  */

#include "stdint.h"
#include "sysconfig.h"
#include "sysdeps.h"
#include <assert.h>

#include "options.h"
#include "events.h"
#include "memory.h"
#include "custom.h"
#include "cia.h"

#include "proto/exec.h"
#include "proto/dos.h"

// #include "uade.h"

#define DIV10 5 /* Yes, a bad identifier. */

/* battclock stuff */
#define RTC_D_ADJ      8
#define RTC_D_IRQ      4
#define RTC_D_BUSY     2
#define RTC_D_HOLD     1
#define RTC_E_t1       8
#define RTC_E_t0       4
#define RTC_E_INTR     2
#define RTC_E_MASK     1
#define RTC_F_TEST     8
#define RTC_F_24_12    4
#define RTC_F_STOP     2
#define RTC_F_RSET     1

static unsigned int clock_control_d = RTC_D_ADJ + RTC_D_HOLD;
static unsigned int clock_control_e = 0;
static unsigned int clock_control_f = RTC_F_24_12;

unsigned int ciaaicr,ciaaimask,ciabicr,ciabimask;
unsigned int ciaacra,ciaacrb,ciabcra,ciabcrb;
unsigned long ciaata,ciaatb,ciabta,ciabtb;
unsigned long ciaatod,ciabtod,ciaatol,ciabtol,ciaaalarm,ciabalarm;
int ciaatlatch,ciabtlatch;

unsigned int ciaapra, ciabpra;

unsigned int gui_ledstate;
int gui_ledstate_forced = 0;

static unsigned long ciaala,ciaalb,ciabla,ciablb;
int ciaatodon, ciabtodon;
static unsigned int ciaaprb,ciaadra,ciaadrb,ciaasdr;
static unsigned int ciabprb,ciabdra,ciabdrb,ciabsdr;
static int div10;
static int kbstate, kback, ciaasdr_unread = 0;

static int prtopen;
static FILE *prttmp;

static void setclr(unsigned int *p, unsigned int val)
{
    if (val & 0x80) {
	*p |= val & 0x7F;
    } else {
	*p &= ~val;
    }
}

static void RethinkICRA(void)
{
    if (ciaaimask & ciaaicr) {
	ciaaicr |= 0x80;
	custom_bank.wput(0xDFF09C,0x8008);
    } else {
	ciaaicr &= 0x7F;
/*	custom_bank.wput(0xDFF09C,0x0008);*/
    }
}

static void RethinkICRB(void)
{
#if 0 /* ??? What's this then? */
    if (ciabicr & 0x10) {
	custom_bank.wput(0xDFF09C,0x9000);
    }
#endif
    if (ciabimask & ciabicr) {
	ciabicr |= 0x80;
	custom_bank.wput(0xDFF09C,0xA000);
    } else {
	ciabicr &= 0x7F;
/*	custom_bank.wput(0xDFF09C,0x2000);*/
    }
}

static uint16_t lastdiv10;

static void CIA_update(void) // used by cia_handler
{
	uint16_t ccount,ciaclocks ;
	int aovfla = 0, aovflb = 0, bovfla = 0, bovflb = 0;

	ccount = cycles - eventtab[ev_cia].oldcycles;
	ccount += lastdiv10;

	ciaclocks = ccount / DIV10;

	lastdiv10 = div10;
	div10 = ccount % DIV10;

	/* CIA A timers */
	if ((ciaacra & 0x21) == 0x01) {

	if (ciaclocks)
	{
		if (ciaata <= ciaclocks)
		{
			aovfla = 1;
			if ((ciaacrb & 0x61) == 0x41)
			{
				if (ciaatb-- == 0) aovflb = 1;
			}

			ciaata -= ciaata ;
		}
		else
		{
			ciaata -= ciaclocks;
		}
	}
	else Printf("No CIA clock\n");
    }

    if ((ciaacrb & 0x61) == 0x01) {
	assert((ciaatb+1) >= ciaclocks);
	if ((ciaatb+1) == ciaclocks) aovflb = 1;
	ciaatb -= ciaatb < ciaclocks ? ciaatb : ciaclocks;
    }

    /* CIA B timers */
    if ((ciabcra & 0x21) == 0x01) {
	assert((ciabta+1) >= ciaclocks);
	if ((ciabta+1) == ciaclocks) {
	    bovfla = 1;
	    if ((ciabcrb & 0x61) == 0x41) {
		if (ciabtb-- == 0) bovflb = 1;
	    }
	}
	ciabta -= ciaclocks;
    }
    if ((ciabcrb & 0x61) == 0x01) {
	assert ((ciabtb+1) >= ciaclocks);
	if ((ciabtb+1) == ciaclocks) bovflb = 1;
	ciabtb -= ciaclocks;
    }
    if (aovfla) {
	Printf("ciaaicr bit 1 is true\n");

	ciaaicr |= 1; 
	Delay(1);
	RethinkICRA();
	ciaata = ciaala;					// reset timer
	if (ciaacra & 0x8) ciaacra &= ~1;
    }
    if (aovflb) {
	ciaaicr |= 2; RethinkICRA();
	ciaatb = ciaalb;					// reset timer
	if (ciaacrb & 0x8) ciaacrb &= ~1;
    }
    if (bovfla) {
	ciabicr |= 1; RethinkICRB();
	ciabta = ciabla;					// reset timer
	if (ciabcra & 0x8) ciabcra &= ~1;
    }
    if (bovflb) {
	ciabicr |= 2; RethinkICRB();
	ciabtb = ciablb;					// reset timer
	if (ciabcrb & 0x8) ciabcrb &= ~1;
    }
}

static void CIA_calctimers(void)
{
    int ciaatimea = -1, ciaatimeb = -1, ciabtimea = -1, ciabtimeb = -1;

    eventtab[ev_cia].oldcycles = cycles;

    if ((ciaacra & 0x21) == 0x01) {
	ciaatimea = (DIV10-div10) + DIV10*ciaata;
    }
    if ((ciaacrb & 0x61) == 0x41) {
	/* Timer B will not get any pulses if Timer A is off. */
	if (ciaatimea >= 0) {
	    /* If Timer A is in one-shot mode, and Timer B needs more than
	     * one pulse, it will not underflow. */
	    if (ciaatb == 0 || (ciaacra & 0x8) == 0) {
		/* Otherwise, we can determine the time of the underflow. */
		ciaatimeb = ciaatimea + ciaala * DIV10 * ciaatb;
	    }
	}
    }
    if ((ciaacrb & 0x61) == 0x01) {
	ciaatimeb = (DIV10-div10) + DIV10*ciaatb;
    }

    if ((ciabcra & 0x21) == 0x01) {
	ciabtimea = (DIV10-div10) + DIV10*ciabta;
    }
    if ((ciabcrb & 0x61) == 0x41) {
	/* Timer B will not get any pulses if Timer A is off. */
	if (ciabtimea >= 0) {
	    /* If Timer A is in one-shot mode, and Timer B needs more than
	     * one pulse, it will not underflow. */
	    if (ciabtb == 0 || (ciabcra & 0x8) == 0) {
		/* Otherwise, we can determine the time of the underflow. */
		ciabtimeb = ciabtimea + ciabla * DIV10 * ciabtb;
	    }
	}
    }
    if ((ciabcrb & 0x61) == 0x01) {
	ciabtimeb = (DIV10-div10) + DIV10*ciabtb;
    }
    eventtab[ev_cia].active = (ciaatimea != -1 || ciaatimeb != -1
			       || ciabtimea != -1 || ciabtimeb != -1);
    if (eventtab[ev_cia].active) {
	unsigned long int ciatime = ~0L;
	if (ciaatimea != -1)
	  ciatime = ciaatimea;
	if (ciaatimeb != -1 && ((unsigned long int) ciaatimeb) < ciatime)
	  ciatime = ciaatimeb;
	if (ciabtimea != -1 && ((unsigned long int) ciabtimea) < ciatime)
	  ciatime = ciabtimea;
	if (ciabtimeb != -1 && ((unsigned long int) ciabtimeb) < ciatime)
	  ciatime = ciabtimeb;
	eventtab[ev_cia].evtime = ciatime + cycles;
    }
    events_schedule();
}

void CIA_handler(void)
{
	cia_lock();
	CIA_update();
	CIA_calctimers();
	cia_unlock();
}

void diskindex_handler(void)
{
    eventtab[ev_diskindex].evtime += cycles - eventtab[ev_diskindex].oldcycles;
    eventtab[ev_diskindex].oldcycles = cycles;
/*    fprintf(stderr,".\n");*/
    ciabicr |= 0x10;
    RethinkICRB();
}

void CIA_hsync_handler(void)
{
    static unsigned int keytime = 0, sleepyhead = 0;

    if (ciabtodon)
	ciabtod++;
    ciabtod &= 0xFFFFFF;

    if (ciabtod == ciabalarm) {
	ciabicr |= 4; RethinkICRB();
    }

}

void CIA_vsync_handler(void)
{
    if (ciaatodon)
	ciaatod++;
    ciaatod &= 0xFFFFFF;
    if (ciaatod == ciaaalarm) {
	ciaaicr |= 4; RethinkICRA();
    }
}

ULONG _spyCIAA(unsigned int addr)
{
    unsigned int tmp;

	addr = addr >> 8;

    switch(addr & 0xf) {
     case 0:				// pra
	return ciaapra;
     case 1:				// prb
	return ciaaprb;
     case 2:				// ddra
	return ciaadra;
     case 3:				// ddrb
	return ciaadrb;
     case 4:				// talo
	return ciaata & 0xff;
     case 5:				// tahi
	return ciaata >> 8;
     case 6:				// tblo
	return ciaatb & 0xff;
     case 7:				// tbhi
	return ciaatb >> 8;
     case 8:				// todlo
	if (ciaatlatch) {
	    return ciaatol & 0xff;
	} else
	    return ciaatod & 0xff;
     case 9:				// todmid
	if (ciaatlatch)
	    return (ciaatol >> 8) & 0xff;
	else
	    return (ciaatod >> 8) & 0xff;
     case 10:			// todhi
	return (ciaatod >> 16) & 0xff;
     case 12:			// sdr
	return ciaasdr;
     case 13:			// icr
	return ciaaicr;
     case 14:			// cra
	return ciaacra;
     case 15:			// crb
	return ciaacrb;
    }
    return 0;
}


static uae_u8 ReadCIAA(unsigned int addr)
{
    unsigned int tmp;

	Printf("ReadCIAA( %08lx)\n", addr);

    switch(addr & 0xf) {
     case 0:				// pra
	return ciaapra;
     case 1:				// prb
	return ciaaprb;
     case 2:				// ddra
	return ciaadra;
     case 3:				// ddrb
	return ciaadrb;
     case 4:				// talo
	return ciaata & 0xff;
     case 5:				// tahi
	return ciaata >> 8;
     case 6:				// tblo
	return ciaatb & 0xff;
     case 7:				// tbhi
	return ciaatb >> 8;
     case 8:				// todlo
	if (ciaatlatch) {
	    ciaatlatch = 0;
	    return ciaatol & 0xff;
	} else
	    return ciaatod & 0xff;
     case 9:				// todmid
	if (ciaatlatch)
	    return (ciaatol >> 8) & 0xff;
	else
	    return (ciaatod >> 8) & 0xff;
     case 10:			// todhi
	ciaatlatch = 1;
	ciaatol = ciaatod; /* ??? only if not already latched? */
	return (ciaatol >> 16) & 0xff;
     case 12:			// sdr
	if (ciaasdr == 1) ciaasdr_unread = 2;
	return ciaasdr;
     case 13:			// icr
	cia_lock();
	tmp = ciaaicr; ciaaicr = 0; RethinkICRA(); 
	cia_unlock();
	return tmp;
     case 14:			// cra
	return ciaacra;
     case 15:			// crb
	return ciaacrb;
    }

    return 0;
}

ULONG _spyCIAB(unsigned int addr)
{
    unsigned int tmp;

	addr = addr >> 8;

    switch(addr & 0xf) {
     case 0:
	return ciabpra;
     case 1:
	return ciabprb;
     case 2:
	return ciabdra;
     case 3:
	return ciabdrb;
     case 4:
	return ciabta & 0xff;
     case 5:
	return ciabta >> 8;
     case 6:
	return ciabtb & 0xff;
     case 7:
	return ciabtb >> 8;
     case 8:
	if (ciabtlatch) {
	    return ciabtol & 0xff;
	} else
	    return ciabtod & 0xff;
     case 9:
	if (ciabtlatch)
	    return (ciabtol >> 8) & 0xff;
	else
	    return (ciabtod >> 8) & 0xff;
     case 10:
	return (ciabtod >> 16) & 0xff;
     case 12:
	return ciabsdr;
     case 13:
	return ciabicr;
     case 14:
	return ciabcra;
     case 15:
	return ciabcrb;
    }
    return 0;
}

static uae_u8 ReadCIAB(unsigned int addr)
{
    unsigned int tmp;

    switch(addr & 0xf) {
     case 0:
	return ciabpra;
     case 1:
	return ciabprb;
     case 2:
	return ciabdra;
     case 3:
	return ciabdrb;
     case 4:
	return ciabta & 0xff;
     case 5:
	return ciabta >> 8;
     case 6:
	return ciabtb & 0xff;
     case 7:
	return ciabtb >> 8;
     case 8:
	if (ciabtlatch) {
	    ciabtlatch = 0;
	    return ciabtol & 0xff;
	} else
	    return ciabtod & 0xff;
     case 9:
	if (ciabtlatch)
	    return (ciabtol >> 8) & 0xff;
	else
	    return (ciabtod >> 8) & 0xff;
     case 10:
	ciabtlatch = 1;
	ciabtol = ciabtod;
	return (ciabtol >> 16) & 0xff;
     case 12:
	return ciabsdr;
     case 13:
	cia_lock();
	tmp = ciabicr; ciabicr = 0; RethinkICRB();
	cia_unlock();
	return tmp;
     case 14:
	return ciabcra;
     case 15:
	return ciabcrb;
    }
    return 0;
}

void WriteCIAA(uae_u16 addr,uae_u8 val)
{
    int oldled, oldovl;
	cia_lock();

    switch(addr & 0xf) {
     case 0:					// pra
	oldovl = ciaapra & 1;
	oldled = ciaapra & 2;
	ciaapra = (ciaapra & ~0x3) | (val & 0x3); 
	gui_ledstate = 0;
	if (!gui_ledstate_forced) {
	  gui_ledstate = (~ciaapra & 2) >> 1;
	} else {
	  gui_ledstate = gui_ledstate_forced & 1;
	}
	/* we don't want to have ersatzkickfile or kickstart roms in uade */
	/*
	if ((ciaapra & 1) != oldovl) {
	    map_banks(oldovl || ersatzkickfile ? &chipmem_bank : &kickmem_bank, 0, 32);
	}
	*/
	break;
     case 1:					// prb
	ciaaprb = val;
	if (prtopen==1) {
#ifndef __DOS__
	    fprintf (prttmp, "%c",val);
#else
	    fputc (val, prttmp);
	    fflush (prttmp);
#endif
	    if (val==0x04) {
#if defined(__unix) && !defined(__BEOS__) && !defined(__DOS__)
		pclose (prttmp);
#else
		fclose (prttmp);
#endif
		prtopen = 0;
	    }
	} else {
#if defined(__unix) && !defined(__BEOS__) && !defined(__DOS__)
	  // prttmp = (FILE *)popen ((const char *)currprefs.prtname, "w");
#else
	  // prttmp = (FILE *)fopen ((const char *)currprefs.prtname, "wb");
#endif
	    prttmp = stderr;
	    if (prttmp != NULL) {
		prtopen = 1;
#ifndef __DOS__
		fprintf (prttmp,"%c",val);
#else
		fputc (val, prttmp);
		fflush (prttmp);
#endif
	    }
	}
	ciaaicr |= 0x10;
	break;
     case 2:					// ddra
	ciaadra = val; break;
     case 3:					// ddrb
	ciaadrb = val; break;
     case 4:					// talo
	CIA_update();
	ciaala = (ciaala & 0xff00) | val;
	CIA_calctimers();
	break;
     case 5:					// tahi
	CIA_update();
	ciaala = (ciaala & 0xff) | (val << 8);
	if ((ciaacra & 1) == 0)
	    ciaata = ciaala;
	if (ciaacra & 8) {
	    ciaata = ciaala;
	    ciaacra |= 1;
	}
	CIA_calctimers();
	break;
     case 6:					// tblo
	CIA_update();
	ciaalb = (ciaalb & 0xff00) | val;
	CIA_calctimers();
	break;
     case 7:					// tbhi
	CIA_update();
	ciaalb = (ciaalb & 0xff) | (val << 8);
	if ((ciaacrb & 1) == 0)
	    ciaatb = ciaalb;
	if (ciaacrb & 8) {
	    ciaatb = ciaalb;
	    ciaacrb |= 1;
	}
	CIA_calctimers();
	break;
     case 8:					// todlo
	if (ciaacrb & 0x80) {
	    ciaaalarm = (ciaaalarm & ~0xff) | val;
	} else {
	    ciaatod = (ciaatod & ~0xff) | val;
	    ciaatodon = 1;
	}
	break;
     case 9:					// todmid
	if (ciaacrb & 0x80) {
	    ciaaalarm = (ciaaalarm & ~0xff00) | (val << 8);
	} else {
	    ciaatod = (ciaatod & ~0xff00) | (val << 8);
	    ciaatodon = 0;
	}
	break;
     case 10:				// todhi
	if (ciaacrb & 0x80) {
	    ciaaalarm = (ciaaalarm & ~0xff0000) | (val << 16);
	} else {
	    ciaatod = (ciaatod & ~0xff0000) | (val << 16);
	    ciaatodon = 0;
	}
	break;
     case 12:				// sdr
	ciaasdr = val; break;
     case 13:				// icr
	setclr(&ciaaimask,val); break; /* ??? call RethinkICR() ? */
     case 14:				// cra
	CIA_update();
	ciaacra = val;
	if (ciaacra & 0x10) {
	    ciaacra &= ~0x10;
	    ciaata = ciaala;
	}
	if (ciaacra & 0x40) {
	    kback = 1;
	}
	CIA_calctimers();
	break;
     case 15:				// crb
	CIA_update();
	ciaacrb = val;
	if (ciaacrb & 0x10) {
	    ciaacrb &= ~0x10;
	    ciaatb = ciaalb;
	}
	CIA_calctimers();
	break;
    }

	cia_unlock();
}

void WriteCIAB(uae_u16 addr,uae_u8 val)
{
    int oldval;
	cia_lock();

    switch(addr & 0xf) {
     case 0:					// pra
        ciabpra  = val;
	break;
     case 1:					// prb
	ciabprb = val; break;
     case 2:					// ddra
	ciabdra = val; break;
     case 3:					// ddrb
	ciabdrb = val; break;
     case 4:					// talo
	CIA_update();
	ciabla = (ciabla & 0xff00) | val;
	CIA_calctimers();
	break;
     case 5:					// tahi
	CIA_update();
	ciabla = (ciabla & 0xff) | (val << 8);
	if ((ciabcra & 1) == 0)
	    ciabta = ciabla;
	if (ciabcra & 8) {
	    ciabta = ciabla;
	    ciabcra |= 1;
	}
	CIA_calctimers();
	break;
     case 6:					//  tblo
	CIA_update();
	ciablb = (ciablb & 0xff00) | val;
	CIA_calctimers();
	break;
     case 7:					// tbhi
	CIA_update();
	ciablb = (ciablb & 0xff) | (val << 8);
	if ((ciabcrb & 1) == 0)
	    ciabtb = ciablb;
	if (ciabcrb & 8) {
	    ciabtb = ciablb;
	    ciabcrb |= 1;
	}
	CIA_calctimers();
	break;
     case 8:					// todlo
	if (ciabcrb & 0x80) {
	    ciabalarm = (ciabalarm & ~0xff) | val;
	} else {
	    ciabtod = (ciabtod & ~0xff) | val;
	    ciabtodon = 1;
	}
	break;
     case 9:					// todmid
	if (ciabcrb & 0x80) {
	    ciabalarm = (ciabalarm & ~0xff00) | (val << 8);
	} else {
	    ciabtod = (ciabtod & ~0xff00) | (val << 8);
	    ciabtodon = 0;
	}
	break;
     case 10:				// todhi
	if (ciabcrb & 0x80) {
	    ciabalarm = (ciabalarm & ~0xff0000) | (val << 16);
	} else {
	    ciabtod = (ciabtod & ~0xff0000) | (val << 16);
	    ciabtodon = 0;
	}
	break;
     case 12:				// sdr
	ciabsdr = val;
	break;
     case 13:				// lcr
	setclr(&ciabimask,val);
	break;
     case 14:				// cra
	CIA_update();
	ciabcra = val;
	if (ciabcra & 0x10) {
	    ciabcra &= ~0x10;
	    ciabta = ciabla;
	}
	CIA_calctimers();
	break;
     case 15:				// crb
	CIA_update();
	ciabcrb = val;
	if (ciabcrb & 0x10) {
	    ciabcrb &= ~0x10;
	    ciabtb = ciablb;
	}
	CIA_calctimers();
	break;
    }

	cia_unlock();
}

void CIA_reset(void)
{
    kback = 1;
    kbstate = 0;

    ciaatlatch = ciabtlatch = 0;
    ciaapra = 3;
    ciaatod = ciabtod = 0; ciaatodon = ciabtodon = 0;
    ciaaicr = ciabicr = ciaaimask = ciabimask = 0;
    ciaacra = ciaacrb = ciabcra = ciabcrb = 0x4; /* outmode = toggle; */
    ciaala = ciaalb = ciabla = ciablb = ciaata = ciaatb = ciabta = ciabtb = 0xFFFF;
    div10 = 0;
    lastdiv10 = 0;
    CIA_calctimers();
    ciabpra = 0x8C;

    /* we don't want ersatzkick or kickstart roms in uade */
    /*
      if (! ersatzkickfile)
      map_banks(&kickmem_bank, 0, 32);
    */
}

void dumpcia(void)
{
    fprintf(stderr,"A: CRA: %02x, CRB: %02x, IMASK: %02x, TOD: %08lx %7s TA: %04lx, TB: %04lx\n",
	   (int)ciaacra, (int)ciaacrb, (int)ciaaimask, ciaatod,
	   ciaatlatch ? " latched" : "", ciaata, ciaatb);
    fprintf(stderr,"B: CRA: %02x, CRB: %02x, IMASK: %02x, TOD: %08lx %7s TA: %04lx, TB: %04lx\n",
	   (int)ciabcra, (int)ciabcrb, (int)ciabimask, ciabtod,
	   ciabtlatch ? " latched" : "", ciabta, ciabtb);
}

/* CIA memory access */

static uae_u32 cia_lget (uaecptr) REGPARAM;
static uae_u32 cia_wget (uaecptr) REGPARAM;
static uae_u32 cia_bget (uaecptr) REGPARAM;
static void cia_lput (uaecptr, uae_u32) REGPARAM;
static void cia_wput (uaecptr, uae_u32) REGPARAM;
static void cia_bput (uaecptr, uae_u32) REGPARAM;

addrbank cia_bank = {
    cia_lget, cia_wget, cia_bget,
    cia_lput, cia_wput, cia_bput,
    default_xlate, default_check
};

static uae_u32 REGPARAM2 cia_lget (uaecptr addr)
{
    return cia_bget(addr+3);
}

static uae_u32 REGPARAM2 cia_wget (uaecptr addr)
{
    return cia_bget(addr+1);
}

static uae_u32 REGPARAM2 cia_bget (uaecptr addr)
{
    if ((addr & 0x3001) == 0x2001)
	return ReadCIAA((addr & 0xF00) >> 8);
    if ((addr & 0x3001) == 0x1000)
	return ReadCIAB((addr & 0xF00) >> 8);
    return 0;
}

static void REGPARAM2 cia_lput (uaecptr addr, uae_u32 value)
{
    cia_bput(addr+3,value); /* FIXME ? */
}

static void REGPARAM2 cia_wput (uaecptr addr, uae_u32 value)
{
    cia_bput(addr+1,value);
}

static void REGPARAM2 cia_bput (uaecptr addr, uae_u32 value)
{
    if ((addr & 0x3001) == 0x2001)
	WriteCIAA((addr & 0xF00) >> 8,value);
    if ((addr & 0x3001) == 0x1000)
	WriteCIAB((addr & 0xF00) >> 8,value);
}

/* battclock memory access */

static uae_u32 clock_lget (uaecptr) REGPARAM;
static uae_u32 clock_wget (uaecptr) REGPARAM;
static uae_u32 clock_bget (uaecptr) REGPARAM;
static void clock_lput (uaecptr, uae_u32) REGPARAM;
static void clock_wput (uaecptr, uae_u32) REGPARAM;
static void clock_bput (uaecptr, uae_u32) REGPARAM;

addrbank clock_bank = {
    clock_lget, clock_wget, clock_bget,
    clock_lput, clock_wput, clock_bput,
    default_xlate, default_check
};

static uae_u32 REGPARAM2 clock_lget (uaecptr addr)
{
    return clock_bget(addr+3);
}

static uae_u32 REGPARAM2 clock_wget (uaecptr addr)
{
    return clock_bget(addr+1);
}

static uae_u32 REGPARAM2 clock_bget (uaecptr addr)
{
    time_t t = time(NULL);
    struct tm *ct;
    ct=localtime(&t);
    switch (addr & 0x3f)
    {
     case 0x03: return ct->tm_sec % 10;
     case 0x07: return ct->tm_sec / 10;
     case 0x0b: return ct->tm_min % 10;
     case 0x0f: return ct->tm_min / 10;
     case 0x13: return ct->tm_hour % 10;
     case 0x17: return ct->tm_hour / 10;
     case 0x1b: return ct->tm_mday % 10;
     case 0x1f: return ct->tm_mday / 10;
     case 0x23: return (ct->tm_mon+1) % 10;
     case 0x27: return (ct->tm_mon+1) / 10;
     case 0x2b: return ct->tm_year % 10;
     case 0x2f: return ct->tm_year / 10;

     case 0x33: return ct->tm_wday;  /*Hack by -=SR=- */
     case 0x37: return clock_control_d;
     case 0x3b: return clock_control_e;
     case 0x3f: return clock_control_f;
    }
    return 0;
}

static void REGPARAM2 clock_lput (uaecptr addr, uae_u32 value)
{
    /* No way */
}

static void REGPARAM2 clock_wput (uaecptr addr, uae_u32 value)
{
    /* No way */
}

static void REGPARAM2 clock_bput (uaecptr addr, uae_u32 value)
{
    switch (addr & 0x3f)
    {
     case 0x37: clock_control_d=value; break;
     case 0x3b: clock_control_e=value; break;
     case 0x3f: clock_control_f=value; break;
    }
}
