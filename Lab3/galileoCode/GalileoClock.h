#ifndef GALILEO_CLOCK
#define GALILEO_CLOCK

int readClock(int file);

int setClock(int file, unsigned char day, unsigned char hour, unsigned char min, unsigned char sec);

#endif
