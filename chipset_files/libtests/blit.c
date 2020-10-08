
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
#include "hardware/blit.h"

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
#define window_width 150
int window_left = 640 - window_width;
int window_top = 30;

void byteToPixels( char c, int x, int y);

int Screenwidth = 320;
int Screenheigh = 200;

void displayScreen( char *data, int w, int h, int maxbytes )
{
	int x,y;
	int c = 0;

	for (y=0;y<h;y++)
	{
		for (x=0;x<w;x++)
		{
			if (c<maxbytes) byteToPixels( data[c], x, y);
			c++;
		}
	}	
}

int screen_bytes = 0;
char *screen = NULL;

bool init_screen()
{

	screen_bytes = Screenwidth/8 * Screenheigh;
	screen = malloc( screen_bytes );	// create a tmp screen !!!
	if (screen == NULL) return false;
	bzero( screen, screen_bytes );
	return true;
}

void free_screen()
{
	if (screen) free(screen);
	screen = NULL;
}

void blit(int spriteWidth, int spriteHeight, char * sprite)
{
	// setup data

	int byteoffset = 0;
	int blitw = spriteWidth / 16;
	int blith = spriteHeight;

	struct Custom *a6 = 0xDFF000;

	// test blitter

	writeChipLong( &(a6 -> bltcon0), 0x09F0000000);			// Copy A->D, no shifts, ascending mode
	writeChipLong( &(a6 -> bltafwm), 0xFFFFFFFFFFF);			// no mask of first/last word
	writeChipWord( &(a6 -> bltcon0), 0);					// A modulo = bytes to skip between lines
	writeChipWord( &(a6 -> bltdmod), Screenwidth / 8 - blitw*2 ) ;	// D Modulo
	writeChipLong( &(a6 -> bltapt), sprite);					// source top left corner
	writeChipLong( &(a6 -> bltdpt), screen + byteoffset);		// destination top left corner
	writeChipWord( &(a6 -> bltsize), blith*64+blitw);			// rectangle size, starts blit
	
}


BOOL openWin()
{
	My_Window = OpenWindowTags( NULL,
				WA_PubScreen,       (ULONG) NULL,
				WA_Left,			window_left,
				WA_Top,			window_top,
				WA_InnerWidth,		Screenwidth,
				WA_InnerHeight,	Screenheigh,

				WA_SimpleRefresh,	TRUE,
				WA_CloseGadget,	TRUE,
				WA_DepthGadget,	TRUE,
				WA_DragBar,		TRUE,
				WA_Borderless,	FALSE,
				WA_SizeGadget,	FALSE,
				WA_SizeBBottom,	TRUE,
				WA_NewLookMenus,	TRUE,

				WA_Title,			"chipset.library screen",
				WA_Activate,		TRUE,
				WA_Flags,			WFLG_RMBTRAP| WFLG_REPORTMOUSE,
				WA_IDCMP,		IDCMP_COMMON,
			TAG_DONE);

	return (My_Window) ? TRUE : FALSE;
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

struct RastPort *rp ;

uint32_t paper = 0xFFFFFFFF;

void byteToPixels( char c, int x, int y)
{
	int bit;
	x *= 8;
	x += My_Window -> BorderLeft;
	y += My_Window -> BorderTop;

	rp = My_Window -> RPort;

	for (bit = 0x80; bit; bit >>= 1 )
	{
		WritePixelColor ( rp,x,y, (c & bit) ? 0xFF000000 : paper ); 	// white paper, black pen
		x++;
	}
}

void strToBin( char **array, char *to, int size )
{
	char *c;

	while (size--)
	{
		*to = 0;
		for (c=array[0];*c;c++) 
		{
			*to<<=1;
			*to|= (*c=='1') ? 1: 0;
		}

		to++;
		array++;
	}
}

int main()
{
	int spriteWidth = 16;
	int spriteHeight = 0;
	char *spriteC[]= {
			 "00000000","00000000",
			 "00000111","11000000",
			 "00001111","11100000",
			 "00001001","11110000",
			 "00111111","11110000",
			 "00000111","11100000",
			 "00000001","10000000",
			 "01111111","11111110",
			 "01110111","11101110",
			 "00001111","11110000",
			 "00011111","11111000",
			 "00000110","01100000",
		};

	char sprite[sizeof(spriteC)/sizeof(char *) ];

	spriteHeight = sizeof(sprite) / (spriteWidth / 8);

	printf("sprite %d,%d\n",spriteWidth,spriteHeight);

	strToBin( spriteC, sprite, sizeof(sprite) );


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

	if (init_screen())
	{
		blit( spriteWidth,  spriteHeight, sprite);

		do
		{
			events();
			displayScreen( screen, Screenwidth/8, Screenheigh, screen_bytes );
			Delay(1);

			paper ^= 0x070707;
		} while ( running);
	}

	free_screen();

	CloseWindow(My_Window);

	close_libs();
	return 0;
}

