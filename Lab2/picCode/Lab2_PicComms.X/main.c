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
#include <htc.h> 
#include <xc.h>
#include <stdlib.h>

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

// CONFIG
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select bit (MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Detect (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)





#define bool int
#define true 1
#define false 0

bool always_on;
unsigned int voltage;

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

void set_send()
{
    TRISC &= 0b11000011;             //Set DATA PORT to output
    return;
}

void set_receive()
{
    TRISC |= 0b00111100;
    return;
}

void set_strobe_input()
{
    TRISA = 0b00000100;
    ANSEL = 0;
    TRISC = 0b00111110;
    set_receive();
    return;
}

void adc_init ()
{   
    
    always_on = false;
    
    // Configure Inputs
    ANSELbits.ANS5 = 1;
    
    //Configure ADC
    ADCON0  = 0b10010101;
    ADCON1  = 0b00010000;
    
    //COnfigure Interrupts with adc
    OPTION_REG = 0b00000111;        // Set Timer 0 to ~60Hz
    INTCONbits.T0IE =1;             //Enable Timer 0 Interrupt
    INTCONbits.PEIE = 1;            //Enable Peripherals Interrupts
    PIE1bits.ADIE = 1;              //Enable ADC Interrupt
    INTCONbits.T0IF = 0;            //Set Timer interrupt flag to 0
    PIR1bits.ADIF = 0;              //Set ADC Interrupt flag to 0;
    INTCONbits.GIE = 1;             //Enable Global Interrupts;

    
}




void interrupt_on()
{
    INTCONbits.GIE = 1;
    return;
}

void interrupt_off()
{
    INTCONbits.GIE = 0;
    return;
}

message receive_msg()
{
    message msg;
    msg.data = 0x0;
    while(!RA2) continue;
    while(RA2) continue;
    interrupt_off();
    set_receive();
    while(!RA2) continue;
    msg.data |= PORTC >> 2;
    while(RA2) continue ;
    return msg;
}

void send_message( message msg)
{
    while(RA2) continue;
    //load message value
    set_send();
    PORTC &= 0b11000011;
    PORTC |= msg.data << 2;
    //wait for galileo to start reading
    while(!RA2) continue;
    //wait for galileo to read message 
    
    
    return;
}

void interrupt isr(void)
{
    
    unsigned i;
    
    if (ADIF)
    {
        ADIF = 0;

        voltage = (ADRESH<<8) + ADRESL;
        
        if (voltage > 400  )
        {
            RC0 = 1;
        }
        else if (!always_on && voltage < 360)
        {
            RC0 = 0;
        }
        else if (always_on)
        {
            RC0 =1;
        }

    }
    
    if (T0IF)
    {
        T0IF = 0;
        //Every .016 seconds launch ADC
        if (ADCON0bits.GO != 1)
        {
            ADCON0bits.GO = 1;

        }
        
    }
    
    
}

// Main program
int main (void)
{
    
    message msg;
    set_strobe_input();
    adc_init();
    always_on = false;
    while(true)
    {
      
        msg = receive_msg();
        switch(msg.data)
        {            
            case MSG_GET :
                msg.data = voltage >> 8;
                msg.data &= 0x2;
                send_message(msg);
                msg.data = voltage >> 4;
                send_message(msg);
                msg.data = voltage;
                send_message(msg);
                msg.data = MSG_ACK;
                send_message(msg);
                break;
            case MSG_LIGHTLED :
                if (always_on)
                {
                    always_on = false;
                }
                else
                {
                    always_on = true;
                }
                break;
            case MSG_RESET :
                break;
            case MSG_PING :
                break;
                

        }
        
        msg.data = MSG_ACK;
        send_message(msg);
        interrupt_on();
    }

    return (EXIT_SUCCESS);
}


