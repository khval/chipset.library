
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <proto/exec.h>
#include <proto/dos.h>

#define __USE_INLINE__
#include <proto/chipset.h>

struct chipsetIFace * Ichipset = NULL;
struct Library *chipsetBase = NULL;

struct IntuitionIFace * IIntuition = NULL;
struct Library *IntuitionBase = NULL;

struct GraphicsIFace * IGraphics = NULL;
struct Library *GraphicsBase = NULL;

int main()
{
	if (init())
	{
		printf("its open, not bad\n");
		printf("press enter to set CIA Timing Accuracy to 40 us, check CPU usage/load\n");
		getchar();
		setCIATimingAccuracy(40);
		printf("press enter to set CIA Timing Accuracy to 20 us, check CPU usage/load\n");
		getchar();
		setCIATimingAccuracy(20);
		printf("press enter to set CIA Timing Accuracy to 10 us, check CPU usage/load\n");
		getchar();
		setCIATimingAccuracy(10);
		getchar();
	}
	close_libs();

	return 0;
}


