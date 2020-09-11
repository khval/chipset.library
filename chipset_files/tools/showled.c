
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

#define window_height 30
#define window_width 100
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

				WA_Title,			"chipset.library leds",
				WA_Activate,		TRUE,
				WA_Flags,			WFLG_RMBTRAP| WFLG_REPORTMOUSE,
				WA_IDCMP,		IDCMP_COMMON,
			TAG_DONE);

	return (My_Window) ? TRUE : FALSE;
}

bool powerLed = false;
bool diskLed = false;

void draw_leds()
{
	int x,y;
	int w2 = window_width / 2;

	x = My_Window -> BorderLeft;
	y = My_Window -> BorderTop;

	RectFillColor ( My_Window -> RPort, 
		x,y,
		x+w2-1, y+window_height, 
		powerLed ? 0xFF00FF00 : 0xFF00AA00 );

	RectFillColor ( My_Window -> RPort, 
		x+w2,y, 
		x+window_width, y+window_height, 
		diskLed ? 0xFFFF0000 : 0xFFAA0000 );
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
	ULONG ciacra;

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

	do
	{
		events();

		ciacra = readChipByte(&(_ciaa ->ciacra));

		powerLed = ciacra & CIAB_LED ? true : false;

		draw_leds();
		Delay(1);
	} while ( running);


	CloseWindow(My_Window);

	close_libs();
	return 0;
}

