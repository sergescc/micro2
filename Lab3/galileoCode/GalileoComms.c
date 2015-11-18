/** CursorCntl.c

	Created: 10/26/2015

	Created BY: Ian Copithorne,
	Edited By; Sergio Coronado and Joseph Braught

	Purpose: function definitions for sending and receiving messages

*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>


#include "CursorCntl.h"
#include "GalileoGPIO.h"
#include "GalileoComms.h"

/* Function: Send Message

	Purpose: Sets STROBE and IO pins in order to send a message

	Input:
		msg: bitfield struct of DATA_PATH_SIZE with the message to be sent
		datapath[]: Array of GPIO pin reference numbers to use as a datapath

	Returns: void
*/
void sendMessage(message msg, int dataPath[])
{
	int i;
	int strobeHandle;
	int tmp;
	int * fileHandles;


	strobeHandle = openGPIO(Strobe, GPIO_DIRECTION_OUT);
	fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_OUT, DATA_PATH_SIZE);
	writeGPIO(strobeHandle, 0);
	usleep(1000*WAIT_MS);

	// set each a IO pin to a value specified by the msg bitfield
	for (i = 0; i < DATA_PATH_SIZE; i++)
	{
		tmp = 0;
		tmp |= msg.data >> i;
		tmp &= 0x1;
		writeGPIO(fileHandles[i], tmp);
		close(fileHandles[i]);
		
	}
	writeGPIO(strobeHandle,1);
	usleep(100*WAIT_MS);
	close(strobeHandle);
}

/* Function:ReceiveMessage

	Purpose: Sets STROBE and IO pins in order to receive a  message

	Input:
		datapath[]: Array of GPIO pin reference numbers to use as a datapath

	Rerutns: message struct with bitfieald of sixze DATA_PATH_SIZE 
*/
message receiveMessage (int dataPath[])
{
	int i;
	int strobeHandle;
	int tmp;
	int * fileHandles;
	message msg;

	msg.data = 0;
	strobeHandle = openGPIO(Strobe, GPIO_DIRECTION_OUT);

	writeGPIO(strobeHandle, 0);


	fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_IN, DATA_PATH_SIZE);
	usleep(100*WAIT_MS);

	printf("\033[s\033[14;0H");
	//set each bit in the IO to the pin that corresponds to its position
	for (i = 0; i < DATA_PATH_SIZE; i++)
	{
		tmp = readGPIO(fileHandles[i]);
		tmp &= 0x1;
		msg.data |= tmp << (DATA_PATH_SIZE - (DATA_PATH_SIZE - i));
		close(fileHandles[i]);
		
	}
	writeGPIO(strobeHandle,1);
	usleep(100*WAIT_MS);
	close(strobeHandle);

	return msg;
}