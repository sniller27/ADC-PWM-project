/*
 * PWM_Module.c
 *
 * Created: 4/22/2021 3:14:37 PM
 *  Author: Frederik
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "PWM_Module.h"
#include "../UART/UART_MODULE.h"  //uses receive interrupt
volatile char state;
enum state {init,listen,fastPWM,phaseCorrectPWM,ph_freq_corr,phaseCorrectPWMalt};
//enum primaryColor {red, yellow, blue};


/*generate a pulses on OC0A pin(PB7) 60% duty cycle - fastPWM update OCR0A at top*/
void init_fastPWM()
{
	DDRB |= (1<<DDB7);// configure OC0A pin for output
	TCCR0A =(1<<COM0A1)|(1<<WGM01)|(1<<WGM00);  // 0x83; select fast PWM, non-inverting mode
	TCCR0B= (1<<CS01); // and set clock pre-scaler to 8
	TCNT0= 0; // force TCNT0 to count up from 0
	OCR0A= 154; // set duty cycle to 60%"=(154/256)*100
}

/*phase correct mode updates OCR0A at bottom*/
void init_phase_correct()
{
	TCCR0A|=(1<<COM0A1)|(1<<WGM00);	//Clear OC0A on Compare Match when up-counting. Set OC0A on Compare Match when down-counting
	TCCR0B =(1<<CS01);   //prescalling by 8 ()
	OCR0A =102;  //40 duty cycle re top=255 (OCR0A: ved CTC bruges det til delay og ved PWM noget andet... via excel dokument 2)
	TCNT0= 0;
	DDRB |= (1<<DDB7);// configure OC0A pin for output
}

/*phase correct and frequency correct
* Since the OCRnx Registers are updated at BOTTOM, the length of the rising
and the falling slopes will always be equal*/
void init_ph_frPWM()
{
	DDRB|=(1<<PB5);   //pin 11
	TCCR1A|=(1<<COM1A1);	//Clear OC1A on Compare Match when up-counting. Set OC1A on Compare Match when down-counting
	TCCR1B =(1<<CS11)|(1<<WGM13);   //prescalling by 8
	ICR1= 204;  //top value then OC1A pin can be used   // 8bit top value=204
	OCR1A =102;  //50 duty cycle re top value=204
}

// alternative udgave af phase correct mode init
void init_phase_correct_alt()
{
	DDRG |= (1<<DDG5);// configure OC0B pin for output	pin 4
	TCCR0A|=(1<<COM0B1)|(1<<WGM00);	//Clear OC0A on Compare Match when up-counting. Set OC0A on Compare Match when down-counting
	TCCR0B =(1<<CS02)|(1<<WGM02);   //pre-scalling by 8

	OCR0A =255;  //hold top value and block the oc0A output
	OCR0B =102;  //40 duty cycle compare value  re top=255
}
