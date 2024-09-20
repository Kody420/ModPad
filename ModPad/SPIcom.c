/*
 * SPIcom.c
 *
 * Created: 04.09.2024 22:54:33
 *  Author: Kody
 */ 

#include "SPIcom.h"

void SPIInit(){
	SPI_Init(SPI_SPEED_FCPU_DIV_8 | SPI_ORDER_MSB_FIRST  | SPI_MODE_MASTER);
	DDRC |= (1 << SS1) | (1 << SS2);
	DDRB &= ~(1 << MISO);
	PORTC |= (1 << SS1) | (1 << SS2);
}

void SPIGetData(module *modules){
	uint8_t SS_pins[]= {SS1, SS2, SS3};
	for (int i = 0; i < NUM_OF_MODULES-1; i++)
	{
		PORTC &= ~(1 << SS_pins[i]);
		SPIDelay(200);
		uint8_t statusID = SPI_ReceiveByte();
		SPIDelay(20);
		if (!(statusID & (1 << 7)))
		{
			SPI_SendByte(0x00);		//Have to send byte here to release module from switch statement
			PORTC |= (1 << SS_pins[i]);
			continue;
		}
		modules[i].ID = statusID & ~(1 << 7);
		
		SPI_SendByte(SPI_DATA);		//Sending command to get data
		for (int x = 0; x < 3; x++)
		{
			SPIDelay(20);
			modules[i].data[x] = SPI_ReceiveByte();
		}
		PORTC |= (1 << SS_pins[i]);
	}
}

void SPISendData(uint16_t command, uint16_t data, uint8_t position){
	uint8_t SS_pins[]= {SS1, SS2, SS3};
	for (int i = 0; i < NUM_OF_MODULES-1; i++)
	{
		PORTC &= ~(1 << SS_pins[position]);
		SPIDelay(200);
		uint8_t statusID = SPI_ReceiveByte();
		SPIDelay(20);
		SPI_SendByte((uint8_t)command);
		SPIDelay(20);
		SPI_SendByte((uint8_t)data);
		PORTC |= (1 << SS_pins[position]);
	}
}

void SPIDelay(uint16_t delay){
	for (int i = 0; i < delay; i++)asm volatile("nop");
}