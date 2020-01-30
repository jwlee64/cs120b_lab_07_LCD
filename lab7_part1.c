/*	Author: Shiyou Wang swang324
 *  Partner(s) Name: Josiah Lee
 *	Lab Section: 
 *	Assignment: Lab #7  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */


//#include <avr/io.h>
//#include <avr/interrupt.h>
#include "io.c"
#include "io.h"

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


enum _states {INIT, NO_PRESS, PRESS_INC, PRESS_DEC, PRESS_RESET} state;
unsigned char counter = 0x00;
unsigned char btns = 0x00;

void Tick(){
	btns = ~PINA & 0x03;
		   
	switch(state){ 
		case INIT: 
			state = NO_PRESS;
		
			LCD_Cursor(1);
			LCD_WriteData('0');
			
			break;		
		
		case NO_PRESS:
			if(btns == 0x01){
				state = PRESS_INC;
				
				if(counter >= 0x09)	{
					counter = 0x09;
				}
				else{
					counter = counter + 0x01;
				}
			}			
			else if(btns == 0x02){
				state = PRESS_DEC;
				
				if(counter <= 0x00)	{
					counter = 0x00;
				}
				else{
					counter = counter - 0x01;
				}
			}
			else if(btns == 0x03){				
				state = PRESS_RESET;
				counter = 0;
			}
			else
				state = NO_PRESS;
			
			LCD_Cursor(1);
			LCD_WriteData(counter + '0');							
							
			break;
		
		case PRESS_INC:
			if( btns == 0x00)
				state = NO_PRESS;
			else if(btns == 0x02)
				state = PRESS_DEC;
			else if(btns== 0x03)
				state = PRESS_RESET;								
						
		break;
		
		case PRESS_DEC:
			if( btns == 0x00)
				state = NO_PRESS;
			else if(btns == 0x01)
				state = PRESS_INC;
			else if(btns== 0x03)
				state = PRESS_RESET;	
			
		break;
		
		case PRESS_RESET:
			if( btns == 0x00)
				state = NO_PRESS;
			else if(btns == 0x01)
				state = PRESS_INC;
			else if(btns == 0x02)
				state = PRESS_DEC;	
			
			counter  = 0;
			LCD_Cursor(1);
			LCD_WriteData(counter + '0');
									
			break;		
		
		default:
		break;
	}
	
}

int main(void){
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	
	LCD_init();
	TimerSet(1000);
	TimerOn();
	state = INIT;
	while(1) {
		Tick();
		while (!TimerFlag);
		TimerFlag = 0;
	}
}
