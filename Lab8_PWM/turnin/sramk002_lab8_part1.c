/*  Author: Sidharth Ramkumar (sramk002@ucr.edu)
 *  Partner(s) Name: none 
 *	Lab Section: 022
 *	Assignment: Lab #8  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Video - https://youtu.be/cfm4tQc_ddU
 */

#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#define b1 (~PINA & 0x01)
#define b2 (~PINA & 0x02)
#define b3 (~PINA & 0x04)

enum Note_States { SMStart, Wait, C_4, D_4, E_4 } notes; 

//0.954 hz is the lowest frequency possible with this function 
//based on settings in PWM_on()
//Passing in 0 as teh frequency will stop the speaker from generating sound 
void set_PWM(double frequency){
	static double current_frequency; //Keeps track of the currently set frequency 
	//Will only update the registers when the frequency changes, otherwise allows music to play uninterrupted.
	if (frequency != current_frequency){
		if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter 
		else {TCCR3B |= 0x03; } //resumes/continues timer/counter

		//prevents OCR3A from overflowing, usign prescaler 64
		//0.954 is the smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR3A = 0xFFFF; }

		//prevents OCR3A from underflowing, using prescaler 64 
		//31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR3A = 0x0000; }
		
		//set OCR3A based on desired frequency
		else { OCR3A = (short) (8000000 / (128 * frequency)) - 1; }

		TCNT3 = 0; //resets counter 
		current_frequency = frequency; //Updates the current frequency 
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
		// COM3A0: Toggle PB3 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
		// WGM32: When counter (TCNT3) matches OCR3A, reset counter
		// CS31 & CS30: Set a prescaler of 64 
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

void Tick_Fct(){
	switch(notes){
	case SMStart:
	   notes = Wait;
	break;

	case Wait:
	   if ((b1 && !b2) && !b3){
	   	notes = C_4;
	   } else if ((!b1 && b2) && !b3){
		notes = D_4;	
	   } else if ((!b1 && !b2) && b3){
	   	notes = E_4;
	   } else {
		notes = Wait;
	   }	
	break;

	case C_4:
	   if ((b1 && !b2) && !b3){
		notes = C_4;	
	   } else {
	   	notes = Wait;
 	   }
	break;

	case D_4:
	   if ((!b1 && b2) && !b3){
		notes = D_4;
	   } else {
		notes = Wait;
	   }
	break;

	case E_4:
	   if ((!b1 && !b2) && b3){
		notes = E_4;
	   } else {
		notes = Wait;
	   }
	break;

	default:
	notes = Wait;
	break;
	}

	switch(notes){
	case Wait:
	   set_PWM(0.0);
	break;

	case C_4:
	   set_PWM(261.63);
	break;

	case D_4:
	   set_PWM(293.66);
	break;

	case E_4:
	   set_PWM(329.63);
	break;
	
	default:
	   set_PWM(0.0);
	break;		
	}

}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
    /* Insert your solution below */
	notes = SMStart;
	PWM_on();
    while (1) {
	Tick_Fct();	
    }
    return 1;
}
