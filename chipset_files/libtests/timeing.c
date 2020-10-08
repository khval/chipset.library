#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "hardware/cia.h"
#include "hardware/custom.h"

#define __USE_INLINE__

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/chipset.h>

struct chipsetIFace * Ichipset = NULL;
struct Library *chipsetBase = NULL;

struct IntuitionIFace * IIntuition = NULL;
struct Library *IntuitionBase = NULL;

struct GraphicsIFace * IGraphics = NULL;
struct Library *GraphicsBase = NULL;
/*
;
; 8520_timing.asm
;
; A complete 8520 timing example.  This blinks the power light at (exactly)
; 3 milisecond intervals.  It takes over the machine, so watch out!
;
; The base Amiga crytal frequencies are:
;           NTSC    28.63636  MHz
;           PAL     28.37516  MHz
;
; The two 16 bit timers on the 8520 chips each count down at 1/10 the CPU
; clock, or 0.715909 MHz.  That works out to 1.3968255 microseconds per count.
; Under PAL the countdown is slightly slower, 0.709379 MHz.
;
; To wait 1/100 second would require waiting 10,000 microseconds.
; The timer register would be set to (10,000 / 1.3968255 = 7159).
;
; To wait 3 miliseconds would require waiting 3000 microseconds.
; The register would be set to (3000 / 1.3968255 = 2148).
;
        INCLUDE "hardware/cia.i"
        INCLUDE "hardware/custom.i"
;
        XREF    _ciaa
        XREF    _ciab
        XREF    _custom
;
        lea     _custom,a3              ; Base of custom chips
        lea     _ciaa,a4                ; Get base address if CIA-A
;
*/



#define TIME 2148

// _ciaa is on an ODD address (e.g. the low byte) -- $bfe001
// _ciab is on an EVEN address (e.g. the high byte) -- $bfd000

struct CIA *_ciaa = (struct CIA *) 0xbfe001;



int main()
{
	uint32_t icr;
	uint32_t d0;

/*
;	 Not supported by chipset.library, this OCS/AGA chipset

        move.w  #$7fff,dmacon(a3)       ; Kill all chip interrupts
;
*/

        if (init()==FALSE) 
        {
                close_libs();
                return 0;                
        }

//	setCIAClockSpeed(500.0);

        d0 = readChipByte( &(_ciaa ->ciacra));
        d0 &= (0x80 | 0x40);
        d0 |= 0x08;
        writeChipByte(&(_ciaa ->ciacra), (uint8_t) d0);
        writeChipByte(&(_ciaa ->ciaicr), 0x7F);

/*
;----Setup, only do once
;----This sets all bits needed for timer A one-shot mode.
        move.b  ciacra(a4),d0           ;Set control register A on CIAA
        and.b   #%11000000,d0           ;Don't trash bits we are not
        or.b    #%00001000,d0           ;using...
        move.b  d0,ciacra(a4)
        move.b  #%01111111,ciaicr(a4)   ;Clear all 8520 interrupts
;
;----Set time (low byte THEN high byte)
;----And the low order with $ff
;----Shift the high order by 8
;
*/

        writeChipByte(&(_ciaa ->ciatalo),TIME & 0xFF);
        writeChipByte(&(_ciaa ->ciatahi),TIME >> 8);

/*
TIME    equ     2148
        move.b  #(TIME&$FF),ciatalo(a4)
        move.b  #(TIME>>8),ciatahi(a4)
;
;----Wait for the timer to count down
*/

	for (;;)        // busy wait
	{
		if (readChipByte(0xBFE001) & 1L<<6)  break;		// wait for left mouse button

		icr = readChipByte(&(_ciaa ->ciaicr));
		if ((1&icr)==0) continue;                        // Wait for timer expired flag

		bitChgChipByte(&(_ciaa ->ciapra),CIAB_LED);     // blink light
		bitSetChipByte(&(_ciaa ->ciacra),0);             // Restart timer

//		getchar();
        }

/*
busy_wait:
        btst.b  #0,ciaicr(a4)           ;Wait for timer expired flag
        beq.s   busy_wait
        bchg.b  #CIAB_LED,ciapra(a4)    ;Blink light
        bset.b  #0,ciacra(a4)           ;Restart timer
        bra.s   busy_wait

        END
*/
        close_libs();

        return 0;
}

