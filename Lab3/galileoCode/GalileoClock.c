#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "GalileoClock.h"

void readClock(int file, unsigned char value[])
{
	int i;
	int received;
	unsigned char command[2];

    useconds_t delay = 2000;
	
	for(i=0; i < 7; i++)
	{
		command[0] = 0x00 | i; // address in rtc to read
		command[1]++;
		
		received = write(file, &command, 2);
		usleep(delay);
		
		received = read(file, &value[i], 1);
		if(recieved != 1)
		{
			perror("reading i2c device\n");
		}
		usleep(delay);
			
	}
	value[2] = value[2] & 0xBF;
	//printf("Time is %02x Days %02x hours %02x minutes %02x seconds\n", value[3], value[2], value[1], value[0]);
}

int setClock(int file, unsigned char wValue[])
{
	int sent;

	useconds_t delay = 2000;
	wValue[2] = wValue[2] | 0x60;
	for(i=0; i < 7; i++)
	{
		sent = write(fd, &wValue, 2);
		usleep(delay);
			
	}
}

int initI2C()
{
    int fd;
    int r;
    
    char *dev = "/dev/i2c-1";
    int addr = 0x68;
	
    fd = open(dev, O_RDWR );
    if(fd < 0)
    {
            perror("Opening i2c device node\n");
            return 1;
    }
		
    r = ioctl(fd, I2C_SLAVE, addr);
		
    if(r < 0)
    {
            perror("Selecting i2c device\n");
    }
    return r;
}
