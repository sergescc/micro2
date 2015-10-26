LINK_TARGET = sensor

OBJS = CursorCntl.o GalileoComms.o GalileoGPIO.o LightSensor.o


all: sensor

sensor: $(OBJS)
	gcc -Wall -g -pthread $(OBJS) -o sensor

CursorCntl.o: CursorCntl.c CursorCntl.h
	gcc -Wall -c CursorCntl.c

GalileoGPIO.o: GalileoGPIO.c GalileoGPIO.h
	gcc -Wall -c GalileoGPIO.c 

GalileoComs.o: GalileoComms.c GalileoComms.h GalileoGPIO.h
	gcc -Wall -c GalileoComms.c 

LightSensor.o: LightSensor.c  CursorCntl.o GalileoComms.o GalileoGPIO.o
	gcc -Wall -c -pthread LightSensor.c 

clean: 
	rm *.o sensor