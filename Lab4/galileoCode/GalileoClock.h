#ifndef GALILEO_CLOCK
#define GALILEO_CLOCK

#define CLOCK_VECTOR_SIZE (7)

#define TIME_STAMP_LENGTH  (20)

void readClock(int file, unsigned char value[]);

int setClock(int file, unsigned char value[]);

int initI2C();

void buildTimeStamp ( unsigned char timeArray[], char timeStamp[]);

#endif
