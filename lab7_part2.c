/*	Author: Shiyou Wang
 *  Partner(s) Name: Josiah Lee
 *	Lab Section: 
 *	Assignment: Lab #7  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */



#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.h"

enum _states {INIT, LED1, LED2, LED3, BEFORE_LONG_LIGHT, LONG_LIGHT } state, restoreState;
unsigned char isPressed = 0x00;
unsigned char curStage = 0x00;
unsigned char score = 0x05;

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1; // Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Ticks

void TimerOn(){
	TCCR1B = 0x0B; // Clear timer and set basic unit.    bit3---clear  bit2bit1bit0---time unit options   clock /64  125,000 ticks/s	
	OCR1A = 125;   // Set unit period.           num * ticks/s  = interupt time 
	TIMSK1 = 0x02; // Enable timer	
	
	TCNT1=0;  // Initial counter
	_avr_timer_cntcurr = _avr_timer_M; // TimerISR period in milliseconds

	SREG |= 0x80;  //Enable global interrupts
}

void TimerOff(){	
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR(){
	TimerFlag = 1;
}

// TCNT1 == OCR1
ISR(TIMER1_COMPA_vect){
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0){		
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


void Tick(){
	isPressed = ~PINA & 0x01;

	switch(state){ 		
		case INIT: {
			LCD_Cursor(1);
			LCD_WriteData('5');

			PORTB = 0x00;
			state = LED1;			
		}
		break;
		
		case LED1:{
			PORTB = 0x01; 

			if(curStage==1){
				curStage==0;
				state = LED2;

				if(isPressed){
					if(score)
						score--;
				}
			}
			else if(isPressed){
				restoreState = LED2;
				curStage = 0;
				state = BEFORE_LONG_LIGHT;
				score++;
			}
		    else
				curStage++;

			state = LED2; 			
		}
		break;
		case LED2:{
			PORTB = 0x02;

			if(curStage==1){
				curStage==0;
				state = LED3;

				if(isPressed){
					if(score)
						score--;
				}

			}
			else if(isPressed){
				restoreState = LED3;
				curStage = 0;
				state = BEFORE_LONG_LIGHT;
				score++;
			}
		    else
				curStage++;			
		}
		break;
		case LED3:{
			PORTB = 0x04;
	
			if(curStage==1){
				curStage==0;
				state = LED1;

				if(isPressed){
					if(score)
						score--;
				}

			}
			else if(isPressed){
				restoreState = LED1;
				curStage = 0;
				state = BEFORE_LONG_LIGHT;
				score++;
			}
		    else
				curStage++;
		}
		break;
		case BEFORE_LONG_LIGHT:{
			if(!isPressed)
				state = LONG_LIGHT;
		}
		break;
		case LONG_LIGHT:{
			
			if(isPressed){
				curStage = 0;
				state = restoreState;
			}

			if(score==9){
				LCD_ClearScreen();
				LCD_Cursor(1);
				LCD_DisplayString(1,"Victory!");	
			}			
		}
		break;
		
		default:
		break;		
	}

	if(score!=9){
		LCD_Cursor(1);
		LCD_WriteData(score + '0');
	}
}

int main(void){
	DDRA = 0x00;PORTA = 0xFF;
	DDRB = 0xFF;PORTB = 0x00;
	TimerSet(50); //    middle = 300/3/2
	TimerOn();
	state = INIT;
	curStage = 0x00;
	while(1) {
		Tick();
		while (!TimerFlag);
		TimerFlag = 0;
	}
}

