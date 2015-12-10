/** LightSensor.c

	Created: 10/20/2015

	Created BY: Ian Copithorne,
	Edited By; Sergio Coronado and Joseph Braught

	Purpose: Program that interfaces with a light sensor module
	that can measure teh voltage of across a photo resistor

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <curl/curl.h>


#include "GalileoGPIO.h"
#include "GalileoClock.h"
#include "CursorCntl.h"
#include "GalileoComms.h"
#include "WindowConfig.h"

#define bool int
#define true (1)
#define false (0)

//Global Variables used for graphing
bool continueGraphing;		// Exit or Continue to Graph
// Mutecx lock for continueGraphing variable


typedef struct sensorArgs  s_Args;
struct sensorArgs
{
	message * msgOut;
	message * msgIn;
	int * dataPath;
	char * cmd;
	pthread_mutex_t * sensorAvailable;
	pthread_mutex_t * sensorLock;
	pthread_cond_t * requestReady;
	pthread_cond_t * resultReady;
	int clkFile;
	char * timestamp;
	unsigned char * timeArray;

};

typedef struct graphArgs g_Args;
struct graphArgs 
{
	pthread_mutex_t * cGmutex;
	bool * continueGraphing;
	s_Args * sArgs; 
};

typedef struct webArgs w_Args;
struct webArgs 
{
	char * userCmd;
	s_Args * sArgs;
};

/* Function: printMenu()

	Purpose: prints a Menu from where the user can choose 

	Input:
		void

	Returns: void
*/


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
	printf("(S)et Clock");

	gotoXY(MENU_START_X, menuLine++);
	printf("(Q)uit");

	recallCursor();
}

/* Function: clearInputBuffer

	Purpose: clears the input buffer to aviod reading erroneuos commands 

	Input:
		void

	Returns: void
*/

void clearInputBuffer()
{
	while (getchar() != '\n');
}

/* Function: geVoltage

	Purpose: Quiet protocla to get a voltage reading 
	returns value of teh voltage reading   

	Input:
		dataPath: GPIO pins to use as the datapath 

	Returns: 
		float voltage: voltage rad by module
*/

float getVoltage(s_Args * sensor)
{
	int i;
	int adcValue = 0;
	float voltage = 0;

	pthread_mutex_lock(sensor->sensorLock);

	*sensor->cmd = 'G';

	pthread_cond_signal(sensor->requestReady);
	pthread_cond_wait(sensor->resultReady, sensor->sensorAvailable);

	for (i = 2; i >= 0; i--)
	{
		adcValue |= (sensor->msgIn[2-i].data << (i * DATA_PATH_SIZE));
	}

	if (sensor->msgIn[3].data == 0xE)
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

	pthread_mutex_unlock(sensor->sensorAvailable);
	pthread_mutex_unlock(sensor->sensorLock);

	return (voltage);

}

/* Function: graphVoltage

	Purpose:Graphs a continue stream of voltage values  

	Input:
		dataPath: GPIO pins to use as the datapath 

	Returns: void
*/

void * graphVoltage(void * param)
{

	g_Args * g_Args = param;
	pthread_mutex_t * cGmutex = g_Args->cGmutex;
	bool * continueGraphing = g_Args->continueGraphing;
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
		pthread_mutex_lock (cGmutex);
		continueGraph = *continueGraphing;
		pthread_mutex_unlock (cGmutex);

		
		if (x > GRAPH_END_X) x = GRAPH_START_X+marginOffset;
		x++;
		

		 
		voltage = getVoltage(g_Args->sArgs);
		

		gotoXY((GRAPH_END_X - 20),(GRAPH_END_Y + 2) );
		printf("Voltage: %f", voltage);

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

void HTTP_GET(const char* url){
	CURL *curl;
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}
}



void * UpdateWebServer ( void * params )
{

	w_Args * wArgs = params;
	s_Args * sArgs = wArgs->sArgs;

	const char* hostname="localhost";
	const int   port=8000;
	const int   id=14;
	const char* password="oranges";
	const char* name="I+am+A+Banana";
	int  adcval;
	char status;
	char buf[1024];

	char timestamp[TIME_STAMP_LENGTH];

	while (1) 
	{

		pthread_mutex_lock(sArgs->sensorLock);
		status = *(wArgs->userCmd);
		pthread_mutex_unlock(sArgs->sensorLock);

		adcval = getVoltage(sArgs);
		buildTimeStamp(sArgs->timeArray, timestamp);
		
		snprintf(buf, 1024, "http://%s:%d/update?id=%d&password=%s&name=%s&data=%d&status=%c&timestamp=%s",
			hostname,
			port,
			id,
			password,
			name,
			adcval,
			status,
            timestamp);

		HTTP_GET(buf);

		usleep(1000*500);

	}	
}

void * SensorThread ( void * params )
{
	int i;
	s_Args * sensor =  params;

	while (1)
	{
		pthread_cond_wait(sensor->requestReady, sensor->sensorAvailable);

		switch (*sensor->cmd)
		{
			case 'P' :
			{
				sensor->msgOut->data = 0x1;
				sendMessage(*sensor->msgOut, sensor->dataPath);
				sensor->msgIn[0] = receiveMessage(sensor->dataPath);
				break;
			}
			case 'L' :
			{
				sensor->msgOut->data = 0xD;
				sendMessage(*sensor->msgOut, sensor->dataPath);
				sensor->msgIn[0] = receiveMessage(sensor->dataPath);
				break;
			}
			case 'R' :
			{
				sensor->msgOut->data = 0x0;
				sendMessage(*sensor->msgOut, sensor->dataPath);
				sensor->msgIn[0] = receiveMessage(sensor->dataPath);
				break;
			}
			case 'G' :
			{
				sensor->msgOut->data = 0x2;
				sendMessage(*sensor->msgOut, sensor->dataPath);
				for ( i = 0; i < 4; i ++)
				{
					sensor->msgIn[i] = receiveMessage(sensor->dataPath);
				}
				break;
			}
			case 'T' :
			{
				sensor->msgOut->data = 0x3;
				sendMessage(*sensor->msgOut, sensor->dataPath);
				sensor->msgIn[0] = receiveMessage(sensor->dataPath);
				break;
			}

		}

		readClock(sensor->clkFile, sensor->timeArray);

		buildTimeStamp(sensor->timeArray, sensor->timestamp);

		pthread_cond_signal(sensor->resultReady);

		pthread_mutex_unlock(sensor->sensorAvailable);

	}

}

/* Function: main

	Purpose: main program performs UI controls and forwards cmd
	 through function calls  

	Input:
		void 

	Returns: o if clean termnation 
*/
int main (void)
{
	char cmd;
	char sensorCmd;
	message msgOut;
	message msgIn[4];
	char timestamp[TIME_STAMP_LENGTH];

	bool continueGraphing;

	int i;
	int strobeHandle;
	int i2cHandle;
	int adcValue;
	int readTime;

	float voltage;
	int dataPath[DATA_PATH_SIZE] = {GP_4, GP_5, GP_6, GP_7};
	pthread_t displayThread;
	pthread_t sensorThread;
	pthread_t webUpdateThread;

	pthread_cond_t resultReady;
	pthread_cond_t requestReady;
	pthread_mutex_t sensorLock;
	pthread_mutex_t sensorAvailable;
	pthread_mutex_t cGmutex;


	pthread_cond_init  (&resultReady, NULL);
	pthread_cond_init  (&requestReady, NULL);
	pthread_mutex_init (&cGmutex, NULL);
	pthread_mutex_init (&sensorLock, NULL);
	pthread_mutex_init (&sensorAvailable, NULL);

	unsigned char timeArray[CLOCK_VECTOR_SIZE];

	int clockFile;
	clockFile = initI2C();

	s_Args sensorData;
	g_Args graphData;
	w_Args webData;

	sensorData.msgOut = &msgOut;
	sensorData.msgIn = msgIn;
	sensorData.dataPath = dataPath;
	sensorData.cmd = &sensorCmd;
	sensorData.sensorAvailable = &sensorAvailable;
	sensorData.sensorLock = &sensorLock;
	sensorData.requestReady = &requestReady;
	sensorData.resultReady = &resultReady;
	sensorData.timestamp = timestamp;
	sensorData.clkFile = clockFile;
	sensorData.timeArray = timeArray;

	graphData.sArgs = &sensorData;
	graphData.cGmutex = &cGmutex;
	graphData.continueGraphing = &continueGraphing;

	webData.sArgs = &sensorData;
	webData.userCmd = &cmd;

	initiateGPIO(Strobe);
	initiateGPIOArray(dataPath, DATA_PATH_SIZE);
	strobeHandle = openGPIO(Strobe, GPIO_DIRECTION_OUT);
	writeGPIO(strobeHandle,1);
	close(strobeHandle);

	initiateGPIO(GP_I2C);
	i2cHandle = openGPIO(GP_I2C, GPIO_DIRECTION_OUT);

	writeGPIO(i2cHandle, 0);
	close(i2cHandle);

	clearPAGE();

	pthread_create(&sensorThread, NULL, &SensorThread, &sensorData );
	pthread_create(&webUpdateThread, NULL, &UpdateWebServer, &webData);
	
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
				pthread_mutex_lock(&sensorLock);

				sensorCmd = cmd;

				pthread_cond_signal(&requestReady);
				pthread_cond_wait(&resultReady, &sensorAvailable);

				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);

				printf("Message Received: %X", msgIn[0].data);

				gotoXY(STATUS_X,STATUS_Y);
				clearLine(STATUS_Y);
				setColor(RESET);

				if (msgIn[0].data == 0xE)
				{
					printf("[\033[0;32m OK \033[m]\t LED Always On");
				}
				else if (msgIn[0].data == 0xC)
				{
					printf("[\033[0;32m OK \033[m]\t LED Triggered By Sensor");
				}
				else
				{
					printf("[\033[0;31m Error \033[m]\t Message Acknowledgment Failed");
				}
				pthread_mutex_unlock(&sensorAvailable);
				pthread_mutex_unlock(&sensorLock);

				break;

			}
			case 'P' :
			{
				pthread_mutex_lock(&sensorLock);

				sensorCmd = cmd;

				pthread_cond_signal(&requestReady);
				pthread_cond_wait(&resultReady, &sensorAvailable);

				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);

				printf("Message Received: %X", msgIn[0].data);

				gotoXY(STATUS_X,STATUS_Y);
				clearLine(STATUS_Y);
				setColor(RESET);

				if (msgIn[0].data == 0xE)
				{
					printf("[\033[0;32m OK \033[m]\t Ping Successful");
				}
				else
				{
					printf("[\033[0;31m Error \033[m]\t Message Acknowledgment Failed");
				}

				pthread_mutex_unlock(&sensorAvailable);
				pthread_mutex_unlock(&sensorLock);

				break;
			}
			case 'R' :
			{
			
				pthread_mutex_lock(&sensorLock);

				sensorCmd = cmd;

				pthread_cond_signal(&requestReady);
				pthread_cond_wait(&resultReady, &sensorAvailable);

				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);

				printf("Message Received: %X", msgIn[0].data);

				gotoXY(STATUS_X,STATUS_Y);
				clearLine(STATUS_Y);
				setColor(RESET);

				if (msgIn[0].data == 0xE)
				{
					printf("[\033[0;32m OK \033[m]\t Reset Successful");
				}
				else
				{
					printf("[\033[0;31m Error \033[m]\t Message Acknowledgment Failed");
				}

				pthread_mutex_unlock(&sensorAvailable);
				pthread_mutex_unlock(&sensorLock);

				break;
			}
			case 'G' :
			{	
				pthread_mutex_lock(&sensorLock);

				sensorCmd = cmd;

				pthread_cond_signal(&requestReady);
				pthread_cond_wait(&resultReady, &sensorAvailable);

				adcValue = 0;
				voltage = 0;
				
				for (i = 2; i >= 0; i--)
				{
					adcValue |= (msgIn[2-i].data << (i * DATA_PATH_SIZE));
					gotoXY(MSG_X,MSG_Y);
					clearLine(MSG_Y);
					printf("Message Received: %X", msgIn[0].data);
				}


				if (msgIn[0].data == 0xE)
				{
					adcValue &= 0x3FF;
					voltage = (float) ((adcValue/1024.0) * 5.0); 
					setColor(YELLOW);
					gotoXY(MSG_X,MSG_Y + 1);
					printf("ADC Value: 0x%X", adcValue);
					gotoXY(MSG_X,MSG_Y + 2);
					printf("Voltage: %lf \033[u", voltage);
					gotoXY(MSG_X,MSG_Y + 3);
					printf("Date: %02x/%02x/%02x Time: %02x:%02x:%02x",timeArray[5], timeArray[4], timeArray[6], timeArray[2], timeArray[1], timeArray[0]);
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

				pthread_mutex_unlock(&sensorAvailable);
				pthread_mutex_unlock(&sensorLock);

				break;

			}
			case 'T' :
			{	
				
				pthread_mutex_lock(&sensorLock);

				sensorCmd = cmd;

				pthread_cond_signal(&requestReady);
				pthread_cond_wait(&resultReady, &sensorAvailable);


				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);

				printf("Message Received: %X", msgIn[0].data);

				gotoXY(STATUS_X,STATUS_Y);
				clearLine(STATUS_Y);
				setColor(RESET);


				if (msgIn[0].data == 0xE)
				{
					printf("[\033[0;32m OK \033[m]\t ADC Toggled On");
				}
				else if (msgIn[0].data == 0xC)
				{
					printf("[\033[0;32m OK \033[m]\t ADC Toggled Off");
				}
				
				pthread_mutex_unlock(&sensorAvailable);
				pthread_mutex_unlock(&sensorLock);
				break;
			}
			case 'D':
			{
				pthread_mutex_lock (&cGmutex);
				continueGraphing = true;
				pthread_mutex_unlock (&cGmutex);

				pthread_create(&displayThread, NULL, graphVoltage, &graphData);

				getchar();

				pthread_mutex_lock (&cGmutex);
				continueGraphing = false;
				pthread_mutex_unlock (&cGmutex);

				pthread_join(displayThread, NULL);

				clearPAGE();

				break;
			}
			case 'S':
			{
				pthread_mutex_lock(&sensorLock);

				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);
				printf("Enter Year: ");
				scanf("%x", &readTime);
				timeArray[6] = readTime;
			
				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);
				printf("Enter Month: ");
				scanf("%x", &readTime);
				timeArray[5] = readTime;
			
				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);
				printf("Enter Day: ");
				scanf("%x", &readTime);
				timeArray[4] = readTime;

				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);
				printf("Enter Hour: ");
				scanf("%x", &readTime);
				timeArray[2] = readTime;
				
				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);
				printf("Enter Minute: ");
				scanf("%x", &readTime);
				timeArray[1] = readTime;

				gotoXY(MSG_X,MSG_Y);
				clearLine(MSG_Y);
				printf("Enter Second: ");
				scanf("%x", &readTime);
				timeArray[0] = readTime;
				timeArray[3] = 0x01;

				setClock(clockFile, timeArray);

				clearInputBuffer();

				pthread_mutex_unlock(&sensorLock);

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
				pthread_mutex_unlock(&sensorLock);
				exit(0);
			}

		}
		recallCursor();

	}

	return (0);
}


