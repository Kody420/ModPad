/*
 * SPIcom.h
 *
 * Created: 04.09.2024 22:54:22
 *  Author: Kody
 */ 


#ifndef SPICOM_H_
#define SPICOM_H_

	//Includes:
		#include <stdio.h>
		#include <avr/io.h>
		#include "..\ModPad\lufa\LUFA\Drivers\Peripheral\SPI.h"

	//Macros:
		#define MOSI PB2
		#define MISO PB3
		#define SCLK PB1
		#define SS PB0
		
		#define SS1 PC7
		#define SS2 PC6
		#define SS3 PD2
		
		#define NUM_OF_MODULES 3
		
		#define SPI_STATUS 0xA0
		#define SPI_DATA 0xA1
		#define SPI_ERROR 0xF

	//Variables:
		typedef struct{
			uint8_t ID;
			uint8_t data[3];
		}module;
	//Function prototypes:
		void SPIInit(void);
		void SPIGetData(module *modules);
		void SPISendData(uint16_t command, uint16_t data, uint8_t position);
			//Delay to wait for module to prepare for receiving or sending data
		void SPIDelay(uint16_t delay);
		
#endif /* SPICOM_H_ */