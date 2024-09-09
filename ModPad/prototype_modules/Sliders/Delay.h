
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
#ifndef DELAY_H_
#define DELAY_H_

#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

static volatile uint16_t delay1 = 0;	//Value that is being counted
static volatile uint16_t delay2 = 0;	//Value that is being counted

void Counting(uint8_t delay);	//Increase delay by one
void CounterReset(uint8_t delay);	//Reset delay to zero
uint16_t Counter(uint8_t delay); //Get the current value of delay

#endif /* DELAY_H_ */