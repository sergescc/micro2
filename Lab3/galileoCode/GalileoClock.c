#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "GalileoClock.h"

void readClock(int file, unsigned char value[])
{
	unsigned char command[2];
        useconds_t delay = 2000;
	
	for(i=0; i < 6; i++)
			{
				command[0] = 0x00 | i; // address in rtc to read
				command[1]++;
				
				file = write(fd, &command, 2);
				usleep(delay);
				
				file = read(fd, &value[i], 1);
				if(file != 1)
				{
					perror("reading i2c device\n");
				}
				usleep(delay);
			
			}
			//printf("Time is %02x Days %02x hours %02x minutes %02x seconds\n", value[3], value[2], value[1], value[0]);
}

int setClock(int file, unsigned char day, unsigned char hour, unsigned char min, unsigned char sec)
{
	unsigned char wValue[2];
	wValue[0] = sec;
	wValue[1] = min;
	wValue[2] = hour;
	wValue[3] = day;
	useconds_t delay = 2000;
	for(i=0; i < 4; i++)
	{
		file = write(fd, &wValue, 2);
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
