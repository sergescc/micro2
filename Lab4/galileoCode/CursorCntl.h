/** CursorCntl.h

	Created: 10/26/2015

	Created BY: Sergio Coronado

	Purpose: Function prototypes for terminal manipulation

*/




#ifndef CURSOR_CONTROL
#define CURSOR_CONTROL

//Colors that can be set for terminal text

typedef enum {
	BLACK, 
	RED,
	GREEN,
	YELLOW,
	BLUE,
	MAGENTA,
	CYAN,
	WHITE,
	RESET
}Colors;



void setColor( Colors color);

void gotoXY(int X, int Y);

void saveCursor();

void clearEOL();

void clearPAGE();

void recallCursor();

void clearBelowLine(int line);

void clearLine(int line);

#endif