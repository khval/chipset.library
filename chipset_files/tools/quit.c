#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <proto/exec.h>
#include "hardware/cia.h"
#include "hardware/custom.h"

#define __USE_INLINE__

#include <proto/dos.h>

#include <proto/chipset.h>

struct IntuitionIFace * IIntuition = NULL;
struct Library *IntuitionBase = NULL;

struct chipsetIFace * Ichipset = NULL;
struct Library *chipsetBase = NULL;

struct GraphicsIFace * IGraphics = NULL;
struct Library *GraphicsBase = NULL;



struct CIA *_ciaa = 0xbfe001;

int main( int args, char *arg[] )
{
//	uint32_t d0;
//	uint32_t a4 = _ciaa;
	uint32_t quit = 0xFF;

        if (init()==FALSE) 
        {
                close_libs();
                return 0;                
        }

	bitChgChipByte(0xBFE001,6);

	Delay(20);

	bitClrChipByte(0xBFE001,6);


        close_libs();

        return 0;
}

