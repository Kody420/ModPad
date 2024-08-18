
/*
 * Delay.c
 *
 * Created: 21.07.2024 22:29:24
 *  Author: Kody
 */ 

#include "Delay.h"

void Counting(void){
	delay++;
}

void CounterReset(void){
	delay = 0;
}
uint16_t Counter(void){
	return delay;
}