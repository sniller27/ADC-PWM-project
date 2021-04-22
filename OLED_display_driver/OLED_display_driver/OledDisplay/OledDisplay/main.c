/*
 * opg1_ny_E20.c
 *
 * Created: 01-09-2020 14:34:13
 * Author : osch
 */ 
#include <xc.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

#include "I2C.h"  //include library for i2c driver
#include "ssd1306.h" //include display driver
void init(){
	PORTK|=0xFF;
	DDRG |=0b00100000;  //D4 as output
	
}

int main(void)
{  
	init();
	
  _i2c_address = 0X78; // write address for i2c interface
  
  I2C_Init();  //initialize i2c interface to display
  InitializeDisplay(); //initialize  display
  
   print_fonts();  //for test and then exclude the  clear_display(); call
   char text[]="en tekst string"; //string declared before use it in sendStrXY - 15 chars long incl spaces
   clear_display();   //use this before writing you own text
  while (1)
  {      
	 
	 sendCharXY('a',1,2);  //one char  - X is line number - from 0 -7 and Y number position of the char an the line - 15 chars 
	 
	 sendStrXY(text,0,0); //line 0  -print the line of text
	  
	  }

	
  

}

