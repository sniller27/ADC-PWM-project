/*
 * UART_MODULE.c
 *
 * Created: 18-03-2021 10:20:27
 *  Author: Jonas
 */ 

#include <avr/io.h>
#include "UART_MODULE.h"
#define F_CPU 16000000UL //UL = unsigned long

// For USART
#define BAUD 19200 // baud rates skal v?re ens p? begge enheder
#define MYUBRRF F_CPU/8/BAUD-1 // full duplex
#define MYUBRRH F_CPU/16/BAUD-1 // half duplex

/**
	UART INITIALIZATION
**/
// init UART1
void uart0_init(unsigned int ubrr){
	
	// setting UCRSRn (USART Control and Status Register) (for A, B and C)
	UCSR0A=(1<<U2X0); // Full duplex // enable full duplex (aka. double speed?) (A register)
	UCSR0B|=(1<<RXEN0)|(1<<TXEN0);// enable receive + enable transmit (B-register)
	
	// UCSZn sættes til 011. UCSZ02 er allerede 0, så derfor sættes UCSZ00 og UCSZ01.
	UCSR0C|=(1<<UCSZ00)|(1<<UCSZ01); // set frame format (C-register) (8 bits, no parity, 1 start bit, 1 stop bit)
	
	// setting UBRRn (USART Baud Rate Register) (16 bits where H is 8 bit high and L is 8 bit low) (skal beregnes: se tabel i datablad)
	// datablad: s.207
	UBRR0H =(unsigned char)(ubrr>>8); // skriver de 8 høje bit. (skiftes da der er 16 bit i alt i UBRRn)
	UBRR0L =(unsigned char)ubrr; // skriver de 8 lave bit.
	
}

/**
	UART CHARACTERS
**/
// transmit one byte/character
void putchUSART0(char tx){
	while(!(UCSR0A & (1<<UDRE0))); // UDREn er 0 ved transmission indtil transmit buffer er tom (venter på at uart kan sende ny byte) (sker i UCSRnA registret)
	UDR0 = tx; // put data into buffer and send it (skriver til UDR registret...UART I/O Data Register)
}

// receive one byte/character (skal ikke bruges i UR-projekt => skal ersttes af service routine)
 char getchUSART0(void){
	while(!(UCSR0A & (1<<RXC0))); // RXCn er 0 ved transmission indtil der er modtaget en byte/character (vente på at uart har modtaget en byte/character) (sker i UCSRnA registret)
	return UDR0; // get received data from buffer (modtager data fra UDR registret...UART I/O Data Register)
}

/**
	UART STRINGS
**/

// transmit one string (by calling putsUSART0 continuously until whole string is sent)
void putsUSART0(char *ptr){
	
	while(*ptr){
		putchUSART0(*ptr);
		ptr++;
	}
	
}

// receive one string
char getsUSART0(char *p, int max){
	
	char cx;
	char i = 0;
	
	// 0x0D er carriage return \r (ENTER!) flytter line tilbage til starter...i<max Prevent buffer overrun
	while(((cx = getchUSART0()) != 0x0D) && i<max){
		putchUSART0(cx);
		*p=cx;
		
		p++;
		i++;
		
		if (i==max)
		{
			return i;	
		}else {
			return 0;
		}
	}
	
	*p=0;
	return 0;
	
}