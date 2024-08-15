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
#ifndef BUTTONS_H_
#define BUTTONS_H_


	/* Includes: */
	// TODO: Add any required includes here
		#include <avr/io.h>
		#include <stdio.h>

	/* Enable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
		extern "C" {
			#endif

	/* Preprocessor Checks: */
		#if !defined(__INCLUDE_FROM_BUTTONS_H)
			#error Do not include this file directly. Include LUFA/Drivers/Board/Buttons.h instead.
		#endif

	/* Public Interface - May be used in end-application: */
		/* Macros: */
		/** Button mask for the first button on the board. */
		#define COL1 PB4
		#define COL2 PB5
		#define COL3 PB6
		#define COL4 PB7
		#define ROW1 PD1
		#define ROW2 PC2
		#define BUTTON PC4
		
		#define COLUMN_SIZE 4
		#define ROW_SIZE 2

	/* Inline Functions: */
		#if !defined(__DOXYGEN__)
		static inline void ButtonsInit(void)
		{
			DDRB &= ~((1 << COL1) | (1 << COL2) | (1 << COL3) | (1 << COL4));
			DDRD |= (1 << ROW1);
			DDRC |= (1 << ROW2);
		}

		static inline void ButtonsDisable(void)
		{
			DDRB &= ~((1 << COL1) | (1 << COL2) | (1 << COL3) | (1 << COL4));
			DDRD &= ~(1 << ROW1);
			DDRC &= ~(1 << ROW2);
			
			PORTB &= ~((1 << COL1) | (1 << COL2) | (1 << COL3) | (1 << COL4));
			PORTD &= ~(1 << ROW1);
			PORTC &= ~(1 << ROW2);
		}

		ATTR_WARN_UNUSED_RESULT
		static inline uint8_t ButtonsGetStatus(uint8_t pin)
		{
			switch(pin){
				case COL1:
					return PINB & (1 << COL1);
				break;
				case COL2:
					return PINB & (1 << COL2);
				break;
				case COL3:
					return PINB & (1 << COL3);
				break;
				case COL4:
					return PINB & (1 << COL4);
				break;
				case ROW1:
					return PIND & (1 << ROW1);
				break;
				case ROW2:
					return PINC & (1 << ROW2);
				break;
				case BUTTON:
					return PINC & (1 << BUTTON);
				break;
			}
		}
		#endif

	/* Disable C linkage for C++ Compilers: */
		#if defined(__cplusplus)
		}
		#endif

#endif /* BUTTONMATRIX_H_ */