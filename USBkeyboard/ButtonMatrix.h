/*
 * Buttons.h
 *
 * Created: 24.05.2024 23:28:26
 *  Author: Kody
 */ 

/*
	Library designed to control buttons on a board. Primary use is for button matrix and one other button. 
	The output is array with position of the button press and it's duration. Lonely button is outputted as bool value.
	There is not really a debounce implemented in to this library. What is relied on is long enough periods between 
	calls for reading the matrix. Al thou I might add ignore cycles after a button is pressed. 
*/
#ifndef BUTTONMATRIX_H_
#define BUTTONMATRIX_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>

//Struct to save what button and for how long is being pressed			  
typedef struct {
	uint8_t row;
	uint8_t column;
	uint8_t duration;
}pressedButton_t;

#define COLUMN_SIZE 4
#define ROW_SIZE 2

#define COL1 PB7
#define COL2 PB6
#define COL3 PB5
#define COL4 PB4
#define ROW1 PC2
#define ROW2 PD1
#define BUTTON PC4
/*
Function description:
	
*/
void MatrixInit(void);
pressedButton_t* ReadKey(void);
bool ButtonPress(void);

#endif /* BUTTONMATRIX_H_ */