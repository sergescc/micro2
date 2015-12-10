# MAKEFILE: u
# Created By: Sergio Coronado
# 10/24/15
# Purpose: used to build and compile the sensor app

LINK_TARGET = sensor

OBJS = CursorCntl.o GalileoComms.o GalileoGPIO.o LightSensor.o GalileoClock.o


all: sensor

sensor: $(OBJS)
	gcc -Wall -g -pthread -lcurl $(OBJS) -o sensor

CursorCntl.o: CursorCntl.c CursorCntl.h
	gcc -Wall -c -g CursorCntl.c

GalileoGPIO.o: GalileoGPIO.c GalileoGPIO.h
	gcc -Wall -c -g GalileoGPIO.c 

GalileoComs.o: GalileoComms.c GalileoComms.h GalileoGPIO.h
	gcc -Wall -c -g GalileoComms.c 

LightSensor.o: LightSensor.c  CursorCntl.o GalileoComms.o GalileoGPIO.o
	gcc -Wall -c -g -pthread -lcurl LightSensor.c 

GalileoClock.o: GalileoClock.c GalileoClock.h

clean: 
	rm *.o sensor