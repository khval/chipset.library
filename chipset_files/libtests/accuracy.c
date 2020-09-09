
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <proto/exec.h>
#include <proto/dos.h>

#define __USE_INLINE__
#include <proto/chipset.h>

BOOL open_lib( const char *name, int ver , const char *iname, int iver, struct Library **base, struct Interface **interface)
{

	*interface = NULL;
	*base = IExec->OpenLibrary( name , ver);

	if (*base)
	{
		 *interface = IExec->GetInterface( *base,  iname , iver, TAG_END );
		if (!*interface) if (IDOS) IDOS -> Printf("Unable to getInterface %s for %s %d!\n",iname,name,ver);
	}
	else
	{
	   	if (IDOS) IDOS -> Printf("Unable to open the %s %ld!\n",name,ver);
	}

	return (*interface) ? TRUE : FALSE;
}

struct chipsetIFace * Ichipset = NULL;
struct Library *chipsetBase = NULL;

BOOL init()
{
	if ( ! open_lib( "chipset.library", 53L , "main", 1, &chipsetBase, (struct Interface **) &Ichipset  ) ) return FALSE;
	return TRUE;
}

#define close_lib(b,i)			\
	if (b) IExec->CloseLibrary(b);	\
	if (i) IExec->DropInterface( (struct Interface *) i );	\
	b = NULL; i = NULL;	

void close_libs()
{
	close_lib( chipsetBase, Ichipset );
}

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


