 /*
  * UAE - The Un*x Amiga Emulator
  *
  * Custom chip emulation
  *
  * Copyright 1995-2002 Bernd Schmidt
  * Copyright 1995 Alessandro Bissacco
  * Copyright 2000-2002 Toni Wilen
  */

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "sysconfig.h"
#include "sysdeps.h"

#include <ctype.h>
#include <assert.h>

#include "options.h"
// #include "threaddep/thread.h"
// #include "uae.h"
// #include "gensound.h"
// #include "sounddep/sound.h"
#include "events.h"
#include "memory.h"

#include "../NallePUH/src/puh.h"

#include "custom.h"
#include "cia.h"
#include "blitter.h"
#include "inputdevice.h"

static void BLTADAT (uae_u16 v)
{
    maybe_blit (0);

    blt_info.bltadat = v;
}
/*
 * "Loading data shifts it immediately" says the HRM. Well, that may
 * be true for BLTBDAT, but not for BLTADAT - it appears the A data must be
 * loaded for every word so that AFWM and ALWM can be applied.
 */
static void BLTBDAT (uae_u16 v)
{
    maybe_blit (0);

    if (bltcon1 & 2)
	blt_info.bltbhold = v << (bltcon1 >> 12);
    else
	blt_info.bltbhold = v >> (bltcon1 >> 12);
    blt_info.bltbdat = v;
}
static void BLTCDAT (uae_u16 v) { maybe_blit (0); blt_info.bltcdat = v; }

static void BLTAMOD (uae_u16 v) { maybe_blit (1); blt_info.bltamod = (uae_s16)(v & 0xFFFE); }
static void BLTBMOD (uae_u16 v) { maybe_blit (1); blt_info.bltbmod = (uae_s16)(v & 0xFFFE); }
static void BLTCMOD (uae_u16 v) { maybe_blit (1); blt_info.bltcmod = (uae_s16)(v & 0xFFFE); }
static void BLTDMOD (uae_u16 v) { maybe_blit (1); blt_info.bltdmod = (uae_s16)(v & 0xFFFE); }

static void BLTCON0 (uae_u16 v) { maybe_blit (0); bltcon0 = v; blinea_shift = v >> 12; }
/* The next category is "Most useless hardware register".
 * And the winner is... */
static void BLTCON0L (uae_u16 v)
{
//    if (! (currprefs.chipset_mask & CSMASK_ECS_AGNUS)) return;

    maybe_blit (0); bltcon0 = (bltcon0 & 0xFF00) | (v & 0xFF);
}
static void BLTCON1 (uae_u16 v) { maybe_blit (0); bltcon1 = v; }

static void BLTAFWM (uae_u16 v) { maybe_blit (0); blt_info.bltafwm = v; }
static void BLTALWM (uae_u16 v) { maybe_blit (0); blt_info.bltalwm = v; }

static void BLTAPTH (uae_u16 v) { maybe_blit (0); bltapt = (bltapt & 0xffff) | ((uae_u32)v << 16); }
static void BLTAPTL (uae_u16 v) { maybe_blit (0); bltapt = (bltapt & ~0xffff) | (v & 0xFFFE); }
static void BLTBPTH (uae_u16 v) { maybe_blit (0); bltbpt = (bltbpt & 0xffff) | ((uae_u32)v << 16); }
static void BLTBPTL (uae_u16 v) { maybe_blit (0); bltbpt = (bltbpt & ~0xffff) | (v & 0xFFFE); }
static void BLTCPTH (uae_u16 v) { maybe_blit (0); bltcpt = (bltcpt & 0xffff) | ((uae_u32)v << 16); }
static void BLTCPTL (uae_u16 v) { maybe_blit (0); bltcpt = (bltcpt & ~0xffff) | (v & 0xFFFE); }
static void BLTDPTH (uae_u16 v) { maybe_blit (0); bltdpt = (bltdpt & 0xffff) | ((uae_u32)v << 16); }
static void BLTDPTL (uae_u16 v) { maybe_blit (0); bltdpt = (bltdpt & ~0xffff) | (v & 0xFFFE); }

static void BLTSIZE (uae_u16 v)
{
	maybe_blit (0);

	blt_info.vblitsize = v >> 6;
	blt_info.hblitsize = v & 0x3F;
	if (!blt_info.vblitsize) blt_info.vblitsize = 1024;
	if (!blt_info.hblitsize) blt_info.hblitsize = 64;

	bltstate = BLT_init;
	do_blitter ();
}

static void BLTSIZV (uae_u16 v)
{
//    if (! (currprefs.chipset_mask & CSMASK_ECS_AGNUS))	return;

	maybe_blit (0);
	oldvblts = v & 0x7FFF;
}

static void BLTSIZH (uae_u16 v)
{
//    if (! (currprefs.chipset_mask & CSMASK_ECS_AGNUS)) 	return;

	maybe_blit (0);
	blt_info.hblitsize = v & 0x7FF;
	blt_info.vblitsize = oldvblts;
	if (!blt_info.vblitsize) blt_info.vblitsize = 32768;
	if (!blt_info.hblitsize) blt_info.hblitsize = 0x800;
	bltstate = BLT_init;
	do_blitter ();
}

uae_u32 last_custom_value = 0;

bool custom_wput ( uaecptr addr, uae_u32 value)
{
	addr &= 0x1FE;
	last_custom_value = value;
	switch (addr) {

/*
	case 0x020: DSKPTH (value); break;
	case 0x022: DSKPTL (value); break;
	case 0x024: DSKLEN (value, hpos); break;
	case 0x026: DSKDAT (value); break;
*/

//	case 0x02A: VPOSW (value); break;
//	case 0x02E: COPCON (value); break;
//	case 0x030: SERDAT (value); break;
//	case 0x032: SERPER (value); break;
//	case 0x034: POTGO (value); break;		// mouse ?

	case 0x040: BLTCON0 (value); break;	// blitter control register 0
	case 0x042: BLTCON1 (value); break;

	case 0x044: BLTAFWM (value); break;	// blitter first word mask for soucre A
	case 0x046: BLTALWM (value); break;	// blitter lasr word mask for source A

	case 0x048: BLTCPTH (value); break;	// Blitter pointer to source C
	case 0x04A: BLTCPTL (value); break;

	case 0x04C: BLTBPTH (value); break;	// Blitter pointer to source B
	case 0x04E: BLTBPTL (value); break;

	case 0x050: BLTAPTH (value); break;	// Blitter pointer to source A
	case 0x052: BLTAPTL (value); break;

	case 0x054: BLTDPTH (value); break;	// Blitter pointer to dest D 
	case 0x056: BLTDPTL (value); break;

	case 0x058: BLTSIZE (value); break;		// blitter start and size (win/width/height)

	case 0x064: BLTAMOD (value); break;	// blitter modulo for source C
	case 0x062: BLTBMOD (value); break;	// blitter modulo for source B
	case 0x060: BLTCMOD (value); break;	// blitter modulo for source A
	case 0x066: BLTDMOD (value); break;	// blitter modulo for dest D

	case 0x070: BLTCDAT (value); break;	// blitter source C data register
	case 0x072: BLTBDAT (value); break;	// blitter source B data register
	case 0x074: BLTADAT (value); break;	// blitter source A data register

	// Not emulating disk and cooper.
/*
	case 0x07E: DSKSYNC (value); break;
	case 0x080: COP1LCH (value); break;
	case 0x082: COP1LCL (value); break;
	case 0x084: COP2LCH (value); break;
	case 0x086: COP2LCL (value); break;
	case 0x088: COPJMP (1); break;
	case 0x08A: COPJMP (2); break;
*/

	// Not emulating display
/*
	case 0x08E: DIWSTRT (hpos, value); break;
	case 0x090: DIWSTOP (hpos, value); break;
	case 0x092: DDFSTRT (hpos, value); break;
	case 0x094: DDFSTOP (hpos, value); break;
*/

//	emulated by NallePuh

/*
	case 0x096: DMACON (hpos, value); break;

//	case 0x098: CLXCON (value); break;		// Collision control, not needed.
	case 0x09A: INTENA (value); break;		// Emulated by NallePuh
	case 0x09C: INTREQ (value); break;		// Emulated by NallePuh
	case 0x09E: ADKCON (value); break;

	case 0x0A0: AUDxLCH (0, value); break;
	case 0x0A2: AUDxLCL (0, value); break;
	case 0x0A4: AUDxLEN (0, value); break;
	case 0x0A6: AUDxPER (0, value); break;
	case 0x0A8: AUDxVOL (0, value); break;
	case 0x0AA: AUDxDAT (0, value); break;

	case 0x0B0: AUDxLCH (1, value); break;
	case 0x0B2: AUDxLCL (1, value); break;
	case 0x0B4: AUDxLEN (1, value); break;
	case 0x0B6: AUDxPER (1, value); break;
	case 0x0B8: AUDxVOL (1, value); break;
	case 0x0BA: AUDxDAT (1, value); break;

	case 0x0C0: AUDxLCH (2, value); break;
	case 0x0C2: AUDxLCL (2, value); break;
	case 0x0C4: AUDxLEN (2, value); break;
	case 0x0C6: AUDxPER (2, value); break;
	case 0x0C8: AUDxVOL (2, value); break;
	case 0x0CA: AUDxDAT (2, value); break;

	case 0x0D0: AUDxLCH (3, value); break;
	case 0x0D2: AUDxLCL (3, value); break;
	case 0x0D4: AUDxLEN (3, value); break;
	case 0x0D6: AUDxPER (3, value); break;
	case 0x0D8: AUDxVOL (3, value); break;
	case 0x0DA: AUDxDAT (3, value); break;
*/

//	bitplane not emulated, 

/*
		case 0x0E0: BPLxPTH (hpos, value, 0); break;
		case 0x0E2: BPLxPTL (hpos, value, 0); break;
		case 0x0E4: BPLxPTH (hpos, value, 1); break;
		case 0x0E6: BPLxPTL (hpos, value, 1); break;
		case 0x0E8: BPLxPTH (hpos, value, 2); break;
		case 0x0EA: BPLxPTL (hpos, value, 2); break;
		case 0x0EC: BPLxPTH (hpos, value, 3); break;
		case 0x0EE: BPLxPTL (hpos, value, 3); break;
		case 0x0F0: BPLxPTH (hpos, value, 4); break;
		case 0x0F2: BPLxPTL (hpos, value, 4); break;
		case 0x0F4: BPLxPTH (hpos, value, 5); break;
		case 0x0F6: BPLxPTL (hpos, value, 5); break;
		case 0x0F8: BPLxPTH (hpos, value, 6); break;
		case 0x0FA: BPLxPTL (hpos, value, 6); break;
		case 0x0FC: BPLxPTH (hpos, value, 7); break;
		case 0x0FE: BPLxPTL (hpos, value, 7); break;

		case 0x100: BPLCON0 (hpos, value); break;
		case 0x102: BPLCON1 (hpos, value); break;
		case 0x104: BPLCON2 (hpos, value); break;
		case 0x106: BPLCON3 (hpos, value); break;

		case 0x108: BPL1MOD (hpos, value); break;
		case 0x10A: BPL2MOD (hpos, value); break;
		case 0x10E: CLXCON2 (value); break;

		case 0x110: BPL1DAT (hpos, value); break;
		case 0x112: BPL2DAT (value); break;
		case 0x114: BPL3DAT (value); break;
		case 0x116: BPL4DAT (value); break;
		case 0x118: BPL5DAT (value); break;
		case 0x11A: BPL6DAT (value); break;
		case 0x11C: BPL7DAT (value); break;
		case 0x11E: BPL8DAT (value); break;
*/
		case 0x180: case 0x182: case 0x184: case 0x186: case 0x188: case 0x18A:
		case 0x18C: case 0x18E: case 0x190: case 0x192: case 0x194: case 0x196:
		case 0x198: case 0x19A: case 0x19C: case 0x19E: case 0x1A0: case 0x1A2:
		case 0x1A4: case 0x1A6: case 0x1A8: case 0x1AA: case 0x1AC: case 0x1AE:
		case 0x1B0: case 0x1B2: case 0x1B4: case 0x1B6: case 0x1B8: case 0x1BA:

//		Not emulating colors or sprites

/*	
		case 0x1BC: case 0x1BE:
			COLOR_WRITE (hpos, value & 0xFFF, (addr & 0x3E) / 2);
			break;
		case 0x120: case 0x124: case 0x128: case 0x12C:
		case 0x130: case 0x134: case 0x138: case 0x13C:
			SPRxPTH (hpos, value, (addr - 0x120) / 4);
			break;
		case 0x122: case 0x126: case 0x12A: case 0x12E:
		case 0x132: case 0x136: case 0x13A: case 0x13E:
			SPRxPTL (hpos, value, (addr - 0x122) / 4);
			break;
		case 0x140: case 0x148: case 0x150: case 0x158:
		case 0x160: case 0x168: case 0x170: case 0x178:
			SPRxPOS (hpos, value, (addr - 0x140) / 8);
			break;
		case 0x142: case 0x14A: case 0x152: case 0x15A:
		case 0x162: case 0x16A: case 0x172: case 0x17A:
			SPRxCTL (hpos, value, (addr - 0x142) / 8);
			break;
		case 0x144: case 0x14C: case 0x154: case 0x15C:
		case 0x164: case 0x16C: case 0x174: case 0x17C:
			SPRxDATA (hpos, value, (addr - 0x144) / 8);
			break;
		case 0x146: case 0x14E: case 0x156: case 0x15E:
		case 0x166: case 0x16E: case 0x176: case 0x17E:
			SPRxDATB (hpos, value, (addr - 0x146) / 8);
			break;
*/

//		case 0x36: JOYTEST (value); break;			// 

		case 0x5A: BLTCON0L (value); break;		// Blitter control register
		case 0x5C: BLTSIZV (value); break;			// Blitter vertical size
		case 0x5E: BLTSIZH (value); break;			// Blitter h size

		// not emulating ECS/AGA video

/*
		case 0x1E4: DIWHIGH (hpos, value); break;	// Display window upper bits for start, stop
		case 0x10C: BPLCON4 (hpos, value); break;	// Bit Plane Control Register (display mask)
		case 0x1DC: BEAMCON0 (value); break;
		case 0x1C0: if (htotal != value) { htotal = value; varsync (); } break;
		case 0x1C2: if (hsstop != value) { hsstop = value; varsync (); } break;
		case 0x1C4: if (hbstrt != value) { hbstrt = value; varsync (); } break;
		case 0x1C6: if (hbstop != value) { hbstop = value; varsync (); } break;
		case 0x1C8: if (vtotal != value) { vtotal = value; varsync (); } break;
		case 0x1CA: if (vsstop != value) { vsstop = value; varsync (); } break;
		case 0x1CC: if (vbstrt < value || vbstrt > value + 1) { vbstrt = value; varsync (); } break;
		case 0x1CE: if (vbstop < value || vbstop > value + 1) { vbstop = value; varsync (); } break;
		case 0x1DE: if (hsstrt != value) { hsstrt = value; varsync (); } break;
		case 0x1E0: if (vsstrt != value) { vsstrt = value; varsync (); } break;
		case 0x1E2: if (hcenter != value) { hcenter = value; varsync (); } break;
		case 0x1FC: FMODE (value); break;
*/


		default:
				return false;
	}

	return true;
}


uae_u32 DENISEID()
{
	return 0xFFFF;
}


uae_u16 DMACONR (void)
{
    return ( pd->m_DMACON | (bltstate==BLT_done ? 0 : 0x4000)
	    | (blt_info.blitzero ? 0x2000 : 0));
}

uae_u32 REGPARAM2 custom_wget (uaecptr addr )
{
	uae_u16 v;
	switch (addr & 0x1FE)
	{
		case 0x002: v = DMACONR (); break;		// DAM control ( and blitter status) read

//		case 0x004: v = VPOSR (); break;
//		case 0x006: v = VHPOSR (); break;
//		case 0x008: v = DSKDATR (current_hpos ()); break;
//		case 0x00A: v = JOY0DAT (); break;
//		case 0x00C: v = JOY1DAT (); break;
//		case 0x00E: v = CLXDAT (); break;
//		case 0x010: v = ADKCONR (); break;
//		case 0x012: v = POT0DAT (); break;
//		case 0x014: v = POT1DAT (); break;
//		case 0x016: v = POTGOR (); break;
//		case 0x018: v = SERDATR (); break;
//		case 0x01A: v = DSKBYTR (current_hpos ()); break;

		// this are done by NallePuh

//		case 0x01C: v = INTENAR (); break;
//		case 0x01E: v = INTREQR (); break;

		case 0x07C: v = DENISEID (); break;

		case 0x180: case 0x182: case 0x184: case 0x186: case 0x188: case 0x18A:
		case 0x18C: case 0x18E: case 0x190: case 0x192: case 0x194: case 0x196:
		case 0x198: case 0x19A: case 0x19C: case 0x19E: case 0x1A0: case 0x1A2:
		case 0x1A4: case 0x1A6: case 0x1A8: case 0x1AA: case 0x1AC: case 0x1AE:
		case 0x1B0: case 0x1B2: case 0x1B4: case 0x1B6: case 0x1B8: case 0x1BA:

//     case 0x1BC: case 0x1BE: v = COLOR_READ ((addr & 0x3E) / 2);	break;

		default:
			v = last_custom_value;
			custom_wput (addr, v);
			last_custom_value = 0xffff;
			return v;
	}
	last_custom_value = v;
	return v;
}

