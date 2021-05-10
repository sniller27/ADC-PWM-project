/*
 * UART_MODULE.c
 *
 * Created: 18-03-2021 10:20:27
 *  Author: Jonas
 */ 

/**
	ADC INITIALIZATION
**/
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>

#include <avr/interrupt.h> // for interrupts



void init_adc(char channel){
	
	// ADC Control and Status Register A
	ADCSRA|=(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2); // sætter frekvens for interne ADC-clock (skal bruges til opløsning) ... ADC pre-scaler select bits (division factor: 128) ... dvs: (intern clock: 16.000.000/128 = 125.000 Hz) (mellem 100 KHz og 200 KHz) => giver 10 bit opløsning
	ADCSRA|=(1<<ADEN); // enable ADC
	
	// ADC Multiplexer Selection Register
	ADMUX=channel; // alle bits sættes til 0? (se tabel 26-4: input channel selections)
	ADMUX|=(1<<REFS0); // sætter reference spænding: AVCC with external capacitor at AREF pin? s.289 (5V)
	// ADLER bit sættes ved 8-bit (ikke 10-bit)
	
	// Digital Input Disable Registers
	DIDR0=(1<<channel); // sætter kanal 0 til deaktiveret
	DIDR0=~DIDR0; // invterer bit masker med hinanden (inverterer alle bits, så kanal 0 er den eneste der nu er aktiveret)
	DIDR1=0xff; // 0xff = 11111111 (alle kanaler her deaktiveres også...så det bare er kanal 0, der bruges)
	
	// ADC Control and Status Register A
	ADCSRA|=(1<<ADSC); // start ADC-conversion (her start adc'en sin sampling)
	
	// aktiver ADC-interrupt (ISR aktiveres når, der er konverteret?)
	ADCSRA|=(1<<ADIE);

}

void enable_auto_trigger_mode(){
	
	// ADCSRA
	ADCSRA|=(1<<ADATE); // enable auto-trigger mode
	
	// ADCSRB (ADC Auto Trigger Source) (trigger source er sat til: Timer/Counter1 Overflow) (timer overflow s.294)
	ADCSRB|=(1<<ADTS2)|(1<<ADTS1);

}