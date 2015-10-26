#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "GalileoGPIO.h"

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

int openGPIO (int gpio, int direction)
{
	char buffer [256];
	int fileHandle;
	int fileMode;

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

void unexport(int gpio)
{
  int fileHandle;
  char buffer[256];

    fileHandle = open("/sys/class/gpio/unexport", O_WRONLY);
    if(ERROR == fileHandle)
    {
      printf("[\033[0;31m Error \033[m]\t Unable to open: %s \n", buffer);
        exit(-1);
    }
    sprintf(buffer, "%d",gpio);
    write(fileHandle, buffer, strlen(buffer));

}

void unexportArray(int gpios[], int gpioNum)
{
  int i;

  for (i = 0; i < gpioNum; i++)
  {
    unexport(gpios[i]);
  }
   
}