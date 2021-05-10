/*
 * opg1_ny_E20.c
 *
 * Created: 01-09-2020 14:34:13
 * Author : osch
 */ 

#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

#include "I2C.h"  //include library for i2c driver
#include "ssd1306.h" //include display driver
#include <avr/interrupt.h> // for interrupts

// local libs
#include "UART/UART_MODULE.h"
#include "ADC/ADC_MODULE.h"
#include "PWM/PWM_Module.h"

// For USART
#define BAUD 19200 // could also be written as 103 (according to table s.231 i datablad)
#define MYUBRRF F_CPU/8/BAUD-1 // full duplex
#define MYUBRRH F_CPU/16/BAUD-1 // half duplex

// Variables for UART
#define MAX 8
char buffer[MAX] = {0};
volatile char flag_UART = 0;
float ADC_val;

char message[] = "\n Write duty cycle in ii:ii (VALUES MIN:MAX) \n";

// ADC interrupt (når ADC konvertering er færdig)
ISR(ADC_vect)
{
	// return 10 bit sampleværdi
	ADC_val = ADCL+(ADCH<<8); // henter digitale værdi fra ADC i registre ADCL og ADCH 
}

// receive complete interrupt service routine (UART receive interrupt) (når der er sendt fra PC-terminal)
ISR(USART0_RX_vect){

	static int i=0;
	buffer[i++]=UDR0; // gemmer UART data i 'buffer'-variabel

	if(i==MAX-1){
		flag_UART=1;
		i=0;
	}
	
}

// compare match interrupt service routine til timer (timer1 overflow interrupt?)
ISR(TIMER1_COMPA_vect)
{
	ADCSRA|=(1<<ADSC); // ADC conversion every time timers ISR is called
}

void enableReceive_Itr(){
	UCSR0B|=(1<<RXCIE0); // enable receive complete interrupt
}

void init_timer1(){
	TCCR1B |=(1<<WGM12); // timer mode: CTC
	TCCR1B |=(1<<CS11)|((1<<CS10)); // timer pre-scaling: 64
	OCR1A = 249; // udregnet (sætter værdi i sammenligningsregister. via excel dokument 1)
	TIMSK1 |=(1<<OCIE1A); // interrupt mode: Output Compare A Match Interrupt Enable
}

/*format a 3 digits after the comma for a sample of 10 bits*/
/*returns a 10 bit sample from a chosen channel*/
int main(void)
{  
	sei(); // enable global interrupt
	
	// INITIALIZATION
	init_timer1(); // init timer for ADC-conversion
	init_adc(0); // init ADC-registre
	uart0_init(MYUBRRF); // UART0 init
	enableReceive_Itr(); // init interrupt RX interrupt (receive interrupt) for UART
	init_phase_correct(); // enable and configure (registers) phase correct PWM
	
	// START MESSAGE
	putsUSART0(message); // send start message via UART (on PC-terminal)
	
	// For OLED display
   _i2c_address = 0X78; // write address for i2c interface
   I2C_Init();  //initialize i2c interface to display
   InitializeDisplay(); //initialize  display
   print_fonts();  //for test and then exclude the  clear_display(); call
   clear_display();   //use this before writing you own text
   
   // Variables
   int ADC_DUTY_CYCLE;
   int val3;
   char val3a[16];
   char val3b[16];  
   char PWM_val1;
   int duty_val1 = 80;
   int duty_val2 = 20;
   int top_val = 1023; // top val er 1023 fordi ADC_val er 10-bits = 2^10 = 1023
   int OCR_CONVERT1 = 0;
   int OCR_CONVERT2 = 0;
   
   int lim1;
   int lim2;
   
   
  while (1)
  {      
	 //init_adc(0); // init ADC-registre
	 //ADCSRA|=(1<<ADSC);
	 
	 	 val3 = ADC_val;
	 	 
		 // beregner ADC duty cycle med værdi fra ADC (spænding på indgang? ISR)
		 // (top val er 2023 fordi ADC_val er 10-bits = 2^10 = 1023 ... 10-bits ADC!!!)
		 ADC_DUTY_CYCLE = (ADC_val/top_val)*100; // beregner duty cycle ud fra målte ADC spænding 'ADC_val'
	 	 
	 	 // received UART values set OCRNA-limits (duty cycles fra ADC sammenlignes med de grænser sent fra PC-terminal!)
		 if (flag_UART == 1)
		 {
			 lim2 = ((buffer[1]-0x30)+((buffer[0]-0x30)*10));
			 lim1 = ((buffer[4]-0x30)+((buffer[3]-0x30)*10));

//			 TEST OF VALUES ON OLED			 
// 			 sprintf(val3a, "%i", lim1);
// 			 sendStrXY(val3a,0,0); //line 0  -print the line of text
// 			 
// 			 sprintf(val3b, "%i", lim2);
// 			 sendStrXY(val3b,1,1); //line 0  -print the line of text
			 
			 duty_val1 = lim1;
			 duty_val2 = lim2;
			 
			 flag_UART = 0;
		 }
		 
	 	 OCR_CONVERT1 = (duty_val1*256)/100; // ligning fra excel hvor OCR er isoleret
		 OCR_CONVERT2 = (duty_val2*256)/100; // ligning fra excel hvor OCR er isoleret
	 	 if (ADC_DUTY_CYCLE>duty_val1)
	 	 {
	 		 PWM_val1= OCR_CONVERT1;
	 	 }
	     else if (ADC_DUTY_CYCLE<duty_val2)
	 	 {
	 		 PWM_val1= OCR_CONVERT2;
	 	 }
		 else
		 {
			 PWM_val1 = (val3>>2);
		 }
	 
	 // casting float values to integers ... OCR0A val afgøres af indtastede duty_val
	 // pwm rate/styrke opdateres, som kan ses på LED?
	 OCR0A = PWM_val1;
	 
	 
	 sprintf(val3a, "%i", PWM_val1);
	 sprintf(val3b, "%i", ADC_DUTY_CYCLE);
	 //sprintf(val3b, "%i", ADC_val);
 
	 sendStrXY(val3a,0,0); //line 0  -print the line of text
	 sendStrXY(val3b,1,1); //line 0  -print the line of text


  }


}

