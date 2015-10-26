#ifndef GALILEO_COMMS
#define GALILEO_COMMS


#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#define WAIT_MS (1)
#define DATA_PATH_SIZE		(4)


typedef struct {

	unsigned char data :DATA_PATH_SIZE;

}message;



void sendMessage(message msg, int dataPath[]);

message receiveMessage (int dataPath[]);

#endif