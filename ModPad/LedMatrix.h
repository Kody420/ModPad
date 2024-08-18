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

		#define EVENT 0x09
		#define USB_CONNECT 0x0a
		#define USB_DISCONNECT 0x0b
		#define USB_ERROR 0x0c

	//Variables:
		typedef struct {
			uint8_t ledRow[8];
			uint8_t ledCol[8];
		}LedMatrxPins_t;

		LedMatrxPins_t pins;

		uint8_t brightness [LED_ROW_SIZE][LED_COLUMN_SIZE];
	
	//Function Prototypes:	
		/*
		Function description:
			Used to setup correct pins and their direction. Setting timer for consistent refresh rate on each led.
		*/
		void LedMatrixInit(void);

		/*
		Function description:
			Just a switch case to choose which effect is currently being used. Main function is to write in to 2D array Brightness to set all the leds
			to right brightness level.
		Arguments:
			effectNum - carries the current effect number for switch case
			effectModifier - carries the current modifier that will be executed 
			buttonStatus - used to see what keys are currently being pressed
		Return value:
			If the effectNum or effectModifier values are not found return 1
		*/
		uint16_t LedMatrixEffect(uint16_t effectNum, uint16_t effectModifier, pressedButton_t* buttonStatus);

		/*
		Function description:
			Controlling outputs (LEDs) based on the value in array Brightness. Timer0 overflow is used for specific refresh frequency.
		*/
		void LedRefresh(void);

#endif /* LEDMATRIX_H_ */