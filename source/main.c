/*	Author: gyama009
 *  Partner(s) Name: 
 *	Lab Section: 022
 *	Assignment: Lab #9 Exercise #3
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
#include "timer.h"

// 0.954 hz is lowest frequency possible with this function, 
// based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound
void set_PWM(double frequency){
	static double current_frequency; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if(frequency != current_frequency){
		if(!frequency){TCCR3B &= 0x08;} // stops timer/counter
		else{TCCR3B |= 0x03;} // resumes/continues timer/counter

		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if(frequency < 0.954){OCR3A = 0xFFFF;}

		// prevents OCR3A from underflowing, using prescaler 64
		// 31250 is largest frequency that will not result in underflow
		else if(frequency > 31250){OCR3A = 0x0000;}

		// set OCR3A based on desired frequency
		else{OCR3A = (short) (8000000 / (128 * frequency)) - 1;}

		TCNT3 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}

void PWM_on(){
	TCCR3A = (1 << COM3A0);
		// COM3A0: Toggle PB3 on compare match between counter and OCR3A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
		// WGM32: When counter (TCNT3) matches OCR3A, reset counter
		// CS31 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off(){
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

unsigned char cnt;
unsigned char holdval;
unsigned char spaceval;
double song[21] = {349.23, 349.23, 293.66, 349.23, 261.63, 349.23, 293.66, 261.63, 349.23, 261.63, 523.25, 587.32, 523.25, 587.32, 523.25, 261.63, 466.16, 440.00, 392.00, 349.23, 698.46};
unsigned char hold[21] = {4, 4, 1, 6, 1, 1, 1, 1, 12, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 8, 2};
unsigned char space[21] = {2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4, 1};

enum States{Start, OFF, SONG, HOLD, SPACE} state;

void Tick(){
        switch(state){
                case Start: // Initial transition
                        cnt = 0;
			holdval = 0;
			spaceval = 0;
			state = OFF;
			break;
		case OFF:
			if((~PINA & 0xFF) == 0x00){
				state = OFF;
			}
			else if((~PINA & 0xFF) == 0x01){
				cnt = 0;
				state = SONG;
			}
			break;
		case SONG:
			if(cnt >= 21 && ((~PINA & 0xFF) == 0x00)){
				state = OFF;
			}
			else if(cnt < 21){
				holdval = 0;
				state = HOLD;
			}
			else if((cnt >= 21) && ((~PINA & 0xFF) == 0x01)){
				state = SONG;
			}
			break;
		case HOLD:
			if(holdval < hold[cnt]){
				holdval++;
				state = HOLD;
			}
			else if(holdval >= hold[cnt]){
				state = SPACE;
			}
			break;
		case SPACE:
			if(spaceval < space[cnt]){
				set_PWM(0);
				spaceval++;
				state = SPACE;
			}
			else if(spaceval >= space[cnt]){
				holdval = 0;
				spaceval = 0;
				cnt++;
				state = SONG;
			}
			break;
                default:
                        state = Start;
                        break;
        } // Transitions
        switch(state){ // State actions
		case OFF:
			set_PWM(0);
			break;
		case SONG:
			if(cnt < 21){
				set_PWM(song[cnt]);
			}
			break;
		case HOLD:
			break;
		case SPACE:
			break;
                default:
                        break;
        } // State actions
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as input
	DDRB = 0x40; PORTB = 0x00; // Configure port B's 8 pins as output
	PWM_on();
	state = Start;
	TimerSet(50);
	TimerOn();
	while (1) {
		Tick();
		while(!TimerFlag){}
		TimerFlag = 0;
	}
	PWM_off();
	return 0;
}
