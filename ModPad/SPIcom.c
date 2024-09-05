/*
 * SPIcom.c
 *
 * Created: 04.09.2024 22:54:33
 *  Author: Kody
 */ 

#include "SPIcom.h"

void SPIInit(){
	SPI_Init(SPI_SPEED_FCPU_DIV_16 | SPI_ORDER_MSB_FIRST  | SPI_MODE_MASTER);
	DDRC |= (1 << SS1) | (1 << SS2);
	DDRB &= ~(1 << MISO);
	PORTC |= (1 << SS1) | (1 << SS2);
}

void SPICommunicate(module *modules){
	uint8_t SS_pins[]= {SS1, SS2, SS3};
	for (int i = 0; i < NUM_OF_MODULES-1; i++)
	{
		PORTC &= ~(1 << SS_pins[i]);
		SPI_SendByte(0x01);		//Sending command to get status and ID
		SPIDelay();
		uint8_t statusID = SPI_ReceiveByte();
		if (!(statusID & (1 << 7)))
		{
			PORTC |= (1 << SS_pins[i]);
			continue;
		}
		modules[i].ID = statusID & ~(1 << 7);
		SPI_SendByte(0x02);		//Sending command to get data
		switch(modules[i].ID){
			case 0x01:
				for (int x = 0; x < 3; x++)
				{
					SPIDelay();
					modules[i].data[x] = SPI_ReceiveByte();
				}
			break;
		}
		PORTC |= (1 << SS_pins[i]);
	}
}

void SPIDelay(){
	for (int i = 0; i < 1000; i++)asm("nop");
}
