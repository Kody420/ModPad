/*
 * Buttons.c
 *
 * Created: 24.05.2024 23:28:37
 *  Author: Kody
 */ 

#include "ButtonMatrix.h"

void MatrixInit(){
	//Setting pins for button matrix
	DDRB &= ~((1 << COL1) | (1 << COL2) | (1 << COL3) | (1 << COL4));
	DDRC |= (1 << ROW1);
	DDRD |= (1 << ROW2);	
}

bool ButtonPress(){
	static uint8_t prevState = 0;
	if (!(PINC & (1 << BUTTON)) && prevState == 0)
	{
		prevState++;
		return 1;
	}
	if (prevState != 0)
	{
		if (prevState == 0xff)prevState = 101;
		prevState++;
	}
	if (PINC & (1 << BUTTON) && prevState > 100)prevState = 0;
	return 0;
}

pressedButton_t* ReadKey(){
	static pressedButton_t pressedKeys [ROW_SIZE*COLUMN_SIZE];
	//uint8_t rowPins[2] = {ROW1, ROW2};
	uint8_t columnPins[4] = {COL1, COL2, COL3, COL4};	
	uint8_t activeKeys = 0;
	for (int x = 0; x < ROW_SIZE; x++)
	{
		if (x)
		{
			PORTC &= ~(1 << ROW1);
			PORTD |= (1 << ROW2);
		}
		else
		{
			PORTC |= (1 << ROW1);
			PORTD &= ~(1 << ROW2);
		}
		for (int i = 0; i < COLUMN_SIZE; i++)
		{
			if (PINB & (1 << columnPins[i]))
			{
				if (pressedKeys[activeKeys].row != x || pressedKeys[activeKeys].column != i)	//Kinda problem with 0 0 not getting in but it shouldn't matter
				{																				//Inconsistency with long press caused by order of reading
					pressedKeys[activeKeys].row = x;											//Could by fixed by nulling duration when the array changes
					pressedKeys[activeKeys].column = i;
					pressedKeys[activeKeys].duration = 0;
				}
				pressedKeys[activeKeys].duration = (pressedKeys[activeKeys].duration % 150) + 1;
				activeKeys++;
			}
		}
	}
	PORTB &= ~(1 << ROW1);
	//Bc pressedKeys is static there is a need for manual reset
	for (int i = activeKeys + 1; i < ROW_SIZE*COLUMN_SIZE; i++)
	{
		pressedKeys[activeKeys].duration = 0;
	}
	return pressedKeys;
}