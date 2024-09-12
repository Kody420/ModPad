/*
 * Sliders.c
 *
 * Created: 19.08.2024 15:25:32
 * Author : Kody
 */ 

#define F_CPU 16000000
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "SPI_AVR8.h"
#include "Charlieplexing.h"

	//Macros:
		//For ADMUX register to select pin
		#define SLIDER1	0x00
		#define SLIDER2 0x01
		#define SLIDER3 0x02
		#define NUM_OF_SLIDERS 3
		
		#define MODULE_ID 0x01	//max is 0x7f, first bit is reserved for status
		
		#define SPI_STATUS 0xA0
		#define SPI_DATA 0xA1
		#define SPI_ERROR 0xFF
		
	//Variables:
	    uint8_t* sliderValues;
		bool newValues = false;
void SlidersInit(){                      
	ADMUX |= (1 << REFS0) | (1 << ADLAR);	//AVCC reference with external capacitor at AREF pin, left adjusted result 
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);	//Turn ADC on, division factor at 64
}		
	
uint8_t* ReadSliders(){
	uint8_t sliders [] = {SLIDER1, SLIDER2, SLIDER3};
	static uint8_t send[NUM_OF_SLIDERS];
	for (uint8_t activeSlider = 0; activeSlider < NUM_OF_SLIDERS; activeSlider++)
	{
		ADMUX = (ADMUX & 0xF0) | (sliders[activeSlider] & 0x0F);
		ADCSRA |= (1 << ADSC);
		while(ADCSRA & (1 << ADSC));
		//Remapping to 0-100
		uint16_t values = ((ADCH >> 1) * 100 + 63) / 127;
		send[activeSlider] = (uint8_t) values;
	}
	return send;
}

int main(void)
{
	CharliPlexInit();
	SlidersInit();
	SPI_Init(SPI_SPEED_FCPU_DIV_8 | SPI_ORDER_MSB_FIRST  | SPI_MODE_SLAVE);

	PCICR |= (1 << PCIE0);	//Enable pin change on pins PCINT7..0
	PCMSK0 |= (1 << PCINT2);	//Enable pin change interrupt on SS
	
	static uint8_t prevSliderValues[NUM_OF_SLIDERS];
    while (1) 
    {
		sliderValues = ReadSliders();
		CharliPlexEffect(KEY_EFFECT4, sliderValues);
		for (int i = 0; i < NUM_OF_SLIDERS; i++)
		{
			if (prevSliderValues[i] != sliderValues[i])
			{
				newValues = true;
				prevSliderValues[i] = sliderValues[i];
			}
		}
    }	
}

ISR(PCINT0_vect){
	/*
		This fixes the flicker in leds but breaks SPI communication. It makes sense by turning global interrupts on this interrupt the leds can refresh even in here.
		But if that happens the communication can't complete and will be missed or send some garbage.
	*/
	//sei();
	if (!(PINB & (1 << SS)))
	{
		SPI_SendByte(MODULE_ID | (newValues << 7));
		switch(SPI_ReceiveByte())
		{
			case SPI_DATA:
			SPI_SendByte(sliderValues[0]);
			SPI_SendByte(sliderValues[1]);
			SPI_SendByte(sliderValues[2]);
			break;
		}
	}
}

