/** GalileoGPIUO.c

  Created: 10/20/2015

  Created BY: Ian Copithorne,
  Edited By; Sergio Coronado and Joseph Braught

  Purpose: function definitions for setting up and opening the GPIO ports
*/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "GalileoGPIO.h"

/* Function: intiateGPIO()

  Purpose: Exports the files nececessary to control a gpio port

  Input:
    gpioL ID of the gpio to initiat

  Returns: 
    0 if succesful exits program if not
*/

int initiateGPIO (int gpio)
{

	int fileHandle;
	char buffer[256];
	

	fileHandle = open("/sys/class/gpio/export", O_WRONLY);
	if (ERROR == fileHandle)
	{
		printf("[\033[0;31m Error \033[m]\t Unable to open /sys/class/gpio/export\n");
		sleep(1);
		exit (-1);
	}
	else
	{
		sprintf(buffer, "%d", gpio);
		write(fileHandle, buffer, strlen(buffer));

		close(fileHandle);
	}

	return (0);
}

int initiateGPIOArray (int gpio[], int gpioCount)
{
	int i;
	
	for ( i = 0; i < gpioCount; i++)
	{
		initiateGPIO(gpio[i]);

	} 
	return (0);
}

/* Function: openGPOpens 

  Purpose: gpio port for specified direction

  Input:
    gpio: ID of the gpio to initiate, 
    direction: GPIO_DIRECTION_OUT or GPIO_DIRECTION_IN

  Returns: 
    0 if succesful exits program if not
*/

int openGPIO (int gpio, int direction)
{
	char buffer [256];
	int fileHandle;
	int fileMode;

      //specify gpio direction

  sprintf(buffer, "/sys/class/gpio/gpio%d/direction", gpio);
  fileHandle = open(buffer, O_WRONLY);
  if(ERROR == fileHandle)
  {
         printf("[\033[0;31m Error \033[m]\t Unable to open: %s \n", buffer);
         exit(-1);
  }

  if (direction == GPIO_DIRECTION_OUT)
  {
         // Set out direction
         write(fileHandle, "out", 3);
         fileMode = O_WRONLY;
  }
  else
  {
         // Set in direction
         write(fileHandle, "in", 2);
         fileMode = O_RDONLY;
  }
  close(fileHandle);

  //Set how I/O will be handled
  sprintf(buffer, "/sys/class/gpio/gpio%d/drive",gpio);
  fileHandle = open(buffer, O_WRONLY);
  if(ERROR == fileHandle)
  {
  	printf("[\033[0;31m Error \033[m]\t Unable to open: %s \n", buffer);
      exit(-1);
  }
  if (direction == GPIO_DIRECTION_OUT)
  {
         // Set out direction
         write(fileHandle, "strong", 6);
  }
  else
  {
         // Set in direction
         write(fileHandle, "pulldown", 8);
  }
  close(fileHandle);


//Open GPIO for Read / Write
  sprintf(buffer, "/sys/class/gpio/gpio%d/value", gpio);
  fileHandle = open(buffer, fileMode);
  if(ERROR == fileHandle)
  {
         printf("[\033[0;31m Error \033[m]\t Unable to open: %s \n", buffer);
         exit(-1);
  }

  return(fileHandle);
}

/* Function: readGPIO

  Purpose: reads the value of a GPIO at a certain position

  Input:
    
    fileHandle: of GPIO port to be read

  Returns: 
    0 if value in the file is 0 and 1 if the value is anything else;
*/

int readGPIO (int fileHandle)
{
		char value;
		int val;

        read(fileHandle, &value, 1);

        if('0' == value)
        {
             // Current GPIO status low
               val = 0;
        }
        else
        {
             // Current GPIO status high
               val = 1;
        }
        

        return val;
}

/* Function: writeGPIO

  Purpose: writes a value to a specified gpio

  Input:
    
    fileHandle: of GPIO port to be written to

  Returns: 
    0
*/


int writeGPIO(int fileHandle, int val)
{
        if(val ==  0)
        {
               // Set GPIO low status
               write(fileHandle, "0", 1);
        }
        else
        {
               // Set GPIO high status
               write(fileHandle, "1", 1);
        }
        

        return 0;
}

/* Function: writeGPIO

  Purpose: Opens the files necessary to read an array of GPIO ports

  Input:
    
    dataPath: array of GPIO ports considerd as the data port
    direction: GPIO_DIRECTION_OUT or GPIO_DIRECTION_IN
    data_size: sisze of the data path beign read 

  Returns: 
    filehandle * and array of filehandles to the GPIO ports specified
*/

int * openGPIOHandles (int dataPath[], int direction, int data_size)
{
	int i;
	int * fileHandles =  malloc (sizeof(int)*data_size);


	for (i = 0; i < data_size; i++)
	{
		fileHandles[i] = openGPIO(dataPath[i],direction);
	}

	return fileHandles;
}

/* Function: unexport

  Purpose: Clean up closes down the gpio port specified

  Input:
  
    gpio: Port to be closed 

  Returns: 
    void

*/
void unexport(int gpio)
{
  int fileHandle;
  char buffer[256];

    fileHandle = open("/sys/class/gpio/unexport", O_WRONLY);
    if(ERROR == fileHandle)
    {
      printf("[\033[0;31m Error \033[m]\t Unable to open: unexport \n");
        exit(-1);
    }
    sprintf(buffer, "%d",gpio);
    write(fileHandle, buffer, strlen(buffer));

}


/* Function: unexportArray

  Purpose: cleans up a passed inm array of gpio ports

  Input:
  
    gpios: Array of ports to be closed
    gpioNum: number of ports being closed

  Returns: 
    void

*/
void unexportArray(int gpios[], int gpioNum)
{
  int i;

  for (i = 0; i < gpioNum; i++)
  {
    unexport(gpios[i]);
  }
   
}