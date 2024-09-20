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
#include <avr/eeprom.h>

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
		
		#define FEATR_EFFECT 0x01
		#define FEATR_BRIGHTNESS 0x02
		#define FEATR_PROFILE 0x03
		#define FEATR_MAPPING 0x04
		
	//Variables:
		//Make struct to save sliderValues and if they changed
		typedef struct {
			uint8_t values[3];
			bool newValues;
		}values_t;
		
	    values_t sliders;
		uint16_t effectNum;
		uint16_t modifierNum;
		
void ReadSliders(values_t* sliders){
	uint8_t pinSelect [] = {SLIDER1, SLIDER2, SLIDER3};
	for (uint8_t activeSlider = 0; activeSlider < NUM_OF_SLIDERS; activeSlider++)
	{
		ADMUX = (ADMUX & 0xF0) | (pinSelect[activeSlider] & 0x0F);
		ADCSRA |= (1 << ADSC);
		while(ADCSRA & (1 << ADSC));
		//Remapping to 0-100
		//Reference voltage is not constant so read values are inconsistent 
		uint16_t value = ((ADCH >> 1) * 100 + 63) / 127;
		if (value != sliders->values[activeSlider])
		{
			sliders->newValues = true;
			sliders->values[activeSlider] = (uint8_t) value;
		}
	}
}		
void SlidersInit(){                      
	ADMUX |= (1 << REFS0) | (1 << ADLAR);	//AVCC reference with external capacitor at AREF pin, left adjusted result 
	ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1);	//Turn ADC on, division factor at 64
	ReadSliders(&sliders);
}		
	
int main(void)
{
	SlidersInit();
	effectNum = CharliPlexInit();
	SPI_Init(SPI_SPEED_FCPU_DIV_8 | SPI_ORDER_MSB_FIRST  | SPI_MODE_SLAVE);

	PCICR |= (1 << PCIE0);	//Enable pin change on pins PCINT7..0
	PCMSK0 |= (1 << PCINT2);	//Enable pin change interrupt on SS

    while (1) 
    {
		ReadSliders(&sliders);
		CharliPlexEffect(KEY_EFFECT4, sliders.values);
		if (modifierNum)
		{
			CharliPLexModifier(modifierNum);
			modifierNum = 0;
		}
    }	
}

ISR(PCINT0_vect){
	if (!(PINB & (1 << SS)))
	{
		SPI_SendByte(MODULE_ID | (sliders.newValues << 7));
		switch(SPI_ReceiveByte())
		{
			case SPI_DATA:
				SPI_SendByte(sliders.values[0]);
				SPI_SendByte(sliders.values[1]);
				SPI_SendByte(sliders.values[2]);
			break;
			case FEATR_EFFECT:
				effectNum = SPI_ReceiveByte() + 0xff;
			break;
			case FEATR_BRIGHTNESS:
				modifierNum = SPI_ReceiveByte() + 0x1ff;
			break;
		}
	}
}

