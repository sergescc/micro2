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
	value[0] = 0x00;

	received = write(file, value, 1);
	
	if (received != 1)
	{
		printf("Connection Failure");

	}
	else
	{
		received = read(file, value, 7);
		if(received != 7)
		{
			printf("Connection Failure");	

		}
	}
	
	//value[2] &= 0xBF;
	//printf("Time is %02x Days %02x hours %02x minutes %02x seconds\n", value[3], value[2], value[1], value[0]);
}

int setClock(int file, unsigned char wValue[])
{
	int i;
	int sent;

	unsigned char message;
	
	unsigned char out[8];
		
	out[0] = 0x00;
	wValue[0] &= 0xBF;
	wValue[1] &= 0x7F;

	for ( i = 1; i < 8 ; i++)
	{
		out[i] = wValue[i -1];
	}

	sent = write(file , wValue, 8);
	if (sent != 8)
	{
		printf("Connection Failure");
	}

	//printf("Set clock is ending\n");
}

int initI2C()
{
    int fd;
    int r;
    
    char *dev = "/dev/i2c-0";
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
    return fd;
}
