  
/*	Author: josiahlee
 *  Partner(s) Name: Shiyou Wang
 *	Lab Section:
 *	Assignment: Lab #7  Exercise #1
 *	Exercise Description: 
 *  Buttons are connected to ​PA0 and ​PA1​. Output ​num and ​PORTD drive the LCD display, 
 *  initially displaying 0. Pressing ​PA0 increments the display (stopping at 9). 
 *  Pressing ​PA1 decrements the display (stopping at 0). If both buttons are depressed 
 *  (even if not initially simultaneously), the display resets to 0. ​If a button is held, 
 *  then the display continues to increment (or decrement) at a rate of once per second. 
 *  Use a synchronous state machine captured in C.
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/io.c>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

enum States { start, none_pressed, A0_pressed, A1_pressed, both_pressed } state;

void TimerOn() {
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0){
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

unsigned char timerCnt;
unsigned char num;

void tick() {
	unsigned char a = (~PINA & 0x03);
	switch ( state ){
		case start: 
			state = none_pressed; 
			num = 7;
			timerCnt = 0;
			break;
		case none_pressed: 	
			timerCnt = 0;
			if ( a == 0x01 ) {
				state = A0_pressed;
				if ( num < 9 )
					num = num + 1;
			}
			else if (a == 0x02) {
				state = A1_pressed;
				if ( num > 0 )
					num = num - 1;
			}
			else if (a == 0x03) {
				state = both_pressed;
			}
			LCD_Cursor(1);
			LCD_WriteData( num + '0');
			break;
		case A0_pressed: 	
			if ( a == 0x00 ) state = none_pressed;
			else if (a == 0x02) state = A1_pressed;
			else if (a == 0x03) state = both_pressed;
			if (timerCnt >= 9){
				num = num + 1;
				timerCnt = 0;
			}else {
				timerCnt++;
			}
			LCD_Cursor(1);
			LCD_WriteData( num + '0');
			break;
		case A1_pressed: 
			if (a == 0x00) state = none_pressed;
			else if ( a == 0x01 ) state = A0_pressed;
			else if (a == 0x03) state = both_pressed;
			if (timerCnt >= 9){
				num = num - 1;
				timerCnt = 0;
			}else {
				timerCnt++;
			}
			LCD_Cursor(1);
			LCD_WriteData( num + '0');
			break;
		case both_pressed: 	
			if (a == 0x00) state = none_pressed;
			else if ( a == 0x01 ) state = A0_pressed;
			else if (a == 0x02) state = A1_pressed;
			break;
		default: state = start; 
			break;
	}
	switch ( state ){
		case none_pressed: 	
			break;
		case A0_pressed: 	
			break;
		case A1_pressed: 
			break;
		case both_pressed: 	
			timerCnt = 0;
			LCD_Cursor(1);
			LCD_WriteData('0');
			break;
		default: state = start; 
			break;
	}
}

void main(void) {
    
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00; // data
    DDRD = 0xFF; PORTD = 0x00; // control 

    LCD_init();
    TimerSet(100);
	TimerOn();
    state = start;
    cnt = 0;
    num = 7;

    while (1) {
    	tick();
    	while(!TimerFlag);
    	TimerFlag = 0;
    }
}