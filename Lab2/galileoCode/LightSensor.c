#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>


#include "GalileoGPIO.h"
#include "CursorCntl.h"
#include "GalileoComms.h"
#include "WindowConfig.h"

#define bool int
#define true (1)
#define false (0)

bool continueGraphing;
pthread_mutex_t cGmutex;

void printMenu()
{
	int menuLine = MENU_START_Y;

	gotoXY(PROMPT_X,PROMPT_Y);
	setColor(CYAN);
	printf("Please Enter Command: ");
	saveCursor();

	gotoXY(MENU_START_X, menuLine++);
	printf("(P)ing Sensor");

	gotoXY(MENU_START_X, menuLine++);
	printf("(L)ight LED");
	
	gotoXY(MENU_START_X, menuLine++);
	printf("(T)oggle ADC");
	
	gotoXY(MENU_START_X, menuLine++);
	printf("(R)eset ADC");

	gotoXY(MENU_START_X, menuLine++);
	printf("(G)et Voltage");

	gotoXY(MENU_START_X, menuLine++);
	printf("(D)isplay Graph");

	gotoXY(MENU_START_X, menuLine++);
	printf("(Q)uit");

	recallCursor();
}

void clearInputBuffer()
{
	while (getchar() != '\n');
}

float getVoltage(int dataPath[])
{
	int i;
	int adcValue = 0;
	float voltage = 0;
	message msgOut;
	message msgIn;

	msgOut.data = 0x2;

	sendMessage(msgOut, dataPath);


	for (i = 2; i >= 0; i--)
	{
		msgIn = receiveMessage(dataPath);
		adcValue |= (msgIn.data << (i * DATA_PATH_SIZE));
	}

	msgIn = receiveMessage(dataPath);

	if (msgIn.data == 0xE)
	{
		adcValue &= 0x3FF;
		voltage = (float) ((adcValue/1024.0) * 5.0); 

	}

	else
	{
		gotoXY(STATUS_X,STATUS_Y);
		clearLine(STATUS_Y);
		setColor(RESET);
		printf("[\033[0;31m Error \033[m]\t Message Acknowledgment Failed \033[u");
		return (-1);
	}

	gotoXY((GRAPH_END_X - 20),(GRAPH_END_Y + 2) );
	printf("Voltage: %f", voltage);

	return (voltage);

}

void * graphVoltage(void * dataPath)
{
	int i;
	int x = GRAPH_START_X;
	int markerValue = REF_V;
	int segmentBinSize = (GRAPH_END_Y - GRAPH_START_Y) / REF_V;
	float voltage;
	int marginOffset;
	float segmentSize = (REF_V * 1.0) / (GRAPH_END_Y - GRAPH_START_Y);
	float marker;
	float min;
	float max;

	bool continueGraph;

	clearPAGE();

	gotoXY(10,1);
	printf("Segement SIze = %f", segmentSize);

	for (i = GRAPH_START_Y ; i <= GRAPH_END_Y; i++)
	{
		gotoXY(x,i);
		if (((i - GRAPH_START_Y) % segmentBinSize) == 0)
		{
			printf("%d-|", markerValue--);
		}
		else 
		{
			printf(" -|");
		}

	}



	marginOffset = 4;

	for (i = GRAPH_START_X + marginOffset; i <= GRAPH_END_X; i++)
	{
		
		printf("_");

	}

	gotoXY(GRAPH_START_X, GRAPH_END_Y+2);
	printf("Press Any Key to Exit");

	x = GRAPH_START_X+marginOffset;

	do 
	{
			pthread_mutex_lock (&cGmutex);
			continueGraph = continueGraphing;
			pthread_mutex_unlock (&cGmutex);

			
			if (x > GRAPH_END_X) x = GRAPH_START_X+marginOffset;
			x++;

			voltage = getVoltage(dataPath);

			if (voltage == -1)
			{
				pthread_exit(0);
			}

			marker = REF_V;


			for( i = GRAPH_START_Y; i <= GRAPH_END_Y; i++ )
			{
				
				gotoXY(x, i);

				min = marker - (segmentSize/2);
				max = marker + (segmentSize/2);
			
			 	if ((min <= voltage) && (max > voltage))
			 	{
			 		printf("*");
			 	}
			 	else if ( i == GRAPH_END_Y)
			 	{
			 		printf("_");
			 	}
			 	else
			 	{
			 		printf(" ");
			 	}

				

				marker -= segmentSize;
	
			}

	}while (continueGraph);

	pthread_exit(0);
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
	pthread_t displayThread;
	pthread_mutex_init (&cGmutex, NULL);


	initiateGPIO(Strobe);
	initiateGPIOArray(dataPath, DATA_PATH_SIZE);
	strobeHandle = openGPIO(Strobe, GPIO_DIRECTION_OUT);
	writeGPIO(strobeHandle,1);
	close(strobeHandle);

	clearPAGE();
	

	while (1)
	{
		printMenu();

		cmd = getchar();

		clearInputBuffer();

		clearBelowLine(MSG_Y);

		saveCursor();



		switch (cmd)
		{
			case 'L':
			{
				
				msgOut.data = 0xD;

				sendMessage(msgOut, dataPath);

				msgIn = receiveMessage(dataPath);

				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);

				printf("Message Received: %X", msgIn.data);

				gotoXY(STATUS_X,STATUS_Y);
				clearLine(STATUS_Y);
				setColor(RESET);

				if (msgIn.data == 0xE)
				{
					printf("[\033[0;32m OK \033[m]\t LED Always On");
				}
				else if (msgIn.data == 0xC)
				{
					printf("[\033[0;32m OK \033[m]\t LED Triggered By Sensor");
				}
				else
				{
					printf("[\033[0;31m Error \033[m]\t Message Acknowledgment Failed");
				}
				break;

			}
			case 'P' :
			{
				
				msgOut.data = 0x1;
				sendMessage(msgOut, dataPath);
				msgIn = receiveMessage(dataPath);

				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);

				printf("Message Received: %X", msgIn.data);

				gotoXY(STATUS_X,STATUS_Y);
				clearLine(STATUS_Y);
				setColor(RESET);

				if (msgIn.data == 0xE)
				{
					printf("[\033[0;32m OK \033[m]\t Ping Successful");
				}
				else
				{
					printf("[\033[0;31m Error \033[m]\t Message Acknowledgment Failed");
				}
				break;
			}
			case 'R' :
			{
				
				msgOut.data = 0x0;
				sendMessage(msgOut, dataPath);
				msgIn = receiveMessage(dataPath);

				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);

				printf("Message Received: %X", msgIn.data);

				gotoXY(STATUS_X,STATUS_Y);
				clearLine(STATUS_Y);
				setColor(RESET);

				if (msgIn.data == 0xE)
				{
					printf("[\033[0;32m OK \033[m]\t Reset Successful");
				}
				else
				{
					printf("[\033[0;31m Error \033[m]\t Message Acknowledgment Failed");
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
					gotoXY(MSG_X,MSG_Y);
					clearLine(MSG_Y);
					printf("Message Received: %X", msgIn.data);
				}

				msgIn = receiveMessage(dataPath);

				if (msgIn.data == 0xE)
				{
					adcValue &= 0x3FF;
					voltage = (float) ((adcValue/1024.0) * 5.0); 
					setColor(YELLOW);
					gotoXY(MSG_X,MSG_Y + 1);
					printf("ADC Value: 0x%X", adcValue);
					gotoXY(MSG_X,MSG_Y + 2);
					printf("Voltage: %lf \033[u", voltage);
					gotoXY(STATUS_X,STATUS_Y);
					setColor(RESET);
					printf("[\033[0;32m OK \033[m]\t ADC Read Successful \033[u");
				}
				else
				{
					gotoXY(STATUS_X,STATUS_Y);
					clearLine(STATUS_Y);
					setColor(RESET);
					printf("[\033[0;31m Error \033[m]\t Message Acknowledgment Failed \033[u");
				}

				break;

			}
			case 'T' :
			{	
				msgOut.data = 0x3;
				sendMessage(msgOut, dataPath);
				msgIn = receiveMessage(dataPath);

				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);

				printf("Message Received: %X", msgIn.data);

				gotoXY(STATUS_X,STATUS_Y);
				clearLine(STATUS_Y);
				setColor(RESET);


				if (msgIn.data == 0xE)
				{
					printf("[\033[0;32m OK \033[m]\t ADC Toggled On");
				}
				else if (msgIn.data == 0xC)
				{
					printf("[\033[0;32m OK \033[m]\t ADC Toggled Off");
				}
				

				break;
			}
			case 'D':
			{
				pthread_mutex_lock (&cGmutex);
				continueGraphing = true;
				pthread_mutex_unlock (&cGmutex);

				pthread_create(&displayThread, NULL, graphVoltage, &dataPath);


				getchar();


				pthread_mutex_lock (&cGmutex);
				continueGraphing = false;
				pthread_mutex_unlock (&cGmutex);

				pthread_join(displayThread, NULL);

				clearPAGE();

				break;
			}
			case 'Q':
			{
				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);
				printf("[\033[0;32m OK \033[m]\t ThankYou For UsingLightSensor");
				unexport(Strobe);
				unexportArray(dataPath, DATA_PATH_SIZE);
				printf("\033[2J\033[0;0H\033[m");
				exit(0);
			}


		}
		recallCursor();

	}

	return (0);
}


