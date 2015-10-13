/* 
 * File:   main.c
 * Author: sergio
 *
 * Created on September 30, 2015, 8:01 PM
 */
/*
 * File:   PIC and Galileo communication
 *
 *
 * simple PIC program example
 * for UMass Lowell 16.480/552
 *
 * Author: Sergio Coronado
 *         Ian Copithorne
 *         Joseph Braught
 *
 * Created on 2015/9/30
 */

/*
   STROBE   I08_RA2_11
   D0       I07_RC2_8
   D1       I06_RC3_7
   D2       I0_RC4_6
   D3       I08_RC5_5
 */

#include <pic16f688.h>

#define MSG_RESET   0x0;
#define MSG_PING    0x1;
#define MSG_GET     0x2;
#define MSG_ACK     0xE;
#define MSG_NOTHIND 0xF;
#define MSG_LGHTLED 0xD;

void set_receive()
{
    TRISAbits.TRISA2 = 1;
    TRISC = 0b00111100;
    ANSEL = 0b00100000;

}

unsigned char receive_msg()
{
    int counter;
    unsigned char message;

    while(!RA2);
    for ( counter = 0 ; counter < 2; counter ++)
    {
        while(RA2);
        message << 1;
        if (counter == 0) message = 0b00000000;
        message = message || RC5;
        message << 1;
        message = message || RC4;
        message << 1;
        message = message || RC3;
        message << 1;
        message = message || RC2;
        while(!RA2);

    }

    return message;
}


// Main program
void main (void)
{
    unsigned char msg;
    while(1)
    {
        msg=receive_msg();
        switch(msg)
        {
            case MSG_ACK :
            {
                break;
            }
            default:
            {

            }
        }
    }

}


