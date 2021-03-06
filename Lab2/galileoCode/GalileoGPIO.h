/** GalileoGPIO.h

	Created: 10/20/2015

	Created BY: Ian Copithorne,
	Edited By; Sergio Coronado and Joseph Braught

	Purpose: function prototypes for sending and receiving messages
	and structure definition for messaging

*/
#ifndef GALILEO_GPIO
#define GALILEO_GPIO
// Definition fo GPIO ports 
#define Strobe     (26) // IO8
#define GP_4       (28) // IO4
#define GP_5     (17) // IO5
#define GP_6     (24) // IO6
#define GP_7     (27) // IO7

//Definition of Direction

#define GPIO_DIRECTION_IN   (1)  
#define GPIO_DIRECTION_OUT  (0)
#define ERROR               (-1)

int initiateGPIO (int gpio);

int initiateGPIOArray (int gpio[], int gpioCount);

int openGPIO (int gpio, int direction);

int readGPIO (int fileHandle);

int writeGPIO(int fileHandle, int val);

int * openGPIOHandles (int dataPath[], int direction, int data_size);

void unexport(int gpio);

void unexportArray(int gpios[], int gpioNum);

#endif