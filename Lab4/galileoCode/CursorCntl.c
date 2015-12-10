/** CursorCntl.c

	Created: 10/26/2015

	Created BY: Sergio Coronado

	Purpose: Defines function Function definitions
	 for terminal manipulation
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "CursorCntl.h"

#define DEBUG

//Set Color

void setColor(Colors color)
{
	switch (color)
	{
		case BLACK :
		{
			printf("\033[0;30m");
			break;
		} 
		case RED :
		{
			printf("\033[0;31m");
			break;
		}
		case GREEN :
		{
			printf("\033[0;32m");
			break;
		}
		case YELLOW :
		{
			printf("\033[0;33m");
			break;
		}
		case BLUE :
		{
			printf("\033[0;34m");
			break;
		}
		case MAGENTA :
		{
			printf("\033[0;35m");
			break;
		}
		case CYAN :
		{
			printf("\033[0;36m");
			break;
		}
		case WHITE :
		{
			printf("\033[0;37m");
		}
		case RESET :
		{
			printf("\033[m");
			break;
		}
	}
}

//goto coordintates x,y

void gotoXY(int X, int Y)
{
	#ifdef DEBUG
	printf("\n");
	#else
	printf("\033[%d;%dH", Y, X);
	#endif
}

//saves the current position of the cursor

void saveCursor()
{
	#ifdef DEBUG
	printf("\n");
	#else
	printf ("\033[s");
	#endif
}

// clears from the current position to the end of the line

void clearEOL()

{
	printf ("\033[K");
}

//clears the entire page

void clearPAGE()
{
	#ifdef DEBUG
	printf("\n");
	#else
	printf ("\033[2J");
	#endif
}

//recalls cursor to last saved position

void recallCursor()
{
	#ifdef DEBUG
	printf("\n");
	#else
	printf ("\033[u");
	#endif
}

// clears below a specified line

void clearBelowLine(int line)
{
	#ifdef DEBUG
	printf("\n");
	#else
	printf ("\033[J");
	#endif
}

//clears the entire line specified

void clearLine(int line)
{
	#ifdef DEBUG
	printf("\n");
	#else
	printf ("\033[s\033[%d;0H\033[2K\033[u", line);
	#endif
}