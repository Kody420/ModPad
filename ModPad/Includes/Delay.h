
/*
 * Delay.h
 *
 * Created: 21.07.2024 22:29:14
 *  Author: Kody
 */

/*
	Just a simple library for counting up. Thats it. Handy as a delay for controlling leds.
	
	!!Currently there is not a timer setup to use this library as millis!! Thats cos i fucked up
	the design of the PCB that this library was created for. I am forced to use a timer for something 
	different to achieve consistent counting.
*/
	//Includes:
		#ifndef DELAY_H_
		#define DELAY_H_
		
		#include <avr/interrupt.h>
		#include <stdio.h>
		#include <stdlib.h>

	//Variables:
		static volatile uint16_t delay1 = 0;	//Value that is being counted
		static volatile uint16_t delay2 = 0;	//Value that is being counted
		
	//Function prototypes:
		/*
		Function description:
			Increase counted variable by one.
		Argument:
			delay - Picking counter
		*/
void Counting(uint8_t delay);	
		//Function prototypes:
		/*
		Function description:
			Reset counter to zero.
		Argument:
			delay - Picking counter
		*/
void CounterReset(uint8_t delay);
		//Function prototypes:
		/*
		Function description:
			Return a value of counter.
		*/
uint16_t Counter(uint8_t delay); //Get the current value of delay

#endif /* DELAY_H_ */