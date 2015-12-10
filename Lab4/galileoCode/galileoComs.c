#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define Strobe     (26) // IO8
#define GP_4       (28) // IO4
#define GP_5	   (17) // IO5
#define GP_6	   (24) // IO6
#define GP_7	   (27) // IO7
#define GP_I2C 	   (29)
#define GPIO_DIRECTION_IN  	(1)  
#define GPIO_DIRECTION_OUT	(0)
#define ERROR              	(-1)
#define DATA_PATH_SIZE		(4)


typedef struct {

	unsigned char data :DATA_PATH_SIZE;

}message;


int initiateGPIO (int gpio);
int initiateGPIOArray (int  gpio[], int gpioCount);
int openGPIO (int gpio, int direction);
int readGPIO (int fileHandle);
int * openGPIOHandles (int dataPath[], int direction, int data_size);
void messageIO (message * msg, int direction, int fileHandles[], int data_size);
void exit(int status);


const int WAIT_MS = 100;





void sendMessage(message msg, int dataPath[])
{
	int i;
	int strobeHandle;
	int tmp;
	int * fileHandles;


	strobeHandle = openGPIO(Strobe, GPIO_DIRECTION_OUT);
	fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_OUT, DATA_PATH_SIZE);
	printf("\033[s\033[13;0H\033[2K\033[13;2H Status: Sending Command\033[u");
	writeGPIO(strobeHandle, 0);
	usleep(1000*WAIT_MS);
	for (i = 0; i < DATA_PATH_SIZE; i++)
	{
		tmp = 0;
		tmp |= msg.data >> i;
		tmp &= 0x1;
		writeGPIO(fileHandles[i], tmp);
		close(fileHandles[i]);
		
	}
	writeGPIO(strobeHandle,1);
	usleep(1000*WAIT_MS);
	close(strobeHandle);
}

message receiveMessage (int dataPath[])
{
	int i;
	int strobeHandle;
	int tmp;
	int * fileHandles;
	message msg;

	msg.data = 0;
	strobeHandle = openGPIO(Strobe, GPIO_DIRECTION_OUT);
	
	printf("\033[s\033[13;0H\033[2K\033[13;2H Status: Reading Message\033[u");
	writeGPIO(strobeHandle, 0);


	fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_IN, DATA_PATH_SIZE);
	usleep(1000*WAIT_MS);

	printf("\033[s\033[14;0H");
		
	for (i = 0; i < DATA_PATH_SIZE; i++)
	{
		tmp = readGPIO(fileHandles[i]);
		tmp &= 0x1;
		msg.data |= tmp << (DATA_PATH_SIZE - (DATA_PATH_SIZE - i));
		close(fileHandles[i]);
		
	}
	writeGPIO(strobeHandle,1);
	usleep(1000*WAIT_MS);
	close(strobeHandle);

	return msg;
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

void clear ()
{
	while (getchar() != '\n' );
}

int main (void)
{
	char cmd;
	message msgOut;
	message msgIn;
	int i;
	int strobeHandle;
	int adcValue;
	float voltage;
	int dataPath[DATA_PATH_SIZE] = {GP_4, GP_5, GP_6, GP_7};


	printf("\033[2J");
	initiateGPIO(Strobe);
	initiateGPIOArray(dataPath, DATA_PATH_SIZE);
	strobeHandle = openGPIO(Strobe, GPIO_DIRECTION_OUT);
	writeGPIO(strobeHandle,1);
	close(strobeHandle);
	

	while (1)
	{

		printf("\033[4;0H\033[2K\033[4;2H\033[36m Please Enter Command: \033[s");
		printf("\033[6;0H\033[2K\033[6;6H (P)ing Sensor");
		printf("\033[7;0H\033[2K\033[7;6H (L)ight LED");
		printf("\033[8;0H\033[2K\033[8;6H (T)oggle ADC");
		printf("\033[9;0H\033[2K\033[9;6H (R)eset ADC");
		printf("\033[10;0H\033[2K\033[10;6H (G)et Voltage");
		printf("\033[11;0H\033[2K\033[11;6H (Q)uit");
		printf("\033[u");
		cmd = getchar();

		//clear result lines
		for ( i = 12; i < 15; i++)
		{
			printf("\033[%d;0H\033[2K", i);
		}
	

		switch (cmd)
		{
			case 'L':
			{
				
				msgOut.data = 0xD;
				sendMessage(msgOut, dataPath);
				msgIn = receiveMessage(dataPath);

				printf("\033[s\033[12;0H\033[2K\033[12;2H Message Received: %X", msgIn.data);

				if (msgIn.data == 0xE)
				{
					printf("\033[s\033[13;0H\033[2K\033[13;2H[\033[0;32m OK \033[m]\t LED Always On \033[u");
				}
				else if (msgIn.data == 0xC)
				{
					printf("\033[s\033[13;0H\033[2K\033[13;2H[\033[0;32m OK \033[m]\t LED Triggered By Sensor \033[u");
				}
				else
				{
					printf("\033[s\033[13;0H\033[2K\033[13;2H[\033[0;31m Error \033[m]\t Message Acknowledgment Failed \033[u");
				}
				break;

			}
			case 'P' :
			{
				
				msgOut.data = 0x1;
				sendMessage(msgOut, dataPath);
				msgIn = receiveMessage(dataPath);

				printf("\033[s\033[12;0H\033[2K\033[12;2H Message Received: %X", msgIn.data);

				if (msgIn.data == 0xE)
				{
					printf("\033[s\033[13;0H\033[2K\033[13;2H[\033[0;32m OK \033[m]\t Ping Successful \033[u");
				}
				else
				{
					printf("\033[s\033[13;0H\033[2K\033[13;2H[\033[0;31m Error \033[m]\t Message Acknowledgment Failed \033[u");
				}
				break;
			}
			case 'R' :
			{
				
				msgOut.data = 0x0;
				sendMessage(msgOut, dataPath);
				msgIn = receiveMessage(dataPath);

				printf("\033[s\033[12;0H\033[K\033[12;2H Message Received: %X", msgIn.data);

				if (msgIn.data == 0xE)
				{
					printf("\033[s\033[13;0H\033[2K\033[13;2H[\033[0;32m OK \033[m]\t Reset Successful \033[u");
				}
				else
				{
					printf("\033[s\033[13;0H\033[2K\033[13;2H[\033[0;31m Error \033[m]\t Message Acknowledgment Failed \033[u");
				}
				break;
			}
			case 'G' :
			{	

				adcValue = 0;
				voltage = 0;
				msgOut.data = 0x2;
				sendMessage(msgOut, dataPath);

				
				for (i = 2; i >= 0; i--)
				{
					msgIn = receiveMessage(dataPath);
					adcValue |= (msgIn.data << (i * DATA_PATH_SIZE));
					printf("\033[s\033[12;0H\033[k\033[12;2H Last Message Received: %X", msgIn.data);
				}

				msgIn = receiveMessage(dataPath);

				
				if (msgIn.data == 0xE)
				{
					adcValue &= 0x3FF;
					printf("\033[s\033[13;0H\033[2K\033[13;2HADC Value: 0x%X\033[u", adcValue);
					voltage = (float) ((adcValue/1024.0) * 5.0); 
					printf("\033[s\033[14;0H\033[2K\033[14;2HVoltage: %lf \033[u", voltage);
					printf("\033[s\033[15;0H\033[2K\033[15;2H[\033[0;32m OK \033[m]\t ADC Read Successful \033[u");
				}
				else
				{
					printf("\033[s\033[13;0H\033[2K\033[13;2H[\033[0;31m Error \033[m]\t Message Acknowledgment Failed \033[u");
				}

				break;

			}
			case 'T' :
			{	
				msgOut.data = 0x3;
				sendMessage(msgOut, dataPath);
				msgIn = receiveMessage(dataPath);

				printf("\033[s\033[12;0H\033[2K\033[12;2H Message Received: %X", msgIn.data);

				if (msgIn.data == 0xE)
				{
					printf("\033[s\033[13;0H\033[2K\033[13;2H[\033[0;32m OK \033[m]\t ADC Toggled On\033[u");
				}
				else if (msgIn.data == 0xC)
				{
					printf("\033[s\033[13;0H\033[2K\033[13;2H[\033[0;32m OK \033[m]\t ADC Toggled Off\033[u");
				}
				

				break;
			}
			case 'Q':
			{
				unexport(Strobe);
				unexportArray(dataPath, DATA_PATH_SIZE);
				printf("\033[2J\033[0;0H\033[m");
				exit(0);
			}


		}

		clear();
	}

	return (0);
}


