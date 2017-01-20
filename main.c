/*
 * main.c
 *
 *  Created on: Feb 7, 2016
 *      Author: mbodis
 *
 * timer source: https://sites.google.com/site/qeewiki/books/avr-guide/timer-on-the-atmega8
 * external timer ds1307: http://davidegironi.blogspot.sk/2013/12/a-ds1307-library-for-avr-atmega.html#.Vw6BbCYvDmE
 *
 *  SETUP - 7segment LED DISPLAY 4 digits with dot:
 *
 *   1 2 3 4 5 6
 *   | | | | | |
 *  -------------
 *  |D1|D2|D3|D4|
 *  -------------
 *   | | | | | |
 *   7 8 9101112
 *
 *	pin 2  - a (digit top segment)
 *  pin 6  - b (digit top-right segment)
 *  pin 10 - c (digit bottom-right segment)
 *  pin 8  - d (digit bottom segment)
 *  pin 7  - e (digit bottom-left segment)
 *  pin 3  - f (digit top-left segment)
 *  pin 11 - g (digit middle segment)
 *  pin 9  - h (dot)
 *
 *  pin 1 -  Digit 1 GRD
 *  pin 4 -  Digit 2 GRD
 *  pin 5 -  Digit 3 GRD
 *  pin 12 - Digit 4 GRD
 *
 *
 * SETUP - ATMEGA8l MCU :
 *
 *  PORT A
 *  	bit 0 - a (digit top segment)
 *  	bit 1 - f (digit top-left segment)
 *  	bit 2 - b (digit top-right segment)
 *  	bit 3 - e (digit bottom-left segment)
 *  	bit 4 - d (digit bottom segment)
 *  	bit 5 - h (dot)
 *  	bit 6 - c (digit bottom-right segment)
 *  	bit 7 - g (digit middle segment)
 *
 *  PORT B
 *  	bit 0 - led circle (10 hr)
 *  	bit 1 - led circle (11 hr)
 *  	bit 2 - led circle (9 hr)
 *  	bit 3 - led circle (8 hr)
 *  	bit 4 - led circle (7 hr)
 *  	bit 5 - led circle (6 hr)
 *  	bit 6 - led circle (5 hr)
 *  	bit 7 - led circle (12 hr)
 *  PORT C
 *  	0 bit -> RTC external timer (SCL)
 *  	1 bit -> RTC external timer (SDA)
 *  	6 bit -> ground digit 3 (7 segment led display)
 *  	7 bit -> ground digit 1 (7 segment led display)
 *  PORT D
 *  	bit 0 - led circle (1 hr)
 *  	bit 1 - led circle (2 hr)
 *  	bit 2 - led circle (4 hr)
 *  	bit 3 - led circle (3 hr)
 *  	bit 5 - ground for digit 3
 *  	bit 6 - ground for digit 4
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uart/uart.h"
#define UART_BAUD_RATE 2400

#include "ds1307/ds1307.h"

uint8_t hr = 12;
uint8_t min = 34;
uint8_t sec = 0;
uint8_t sec5 = 0;

static int SLEEP_TIME_NUMBERS = 3;

// get time from ds1307
uint8_t year = 0;
uint8_t month = 0;
uint8_t day = 0;
uint8_t hour = 0;
uint8_t minute = 0;
uint8_t second = 0;


void init_my_pins(){
	DDRA = 0b11111111; // set bit 0-7 as output (7segment display)
	DDRB = 0b11111111; // set bit 0-7 as output (red + white leds)
	DDRC = 0b11111111; // set bit 0,1,2,3 as input
	DDRD = 0b11111111; // set bit 5,6 as input other 0,1,2,3 as output (red + white leds)
}

void main(){
	init_my_pins();
	init_my_timer();
	ds1307_init();

	//check set date // TODO for setting time
	ds1307_setdate(15, 12, 24, 00, 00, 00);

	while(1) {
		draw_time();
		draw_clock_led();
	}
}

void init_my_timer(){
	// 951 slower than (1 sec) real time
	// set prescaler
	OCR1A = 951;

	// Mode 4, CTC (clear time on compare) on OCR1A
	TCCR1B |= (1 << WGM12);


	//time mask: Set interrupt on compare match
	TIMSK |= (1 << OCIE1A);


	// set prescaler to 1024 and start the timer
	TCCR1B |= (1 << CS12) | (1 << CS10);


	// (interrupt.h) set external interrupt - enable interrupts
	sei();

}

/*
 * interrupt service routine
 */
ISR (TIMER1_COMPA_vect)
{
	//get date
	ds1307_getdate(&year, &month, &day, &hour, &minute, &second);

	if (hr != hour) hr = hour;
	if (min != minute) min = minute;
	if (sec != second ) sec = second;
	if (sec5 != second/5 ) sec5 = second/5;

}

void draw_clock_led(){
	for(int i=1; i<=((sec5)+1); i++){
		light_on_clock_led(i);
		light_off_clock_led();
	}
}

void light_on_clock_led(int hr){
	switch(hr){
		// time: 1hr
		case 1:
		PORTD |= 0b00000001;
		break;

		// time: 2hr
		case 2:
		PORTD |= 0b000000010;
		break;

		// time: 3hr
		case 3:
		PORTD |= 0b000001000;
		break;

		// time: 4hr
		case 4:
		PORTD |= 0b00000100;
		break;

		// time: 5hr
		case 5:
		PORTB = 0b01000000;
		break;

		// time: 6hr
		case 6:
		PORTB = 0b00100000;
		break;

		// time: 7hr
		case 7:
		PORTB = 0b00010000;
		break;

		// time: 8hr
		case 8:
		PORTB = 0b00001000;
		break;

		// time: 9hr
		case 9:
		PORTB = 0b00000100;
		break;

		// time: 10hr
		case 10:
		PORTB = 0b00000001;
		break;

		// time: 11hr
		case 11:
		PORTB = 0b00000010;
		break;

		// time: 12hr
		case 12:
		PORTB = 0b10000000;
		break;
	}

}

void light_off_clock_led(){
	PORTB = 0b00000000;
	PORTD &= ~(1 << 0);
	PORTD &= ~(1 << 1);
	PORTD &= ~(1 << 2);
	PORTD &= ~(1 << 3);
}

void draw_time(){

	//hr1
	draw_number(1, hr/10, 0);
	sleep(SLEEP_TIME_NUMBERS);
	clear_numbers();

	//hr2
	if (sec%2==0 ){
		draw_number(2, hr%10, 1);
	}else{
		draw_number(2, hr%10, 0);
	}
	sleep(SLEEP_TIME_NUMBERS);
	clear_numbers();

	//min1
	draw_number(3, min/10, 0);
	sleep(SLEEP_TIME_NUMBERS);
	clear_numbers();

	//min2
	draw_number(4, min%10, 0);
	sleep(SLEEP_TIME_NUMBERS);
	clear_numbers();
}

void clear_numbers(){
	draw_number(-1, -1, 0);
}

/*
 * digit <1-4> // -1 clear
 * numberToDraw <0-4> // -1 clear
 * dot <0-1>
 */
void draw_number(int digit, int numberToDraw, int dot){

	int newVal = 0b00000000;

	switch(numberToDraw){
	// a,b,c,d,e,f
	case 0:
		newVal = 0b01011111;
		break;
	// b,c
	case 1:
		newVal = 0b01000100;
		break;
	// a,b,d,e,g
	case 2:
		newVal = 0b10011101;
		break;
	// a,b,c,d,g
	case 3:
		newVal = 0b11010101;
		break;
	// b, c, f, g
	case 4:
		newVal = 0b11000110;
		break;
	// a,c,d,f,g
	case 5:
		newVal = 0b11010011;
		break;
	// a,c,d,e,f,g
	case 6:
		newVal = 0b11011011;
		break;
	// a,b,c
	case 7:
		newVal = 0b01000101;
		break;
	// a,b,c,d,e,f,g
	case 8:
		newVal = 0b11011111;
		break;
	// a,b,c,d,f,g
	case 9:
		newVal = 0b11010111;
		break;
	}

	// h
	if (dot == 1){
		newVal |= 0b00100000;
	}
	selectDigit(digit);

	PORTA = newVal;
}

/*
 * select digit: display left to right
 */
void selectDigit(int digit){

	int emptyD = 0b10011111;
	int emptyC = 0b00111111;
		switch (digit){

			case -1:
				DDRD = emptyD;
				DDRC = emptyC;
				break;

			case 1:
				DDRD = emptyD;
				DDRC = 0b10111111;
			break;

			case 2:
				DDRC = emptyC;
				DDRD = 0b10111111;
			break;

			case 3:
				DDRD = emptyD;
				DDRC = 0b01111111;
			break;

			case 4:
				DDRC = emptyC;
				DDRD = 0b11011111;
			break;
		}


}

void sleep(uint8_t millisec){
	while(millisec){
			_delay_ms(1);/* 1 ms delay */
			millisec--;
	}
}
