
#include <stdint.h>
#include <stdio.h>
#include "NallePUH/src/PUH.h"
#include "sysconfig.h"
#include "sysdeps.h"
#include "custom.h"

extern struct PUHData* pd;

void set_special (uint32_t x)
{
	printf("%s(%08x)\n",__FUNCTION__,x);

//    regs.spcflags |= x;
}

void unset_special (uint32_t x)
{
	printf("%s(%08x)\n",__FUNCTION__,x);

//    regs.spcflags &= ~x;
}

void INTREQ_0 (uint16_t v)
{
    setclr (&pd->m_INTREQ,v);
    set_special (SPCFLAG_INT);
}

void INTREQ (uint16_t v)
{
	INTREQ_0 (v);

//    if ((v & 0x8800) == 0x0800) serdat &= 0xbfff;
 
	rethink_cias ();
}

void setclr (uae_u16 *p, uae_u16 val)
{
    if (val & 0x8000)
	*p |= val & 0x7FFF;
    else
	*p &= ~val;
}
