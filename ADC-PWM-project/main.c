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
#define MAX 11
char buffer[MAX] = {0};
float val1;
// void init(){
// 	PORTK|=0xFF;
// 	DDRG |=0b00100000;  //D4 as output
// 	
// }

ISR(ADC_vect)
{
	// return 10 bit sampleværdi
	val1 = ADCL+(ADCH<<8);
}


void init_adc(){
	
	// ADC Control and Status Register A
	ADCSRA|=(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2); // pre-scaler selection (128) ... dvs: (intern clock: 16.000.000/128 = 125.000 Hz) (mellem 100 KHz og 200 KHz)
	ADCSRA|=(1<<ADEN); // enable ADC

}

void enable_auto_trigger_mode(){
	
	// ADCSRA
	ADCSRA|=(1<<ADATE); // enable auto-trigger mode
	
	// ADCSRB (hvilken mode? en af de 3 sidste?) (timer overflow s.294)
	ADCSRB|=(1<<ADTS2)|(1<<ADTS1);

}

int to_duty_cycle(int val, int ADC_MAX){
	return (100*val)/ADC_MAX;
}

unsigned char to_duty_cycle2(unsigned char temp){
	return (char)((100*temp)/1024);
}


/*format a 3 digits after the comma for a sample of 10 bits*/
void format_frac(int sample){
// 	voltage=(10*sample/2046); //integer as 5 is multiplyed by 2 then the reference maks also 2*1023
// 	buffer[0]=voltage+0x30;//ascii for the integer
// 	frac=(10*sample)%2046;
// 	buffer[1]='.';
// 	buffer[2]=10*frac/2046+0x30;
// 	frac=(10*frac)%2046;
// 	buffer[3]=(10*frac)/2046+0x30;
// 	frac=(10*frac)%2046;
// 	if(frac>=1023)
// 	buffer[3]++;
// 	if(buffer[3]==(0x30+10)){
// 		buffer[3]=0x30;
// 		buffer[2]++;
// 		if(buffer[2]==(0x30+10)){
// 			buffer[2]=0x30;
// 			buffer[0]++;
// 		}
// 	}
}

/*returns a 10 bit sample from a chosen channel*/
void get_sample(char channel){

	// ADC Multiplexer Selection Register
	ADMUX=channel;
	ADMUX|=(1<<REFS0); // reference spænding: AVCC with external capacitor at AREF pin? s.289 (5V)
	// ADLER bit sættes ved 8-bit (ikke 10-bit)
	
	// Digital Input Disable Registers
	DIDR0=(1<<channel);
	DIDR0=~DIDR0; // invterer bit masker med hinanden
	DIDR1=0xff;
	
	// ADC Control and Status Register A
	ADCSRA|=(1<<ADSC); // start ADC-conversion (her start adc'en sin sampling)
	
// 	// polling (via ADC Interrupt Flag)
// 	while(!(ADCSRA&(1<<ADIF))); 
// 
// 	// return 10 bit sampleværdi
// 	return(ADCL+(ADCH<<8));

}



float calc_OCNA_limit(int duty, int top_val){
	return duty*((float)top_val/(float)100);
}






int main(void)
{  
	// aktiver ADC-interrupt
	ADCSRA|=(1<<ADIE);
	sei();
	
	//ved auto-trigger => konfig reg b ... lyder som om vi skal bruge auto-trigger?
	
	init_adc(); // init ADC-registre
	//get_sample(0);
	uart0_init(MYUBRRF); // UART0 init
	
	init_phase_correct();
	
  _i2c_address = 0X78; // write address for i2c interface
  
  I2C_Init();  //initialize i2c interface to display
  InitializeDisplay(); //initialize  display
  
   print_fonts();  //for test and then exclude the  clear_display(); call
   char text[]="en tekst string"; //string declared before use it in sendStrXY - 15 chars long incl spaces
   clear_display();   //use this before writing you own text
   
   //DDRF=(1<<PF0);  // sæt som input
   
   float val2;
   int val3;
   int val4;
   
   char val3a[16];
   char val3b[16];
   
   unsigned char newval1;
   unsigned char newval2;
   
   int duty_val1 = 0;
   int duty_val2 = 0;
   int top_val = 1023;
   char val_ocrna_string[16];
   int val_ocrna1;
   int val_ocrna2;
   float val_ocrna_f = 0;
   
  while (1)
  {      
	  
	  
	 get_sample(0); // skal den kaldes her?
	 //val1 = get_sample(0); // input fra ADC
	 //val2 = to_duty_cycle(val1,1024); // duty cycle
	 val2 = (val1/1023)*100; // duty cycle
	 
	 // casting float values to integers
	 val3 = val1;
	 val4 = val2;
	 
	 //putsUSART0(buffer);//return the buffer (string sent to terminal)
	 
	 sprintf(val3a, "%i", val3);
	 sprintf(val3b, "%i", val4);
	 
	 //sendCharXY('a',1,2);  //one char  - X is line number - from 0 -7 and Y number position of the char an the line - 15 chars 
	 
	 sendStrXY(val3a,0,0); //line 0  -print the line of text
	 sendStrXY(val3b,1,1); //line 0  -print the line of text
	 
	 
	 // received UART values set OCRNA-limits
	 duty_val1 = 80;
	 duty_val2 = 20;
	 
	 val_ocrna1 = calc_OCNA_limit(duty_val1,top_val); // 818
	 val_ocrna2 = calc_OCNA_limit(duty_val2,top_val); // 204
	 
	 // 	  if (val_ocrna1<adc_val)
	 // 	  {
	 // 		  adc_val = val_ocrna1;
	 // 	  }
	 //
	 // 	  if (val_ocrna2>adc_val)
	 // 	  {
	 // 		  adc_val = val_ocrna2;
	 // 	  }
	 sprintf(val_ocrna_string, "%i", val_ocrna1);
	 sendStrXY(val_ocrna_string,2,2);

  }


}

