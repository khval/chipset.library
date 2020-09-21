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



struct CIA *_ciaa = (struct CIA *) 0xbfe001;

int main()
{
        uint32_t d0;
        uint32_t a4 = (uint32_t) _ciaa;
	double cia_time_us = 0.0f;

        if (init()==FALSE) 
        {
                close_libs();
                return 0;                
        }

	cia_time_us = 100000.0f / 70938.92f ;	// 1 sec / hz

	printf("CIASpeed: %0.3lf us, %d Hz\n",cia_time_us, (ULONG) (100000.0f / cia_time_us) );

	printf("Enter the clock speed in micro seconds:\n");
	scanf("%lf",&cia_time_us);

	setCIAClockSpeed(cia_time_us);

	close_libs();
	return 0;
}

