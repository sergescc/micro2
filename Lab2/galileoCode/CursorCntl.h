#ifndef CURSOR_CONTROL
#define CURSOR_CONTROL



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