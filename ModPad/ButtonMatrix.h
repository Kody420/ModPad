/*
 * ButtonMatrix.h
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

	//Includes:
		#include <avr/io.h>
		#include <avr/interrupt.h>
		#include <stdio.h>
		#include <stdbool.h>
	
	//Macros:
		#define COLUMN_SIZE 4
		#define ROW_SIZE 2

		#define COL1 PB7
		#define COL2 PB6
		#define COL3 PB5
		#define COL4 PB4
		#define ROW1 PC2
		#define ROW2 PD1
		#define BUTTON PC4
		
	//Variables:		  
		typedef struct {
			uint8_t row;
			uint8_t column;
			uint8_t duration;
		}pressedButton_t;
	
	//Function Prototypes:
		/*
		Function description:
			Setting pin directions
		*/
		void MatrixInit(void);
		/*
		Function description:
			Reading which key is being pressed. Supports multi-key press registration.
		Return value:
			Pointer to array of structs holding coordinates what keys and for how long are pressed.
		*/
		pressedButton_t* ReadKey(void);
		/*
		Function description:
			Reading of additional button on the board. Debounce implemented.
		Return value:
			Bool if button is pressed or not.
		*/
		bool ButtonPress(void);

#endif /* BUTTONMATRIX_H_ */