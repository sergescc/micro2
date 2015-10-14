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
   D0       I04_RC2_8
   D1       I05_RC3_7
   D2       I06_RC4_6
   D3       I07_RC5_5
 */

#include <pic16f688.h>

unsigned char always_on;


enum messages {
    MSG_RESET   = 0x0,
    MSG_PING    = 0x1,
    MSG_GET     = 0x2,
    MSG_ACK     = 0xE,
    MSG_NOTHING = 0xF,
    MSG_LIGHTLED = 0xD
};

typedef struct
{
    unsigned char data :4;
    
}message;

void set_receive()
{
    TRISAbits.TRISA2 = 1;
    TRISC = 0b00111110;
}

void adc_init ()
{
    ANSEL   = 0b00100000;
    ADCON0  = 0b10010101;
    INTCON  = 0b11000000;
    PIE1    = 0b01000000;
    
}

void set_send()
{
    TRISC = 0b00000010;
}

void set_strobe_input()
{
    TRISAbits.TRISA2 = 1;
}

message receive_msg()
{
    message msg;
    msg.data = 0x0;
    while(RA2);
    set_receive();
    while(!RA2);
    msg.data |= PORTC >> 2;
    while(RA2);
    return msg;
}

void send_message( message msg)
{
    //load message value
    set_send();  
    PORTC &= 0b11000011;
    PORTC |= msg.data << 2;
    //wait for galileo to start reading
    while(!RA2);
    //wait for galileo to stop reading 
    while(RA2);
    set_receive();
}


void interrupt isr(void)
{
    if (ADIF)
    {
        ADIF = 0;
        
        unsigned char darkness;
        darkness = ADRESH;
        if (darkness > 2 )
        {
            RC0 = 1;
        }
        else if (always_on != 0xFF)
        {
            RC0 = 0;
        }
        GO =1;
    }
}

// Main program
void main (void)
{

    message msg;
    set_strobe_input;
    adc_init;
    
    while(1)
    {
        msg=receive_msg();
        switch(msg.data)
        {
            case MSG_GET :
            {
                msg.data = ADRESH;
                msg.data &= 0x2;
                send_message(msg);
                msg.data = ADRESL >> 4;
                send_message(msg);
                msg.data = ADRESL;
                send_message(msg);
                msg.data = MSG_ACK;
                send_message(msg);
                break;
            }
            case MSG_PING :
            default :
            {
                msg.data = MSG_ACK;
                send_message(msg);
                break;
            }
            case MSG_RESET :
            {
                break;
            }
            case MSG_LIGHTLED :
            {
                always_on ^= 0xFF;
                break;
            }
        }
    }

}


