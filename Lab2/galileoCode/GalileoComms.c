/*
 * File:   PIC and Galileo communication          
 *         
 * 
 * simple Galileo program example
 * for UMass Lowell 16.480/552
 * 
 * Author: Roy
 *
 * Created on 2014/9/13
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#define Strobe     (26) // IO8
#define GP_4       (28) // IO4
#define GP_5	   (17) // IO5
#define GP_6	   (24) // IO6
#define GP_7	   (27) // IO7
#define GPIO_DIRECTION_IN      (1)  
#define GPIO_DIRECTION_OUT     (0)
#define ERROR                  (-1)


typedef struct{
	
	unsigned char data[4];

} message;

int openGPIO(int gpio, int direction );
int writeGPIO(int fHandle, int val);
int readGPIO(int fileHandle, int gpio);


//open GPIO and set the direction
int openGPIO(int gpio, int direction )
{

		
		char buffer[256];
        int fileHandle;
        int fileMode;

  //Export GPIO
        fileHandle = open("/sys/class/gpio/export", O_WRONLY);
        if(ERROR == fileHandle)
        {
               puts("Error: Unable to open /sys/class/gpio/export");
               return(-1);
        }
        sprintf(buffer, "%d", gpio);
        write(fileHandle, buffer, strlen(buffer));
        close(fileHandle);

   //Direction GPIO
        sprintf(buffer, "/sys/class/gpio/gpio%d/direction", gpio);
        fileHandle = open(buffer, O_WRONLY);
        if(ERROR == fileHandle)
        {
               puts("Unable to open file:");
               puts(buffer);
               return(-1);
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


   //Open GPIO for Read / Write
        sprintf(buffer, "/sys/class/gpio/gpio%d/value", gpio);
        fileHandle = open(buffer, fileMode);
        if(ERROR == fileHandle)
        {
               puts("Unable to open file:");
               puts(buffer);
               return(-1);
        }

        return(fileHandle);  //This file handle will be used in read/write and close operations.
}

//write value
int writeGPIO(int fHandle, int val)
{
	printf("WValue: %d \n", val);

    if(val == 0)
	{
		write(fHandle, "0", 1);
	}
	else
	{
		write(fHandle, "1", 1);
	}

	return(0);
}
int openFileForReading (gpio)
{
		char buffer [256];
		int fileHandle;
	    sprintf(buffer, "/sys/class/gpio/gpio%d/value", gpio);
        fileHandle = open(buffer, O_RDONLY);
        if(ERROR == fileHandle)
        {
               puts("Unable to open file:");
               puts(buffer);
               return(-1);
        }

        return(fileHandle);
}
//Read from the GPIO
int readGPIO(int fileHandle, int gpio)
{
	
	int value;
	fileHandle = openFileForReading(gpio);
    read(fileHandle, &value, 1);

		
    if(value  == '0')
    {
        // Current GPIO status low
        value = 0;
    }
    else
    {
        // Current GPIO status high
        value = 1;
    }

    printf("RValue: %d \n", value);
    close(fileHandle);
    return value;

}


void messageIO (int GPIO_DIRECTION, message * msg)
{
	int i;
	int fileHandleGPIO_4;
	int fileHandleGPIO_5;
	int fileHandleGPIO_6;
	int fileHandleGPIO_7;

	fileHandleGPIO_4 = openGPIO(GP_4, GPIO_DIRECTION_OUT);
	fileHandleGPIO_5 = openGPIO(GP_5, GPIO_DIRECTION_OUT);
	fileHandleGPIO_6 = openGPIO(GP_6, GPIO_DIRECTION_OUT);
	fileHandleGPIO_7 = openGPIO(GP_7, GPIO_DIRECTION_OUT);



	if (GPIO_DIRECTION == GPIO_DIRECTION_OUT)
	{

		
		printf("CMD: ");
		for (i = 3 ; i >= 0; i-- )
		{
			printf("%d ", msg->data[i]);
		}
		printf("\n");


		writeGPIO(fileHandleGPIO_4, msg->data[0]);
		writeGPIO(fileHandleGPIO_5, msg->data[1]);
		writeGPIO(fileHandleGPIO_6, msg->data[2]);
		writeGPIO(fileHandleGPIO_7, msg->data[3]);

		printf("\n");

	}
	else
	{
		
		msg->data[0] = readGPIO(fileHandleGPIO_4, GP_4);
		msg->data[1] = readGPIO(fileHandleGPIO_5, GP_5);
		msg->data[2] = readGPIO(fileHandleGPIO_6, GP_6);
		msg->data[3] = readGPIO(fileHandleGPIO_7, GP_7);

		printf("Response: ");
		for (i = 3 ; i >= 0; i--)
		{
			printf("%d ", msg->data[i]);
		}
		printf("\n");
	}


}

void delay( int milliseconds)
{
	long pause;
	clock_t now;
	clock_t then;

	pause = milliseconds*(CLOCKS_PER_SEC/1000);

	now = then = clock();

	printf("Start: %ld \n", now);

	while ( now - then < pause)
	{
		now = clock();
	}

	printf("Stop: %ld \n", now);
}


//main
int main(void)
{

	int fileHandleGPIO_S;
	int i;
	int j;
	int adcValue;
	int response;
	char cmd;
	char tmp;
	double voltage;


	message msg;

	fileHandleGPIO_S = openGPIO(Strobe, GPIO_DIRECTION_OUT);
       
        while(1)
        {
        	for (i = 0; i < 4; i++)
        	{
        		msg.data[i] = 0;
        	}
        	while ( tmp != '\n')
        	{
        		scanf("%c", &tmp);
        	}
			
			response = 0;
			printf("\nCommand (G,P,R): ");
			scanf("%c", &cmd);
			printf("\n");
			if (cmd == 'G')
			{
				printf("\nGetting Voltage Value\n");
				writeGPIO(fileHandleGPIO_S, 0);
				//write command
				msg.data[1] = 1;
				messageIO(GPIO_DIRECTION_OUT, &msg);
			
				writeGPIO(fileHandleGPIO_S, 1);
				//wait for pic to read
				
				//end write operation
				voltage = 0;

				for ( i = 0; i <4 ; i++)
				{

					

					writeGPIO(fileHandleGPIO_S, 0);
					
					delay(1000);

					writeGPIO(fileHandleGPIO_S, 1);

					messageIO(GPIO_DIRECTION_IN, &msg);

					printf("Nibble-%d: ", i);


					if ( i < 3 )
					{


						for ( j = 3 ;  j >= 0; j--)
						{
							printf("%d ", msg.data[j]);
							adcValue |= msg.data[j];
							adcValue <<= 1;
						}
					}
					else
					{
						response = 0;

						for ( j = 3 ;  j >= 0; j--)
						{
							printf("%d ", msg.data[j]);
							response |= msg.data[j];
							response <<= 1;
						}
					}

					printf("\n");

					
						
				}

				if (response != 0xE)
				{
					printf("Acknoledgement Not Received\n");
				}

				voltage = adcValue/1024.0 * 5;

				printf("Voltage: %lf \n", voltage);
			
    		}
			else if(cmd == 'P')
			{
				printf("\nPinging Device");
				writeGPIO(fileHandleGPIO_S, 0);
				//write command
				msg.data[0] = 1;
				messageIO(GPIO_DIRECTION_OUT, &msg);
				
				//end write operation
				writeGPIO(fileHandleGPIO_S, 1);
				//wait for pic to read
				delay(1000);

				writeGPIO(fileHandleGPIO_S, 0);

				delay(1000);

				writeGPIO(fileHandleGPIO_S, 1);

				messageIO(GPIO_DIRECTION_IN, &msg);

				printf("Message: ");

				response = 0;

				for ( i = 3 ; i < 0; i--)
				{
					printf("%d ", msg.data[i]);
					response |= msg.data[i];
					response <<= 1;

				}
				
				if (response == 0xE)		{
					printf("Message Acknowledged.\n");
				}
				
			}
			else if(cmd == 'R')
			{
				
			}
			else
			{
				printf("Invalid Command");
			}
		}
}
