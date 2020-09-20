
#define __USE_INLINE__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/chipset.h>

#include "hardware/cia.h"
#include "hardware/custom.h"

struct IntuitionIFace * IIntuition = NULL;
struct Library *IntuitionBase = NULL;

struct chipsetIFace * Ichipset = NULL;
struct Library *chipsetBase = NULL;

struct GraphicsIFace * IGraphics = NULL;
struct Library *GraphicsBase = NULL;

struct Window *My_Window = NULL;

#define IDCMP_COMMON IDCMP_MOUSEBUTTONS | IDCMP_INACTIVEWINDOW | IDCMP_ACTIVEWINDOW  | \
	IDCMP_CHANGEWINDOW | IDCMP_MOUSEMOVE | IDCMP_REFRESHWINDOW | IDCMP_RAWKEY | \
	IDCMP_EXTENDEDMOUSE | IDCMP_CLOSEWINDOW | IDCMP_NEWSIZE | IDCMP_INTUITICKS | IDCMP_MENUPICK | IDCMP_GADGETUP

#define window_height 340
#define window_width 320
int window_left = 640 - window_width;
int window_top = 30;

BOOL openWin()
{
	My_Window = OpenWindowTags( NULL,
				WA_PubScreen,       (ULONG) NULL,
				WA_Left,			window_left,
				WA_Top,			window_top,
				WA_InnerWidth,		window_width,
				WA_InnerHeight,	window_height,

				WA_SimpleRefresh,	TRUE,
				WA_CloseGadget,	TRUE,
				WA_DepthGadget,	TRUE,
				WA_DragBar,		TRUE,
				WA_Borderless,	FALSE,
				WA_SizeGadget,	FALSE,
				WA_SizeBBottom,	TRUE,
				WA_NewLookMenus,	TRUE,

				WA_Title,			"chipset.library CIAA",
				WA_Activate,		TRUE,
				WA_Flags,			WFLG_RMBTRAP| WFLG_REPORTMOUSE,
				WA_IDCMP,		IDCMP_COMMON,
			TAG_DONE);

	return (My_Window) ? TRUE : FALSE;
}

bool powerLed = false;
bool diskLed = false;


struct  xx {
	uint32_t addr;
	char *name;
} ;

struct xx list[] =
{
	{0xBFE001, "pra"},
	{0xBFE101, "prb"},
	{0xBFE201, "ddra"},
	{0xBFE301, "ddrb"},
	{0xBFE401, "talo"},
	{0xBFE501, "tahi"},
	{0xBFE601, "tblo"},
	{0xBFE701, "tbhi"},
	{0xBFE801, "todlo"},
	{0xBFE901, "todmid"},
	{0xBFEA01, "todhi"},
	{0xBFEB01, "<not used>"},
	{0xBFEC01, "sdr"},
	{0xBFED01, "icr"},
	{0xBFEE01, "cra"},
	{0xBFEF01, "crb"},
	{0x0,""}
};

void print_item(int i, int y)
{
	char hex[20];

	sprintf(hex,"%08x", list[i].addr);

	Move( My_Window -> RPort, 20,y+20 );
	Text( My_Window -> RPort, hex, strlen(hex) );

	Move( My_Window -> RPort, 120,y+20 );
	Text( My_Window -> RPort, list[i].name, strlen(list[i].name));

	sprintf(hex,"%08x",spyCIAA( list[i].addr));

	Move( My_Window -> RPort, 220,y+20 );
	Text( My_Window -> RPort, hex, strlen(hex) );
}

void update_print_item(int i, int y)
{
	char hex[20];

	sprintf(hex,"%08x",spyCIAA( list[i].addr));

	Move( My_Window -> RPort, 220,y+20 );
	Text( My_Window -> RPort, hex, strlen(hex) );
}


void show_stats()
{
	int i,x,y;
	x = My_Window -> BorderLeft;
	y = My_Window -> BorderTop;

	SetAPen( My_Window -> RPort, 1 );

	for (i=0;list[i].addr;i++)
	{
		print_item( i, y+ i * 20 );
	}
}

void update_show_stats()
{
	int i,x,y;
	x = My_Window -> BorderLeft;
	y = My_Window -> BorderTop;

	SetAPen( My_Window -> RPort, 1 );

	for (i=0;list[i].addr;i++)
	{
		update_print_item( i, y+ i * 20 );
	}
}

bool running = true;

void events()
{
	struct IntuiMessage *msg;

	while (msg = (struct IntuiMessage *) GetMsg( My_Window -> UserPort) )
	{
		switch (msg -> Class) 
		{
			case IDCMP_CLOSEWINDOW: 
					running = false;
					break;
		}

		ReplyMsg( (struct Message*) msg );
	}
}

struct CIA *_ciaa = 0xbfe001;

int main()
{

	if (init() == FALSE)
	{
		close_libs();
		return 0;
	}	

	if (openWin() == FALSE)
	{
		close_libs();
		return 0;
	}

	show_stats();

	do
	{
		events();

		update_show_stats();
		Delay(1);
	} while ( running);


	CloseWindow(My_Window);

	close_libs();
	return 0;
}

