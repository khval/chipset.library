
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <proto/exec.h>
#include <proto/dos.h>

#define __USE_INLINE__
#include <proto/chipset.h>

extern struct IntuitionIFace * IIntuition;
extern struct Library *IntuitionBase;

extern struct GraphicsIFace * IGraphics;
extern struct Library *GraphicsBase;

extern struct chipsetIFace * Ichipset;
extern struct Library *chipsetBase;

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

BOOL init()
{
	if ( ! open_lib( "chipset.library", 53L , "main", 1, &chipsetBase, (struct Interface **) &Ichipset  ) ) return FALSE;
	if ( ! open_lib( "intuition.library", 53L , "main", 1, &IntuitionBase, (struct Interface **) &IIntuition  ) ) return FALSE;
	if ( ! open_lib( "graphics.library", 53L , "main", 1, &IntuitionBase, (struct Interface **) &IGraphics  ) ) return FALSE;
	return TRUE;
}

#define close_lib(b,i)			\
	if (b) IExec->CloseLibrary(b);	\
	if (i) IExec->DropInterface( (struct Interface *) i );	\
	b = NULL; i = NULL;	

void close_libs()
{
	close_lib( chipsetBase, Ichipset );
	close_lib( IntuitionBase, IIntuition );
	close_lib( IntuitionBase, IGraphics );
}
