/*
 * LedMatrix.h
 *
 * Created: 12.04.2024 22:07:09
 *  Author: Kody
 */ 

#ifndef LEDMATRIX_H_
#define LEDMATRIX_H_

	//Includes:
		#include <avr/io.h>
		#include <avr/interrupt.h>
		#include <stdio.h>
		#include <stdbool.h>
		#include <stdlib.h>
		#include <avr/eeprom.h>

		#include "ButtonMatrix.h"
		#include <Includes/Delay.h>
		#include <Includes/usb_hid_keys.h>

	//Macros:
		#define LED_COLUMN_SIZE 4
		#define LED_ROW_SIZE 2

		#define ROW_LED1 PD0
		#define ROW_LED2 PC5
		#define COL_LED1 PD4
		#define COL_LED2 PD5
		#define COL_LED3 PD7
		#define COL_LED4 PD6

	//Variables:
		typedef struct {
			uint8_t ledRow[8];
			uint8_t ledCol[8];
		}LedMatrxPins_t;
	//Function Prototypes:	
		/*
		Function description:
			Used to setup correct pins and their direction. Setting timer for consistent refresh rate on each led.
		Return value:
			effectNum saved in eeprom.
		*/
		uint16_t LedMatrixInit(void);
		/*
		Function description:
			Just a switch case to choose which effect is currently being used. Main function is to write in to 2D array brightness to set all the LEDs
			to right brightness level.
		Arguments:
			effectNum - carries the current effect number that will be executed
			buttonStatus - used to see what keys are currently being pressed
		*/
		void LedMatrixEffect(uint16_t effectNum, pressedButton_t* buttonStatus);
		/*
		Function description:
			A switch case to choose which modifier is being applied. If needed modified values are saved to eeprom
		Arguments:
			modifierNum - carries the current modifier that will be executed
		*/
		void LedMatrixModifier(uint16_t modifierNum);
		/*
		Function description:
			A switch case to choose what event is going to get displayed on LEDs.
		Arguments:
			eventNum - carries the current event that will be executed
		*/
		void LedMatrixEvent(uint16_t eventNum);
		/*
		Function description:
			Controlling outputs (LEDs) based on the value in array brightness. Timer2 overflow is used for specific refresh frequency.
		*/
		void LedRefresh(void);

#endif /* LEDMATRIX_H_ */