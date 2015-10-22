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
#define GPIO_DIRECTION_IN      	(1)  
#define GPIO_DIRECTION_OUT     	(0)
#define ERROR                  	(-1)
#define DATA_PATH_SIZE			(4)

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
		printf("[\033[0;32m OK \033[m]\t Initiated GPIO: %d \n",  gpio);
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


   //Open GPIO for Read / Write
        sprintf(buffer, "/sys/class/gpio/gpio%d/value", gpio);
        fileHandle = open(buffer, fileMode);
        if(ERROR == fileHandle)
        {
               printf("[\033[0;31m Error \033[m]\t Unable to open: %s \n", buffer);
\
               exit(-1);
        }

        return(fileHandle);
}

int readGPIO (int fileHandle)
{
		int value;

        read(fileHandle, &value, 1);

        if('0' == value)
        {
             // Current GPIO status low
               value = 0;
        }
        else
        {
             // Current GPIO status high
               value = 1;
        }
        

        return value;
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
	int * fileHandles  = malloc(sizeof(int) * data_size);


	for (i = 0; i < data_size; i++)
	{
		fileHandles[i] = openGPIO(dataPath[i],direction);
	}

	return fileHandles;
}



void messageIO (message * msg, int direction, int fileHandles[], int data_size)
{
	int i;
	int tmp;

	
	for (i =0 ; i < data_size; i++)
	{
		if (direction == GPIO_DIRECTION_OUT)
		{

			tmp = msg->data >> i;
			tmp &= 0x1;
			writeGPIO(fileHandles[i], tmp);
			//close(fileHandles[i]);
		}
		else
		{
			
			if (i == 0)
			{
				msg->data = 0x0;
			}

			tmp = readGPIO(fileHandles[i]);
			msg->data |= tmp << (DATA_PATH_SIZE - (DATA_PATH_SIZE - i));
			//close(fileHandles[i]);

		}
	}
	
	


}

int main (void)
{
	char cmd;
	message msg;
	printf("\033[2J");

	int i;
	int dataPath[DATA_PATH_SIZE] = {GP_4, GP_5, GP_6, GP_7};
	int strobeHandle;
	int * fileHandles;
	int adcValue;

	float voltage;

	initiateGPIO(Strobe);
	initiateGPIOArray(dataPath, DATA_PATH_SIZE);

	strobeHandle = openGPIO(Strobe, GPIO_DIRECTION_OUT);
	writeGPIO(strobeHandle,1);
	

	while (1)
	{

		printf("\033[4;0H\033[K\033[4;2H\033[36m Please Enter Command: \033[s");
		printf("\033[6;0H\033[K\033[6;6H (P)ing Sensor");
		printf("\033[7;0H\033[K\033[7;6H (L)ight LED");
		printf("\033[8;0H\033[K\033[8;6H (T)oggle ADC");
		printf("\033[9;0H\033[K\033[9;6H (R)eset ADC");
		printf("\033[10;0H\033[K\033[10;6H (G)et Voltage");
		printf("\033[u");
		cmd = getchar();
	

		switch (cmd)
		{
			case 'L':
			{
				writeGPIO(strobeHandle, 0);

				msg.data = 0xD;
				fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_OUT, DATA_PATH_SIZE);
				messageIO(&msg, GPIO_DIRECTION_OUT, dataPath, DATA_PATH_SIZE);
				printf("\033[s\033[11;0H\033[K\033[10;2H Status: Sending Command");
				usleep(1000 *100);
				writeGPIO(strobeHandle,1);
				usleep(1000 *100);
				writeGPIO(strobeHandle,0);
				fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_IN, DATA_PATH_SIZE);
				usleep(1000*100);
				writeGPIO(strobeHandle,1);
				printf("\033[11;0H\033[K\033[4B Status: Reading Message");
				messageIO(&msg, GPIO_DIRECTION_IN, fileHandles, DATA_PATH_SIZE);
				printf("\033[12;0H\033[K\033[4B Message: 0x%X ",  msg.data);
				if (msg.data == 0xE)
				{
					printf("\033[11;0H\033[K\033[4B Status: LED Toggled ON\033[u");
				}
				else if (msg.data == 0xC)
				{
					printf("\033[11;0H\033[K\033[4B Status: LED Toggled OFF\033[u");
				}
				break;

			}
			case 'P' :
			{
				msg.data = 0x1;
				fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_OUT, DATA_PATH_SIZE);
				messageIO(&msg, GPIO_DIRECTION_OUT, dataPath, DATA_PATH_SIZE);
				printf("\033[s\033[11;0H\033[K\033[10;2H Status: Sending Command");
				usleep(1000 *100);
				writeGPIO(strobeHandle,1);
				usleep(1000 *100);
				writeGPIO(strobeHandle,0);
				fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_IN, DATA_PATH_SIZE);
				usleep(1000*100);
				writeGPIO(strobeHandle,1);
				printf("\033[11;0H\033[2\033[4B Status: Reading Message");
				messageIO(&msg, GPIO_DIRECTION_IN, fileHandles, DATA_PATH_SIZE);
				printf("\033[12;0H\033[K\033[4B Message: 0x%X ",  msg.data);
				if (msg.data == 0xE)
				{
					printf("\033[11;0H\033[K\033[4B Status: Ping Successful\033[u");
				}
				break;
			}
			case 'R' :
			{
				msg.data = 0x0;
				fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_OUT, DATA_PATH_SIZE);
				messageIO(&msg, GPIO_DIRECTION_OUT, dataPath, DATA_PATH_SIZE);
				printf("\033[s\033[11;0H\033[K\033[10;2H Status: Sending Command");
				usleep(1000 *100);
				writeGPIO(strobeHandle,1);
				usleep(1000 *100);
				writeGPIO(strobeHandle,0);
				fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_IN, DATA_PATH_SIZE);
				usleep(1000*100);
				writeGPIO(strobeHandle,1);
				printf("\033[11;0H\033[K\033[4B Status: Reading Message");
				messageIO(&msg, GPIO_DIRECTION_IN, fileHandles, DATA_PATH_SIZE);
				printf("\033[12;0H\033[K\033[4B Message: 0x%X ",  msg.data);
				if (msg.data == 0xE)
				{
					printf("\033[11;0H\033[K\033[4B Status: Reset Successful ADC Reset\033[u");
				}
				break;
			}
			case 'G' :
			{
				adcValue = 0;
				voltage = 0;
				msg.data = 0x2;
				fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_OUT, DATA_PATH_SIZE);
				messageIO(&msg, GPIO_DIRECTION_OUT, dataPath, DATA_PATH_SIZE);
				printf("\033[s\033[11;0H\033[K\033[10;2H Status: Sending Command");
				usleep(1000 *100);
				writeGPIO(strobeHandle,1);
				usleep(1000 *100);
				for (i = 0; i < 3; i++)
				{
					writeGPIO(strobeHandle,0);
					fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_IN, DATA_PATH_SIZE);
					usleep(1000*100);
					writeGPIO(strobeHandle,1);
					printf("\033[11;0H\033[K\033[4B Status: Reading Message");
					messageIO(&msg, GPIO_DIRECTION_IN, fileHandles, DATA_PATH_SIZE);
					printf("\033[12;0H\033[K\033[4B Message: 0x%X ",  msg.data);
					adcValue |= msg.data << (i * DATA_PATH_SIZE);
					usleep(1000*100);
				}
				writeGPIO(strobeHandle,0);
				fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_IN, DATA_PATH_SIZE);
				usleep(1000*100);
				writeGPIO(strobeHandle,1);
				printf("\033[11;0H\033[K\033[4B Status: Reading Message");
				messageIO(&msg, GPIO_DIRECTION_IN, fileHandles, DATA_PATH_SIZE);
				printf("\033[12;0H\033[K\033[4B Message: 0x%X ",  msg.data);
				if (msg.data == 0xE)
				{
					printf("\033[11;0H\033[K\033[4B Status: ADC Read Succesful\033[u");
				}
				printf("\033[14;0HADC Value: %X", adcValue);
				adcValue &= 0x3FF;
				voltage = (float) ((adcValue/1024) * 5.0); 
				printf("\033[12;0H\033[K\033[4B Voltage: %lf \033[u", voltage);
				break;

			}
			case 'T' :
			{
				msg.data = 0x3;
				fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_OUT, DATA_PATH_SIZE);
				messageIO(&msg, GPIO_DIRECTION_OUT, dataPath, DATA_PATH_SIZE);
				printf("\033[s\033[11;0H\033[K\033[10;2H Status: Sending Command");
				usleep(1000 *100);
				writeGPIO(strobeHandle,1);
				usleep(1000 *100);
				writeGPIO(strobeHandle,0);
				fileHandles = openGPIOHandles(dataPath, GPIO_DIRECTION_IN, DATA_PATH_SIZE);
				usleep(1000*100);
				writeGPIO(strobeHandle,1);
				printf("\033[11;0H\033[K\033[4B Status: Reading Message");
				messageIO(&msg, GPIO_DIRECTION_IN, fileHandles, DATA_PATH_SIZE);
				printf("\033[12;0H\033[K\033[4B Message: 0x%X ",  msg.data);
				if (msg.data == 0xE)
				{
					printf("\033[11;0H\033[K\033[4B Status: Reset Successful ADC Toggled Off\033[u");
				}
				else if (msg.data == 0xC)
				{
					printf("\033[11;0H\033[K\033[4B Status: Reset Successful ADC Toggled Off\033[u");
				}

				break;
			}

		}
	}

	return (0);
}


