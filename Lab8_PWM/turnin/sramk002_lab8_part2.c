/*  Author: Sidharth Ramkumar (sramk002@ucr.edu)
 *  Partner(s) Name: none 
 *	Lab Section: 022
 *	Assignment: Lab #8  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#define b1 (~PINA & 0x01)
#define b2 (~PINA & 0x02)
#define b3 (~PINA & 0x04)

enum Speaky_On { SMStart1, Off, On, Wait_On } speaky;
enum Note_States { SMStart2, Wait, Inc, Dec, Inc_Wait, Dec_Wait } notes;
 
int speaky_note;

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

void Tick_Fct1(){
	switch(speaky){
	case SMStart1:
	speaky = Off;
	break;

	case Off:
	if (!b1){
	speaky = Off;
	} else if (b1){
	speaky = On;
	}
	break;

	case On:
	if (!b1){
	speaky = Wait_On;
	} else {
	speaky = On;
	}
	break;

	case Wait_On:
	if (!b1){
	speaky = Wait_On;
	} else if (b1){
	speaky = Off;
	}
	break;
	}

	switch(speaky){
	case Off:
	PWM_off();
	break;
	
	case On:
	PWM_on();
	break;

	default:
	break;
	}
}

void Tick_Fct2(double freqs[]){
	switch(notes){
   	case SMStart2:
	notes = Wait;
	break;

	case Wait:
	if (b2){
		notes = Inc;
	} else if (b3){
		notes = Dec;
	} else {
		notes = Wait;
	}
	break;

	case Inc:
	notes = Inc_Wait;
	break;
	
	case Dec:
	notes = Dec_Wait;
	break;
	
	case Inc_Wait:
	if (b2){
	notes = Inc_Wait;
	} else {
	notes = Wait;
	}
	break;

	case Dec_Wait:
	if (b3){
	notes = Dec_Wait;
	} else {
	notes = Wait;
	}
	break;
}

	switch(notes){
	case Inc:
		if (speaky_note < 8){
		speaky_note += 1;
		set_PWM(freqs[speaky_note]);
		}		
	break;

	case Dec:
		if (speaky_note > 0){
		speaky_note -= 1;
		set_PWM(freqs[speaky_note]);
		}
	break;

	default:
	break;
	}
}



int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
    /* Insert your solution below */
	double freq[8] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};
	speaky_note = 0;
	speaky = SMStart1;
	notes = SMStart2;
    while (1) {
	Tick_Fct1();	
	Tick_Fct2(freq);
    }
    return 1;
}
