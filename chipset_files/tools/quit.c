#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <proto/exec.h>
#include "hardware/cia.h"
#include "hardware/custom.h"

#define __USE_INLINE__

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

	printf("chip ram at: %08x\n",chipAddressToHost(0));

	if (args==2)
	{
		printf("%s\n",arg[1]);

		if (strcasecmp(arg[1],"FALSE")==0) quit = 0x00;
	}

	printf("quit = %x\n",quit);

	writeChipByte(0x00000000, quit);

/*
        for (;;)        // busy wait
        {
                if (1&readChipByte(&(_ciaa ->ciaicr))) continue;                        // Wait for timer expired flag
                writeChipByte(&(_ciaa ->ciacra),CIAB_LED ^ readChipByte(&(_ciaa ->ciacra)));     // blink light
                writeChipByte(&(_ciaa ->ciacra),1|readChipByte(&(_ciaa ->ciacra)));             // Restart timer
        }
*/

        close_libs();

        return 0;
}

