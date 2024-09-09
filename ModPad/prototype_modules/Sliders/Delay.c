
/*
 * Delay.c
 *
 * Created: 21.07.2024 22:29:24
 *  Author: Kody
 */ 

#include "Delay.h"

void Counting(uint8_t delay){
	if (delay == 1)delay1++;
	else if (delay == 2)delay2++;
}

void CounterReset(uint8_t delay){
	if (delay == 1)delay1 = 0;
	else if (delay == 2)delay2 = 0;
}
uint16_t Counter(uint8_t delay){
	if (delay == 1)return delay1;
	else if (delay == 2)return delay2++;
	return delay == 1 ? delay1 : delay2;
}