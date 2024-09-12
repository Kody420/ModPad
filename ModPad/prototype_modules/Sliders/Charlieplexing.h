/*
 * Charlieplexing.h
 *
 * Created: 06.09.2024 16:36:24
 *  Author: Kody
 */ 


#ifndef CHARLIEPLEXING_H_
#define CHARLIEPLEXING_H_
	
	#define F_CPU 16000000
	//Includes:
		#include <avr/io.h>
		#include <avr/interrupt.h>
		#include <stdbool.h>
		#include <util/delay.h>
		#include <limits.h>
		#include "Delay.h"
		#include "../../Includes/usb_hid_keys.h"
	
	//Macros:
		#define LEDA1 PD6
		#define LEDB2 PD5
		#define LED3 PD4
		#define LEDS (LEDA1 | LEDB2 | LED3)
		#define NUM_OF_LEDS 6
		#define LED_COLUMN_SIZE 3
		#define LED_ROW_SIZE 2
	
	//Variables:
		uint8_t brightness[2][3];
		
	//Functions:
		void CharliPlexInit(void);
		void CharliPlexEffect(uint16_t effectNum, uint8_t* sliderValues);
		
#endif /* CHARLIEPLEXING_H_ */