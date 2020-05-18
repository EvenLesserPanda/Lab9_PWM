/*	Author: gyama009
 *  Partner(s) Name: 
 *	Lab Section: 022
 *	Assignment: Lab #9 Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

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
double arr[8] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};

enum States{Start, OFF_Release, ON_Press, ON_Release, OFF_Press, Increment, Decrement} state;

void Tick(){
        switch(state){
                case Start: // Initial transition
                        state = OFF_Release;
			cnt = 0;
                        break;
                case OFF_Release:
			if((~PINA & 0xFF) == 0x01){
				set_PWM(arr[cnt]);
				state = ON_Press;
			}
			else if((~PINA & 0xFF) == 0x00){
				state = OFF_Release;
			}
			break;
		case ON_Press:
			if((~PINA & 0xFF) == 0x01){
				state = ON_Press;
			}
			else if((~PINA & 0xFF) == 0x00){
				state = ON_Release;
			}
			break;
		case ON_Release:
			if((~PINA & 0xFF) == 0x02){
				if(cnt <= 6){
					cnt++;
					set_PWM(arr[cnt]);
				}
				state = Increment;
			}
			else if((~PINA & 0xFF) == 0x04){
				if(cnt >= 1){
					cnt--;
					set_PWM(arr[cnt]);
				}
				state = Decrement;
			}
			else if((~PINA & 0xFF) == 0x00){
				state = ON_Release;
			}
			else if((~PINA & 0xFF) == 0x01){
				set_PWM(0);
				state = OFF_Press;
			}
			break;
		case OFF_Press:
			if((~PINA & 0xFF) == 0x01){
				state = OFF_Press;
			}
			else if((~PINA & 0xFF) == 0x00){
				state = OFF_Release;
			}
			break;
		case Increment:
			if((~PINA & 0xFF) == 0x02){
				state = Increment;
			}
			else if((~PINA & 0xFF) == 0x00){
				state = ON_Release;
			}
			else if((~PINA & 0xFF) == 0x01){
				state = OFF_Press;
			}
			break;
		case Decrement:
			if((~PINA & 0xFF) == 0x04){
				state = Decrement;
			}
			else if((~PINA & 0xFF) == 0x00){
				state = ON_Release;
			}
			else if((~PINA & 0xFF) == 0x01){
				state = OFF_Press;
			}
			break;
                default:
                        state = Start;
                        break;
        } // Transitions
        switch(state){ // State actions
		case OFF_Release:
                        break;
                case ON_Press:
                        break;
                case ON_Release:
                        break;
                case OFF_Press:
                        break;
                case Increment:
                        break;
                case Decrement:
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
	while (1) {
		Tick();
	}
	PWM_off();
	return 0;
}
