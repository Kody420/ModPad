/*
 * Charlieplexing.c
 *
 * Created: 06.09.2024 16:36:15
 *  Author: Kody
 */ 

#include "Charlieplexing.h"

uint8_t brightness[2][3];
uint8_t maxBrightness;
uint16_t EEMEM eepromEffectNum;
uint8_t EEMEM eepromMaxBrightness;

uint16_t CharliPlexInit(){
	//Timer0 setup
	TCCR0A |= (1 << WGM01) | (1 << WGM00);	//Fast PWM
	TCCR0B |= (1 << CS01);	//Prescaler 8
	
	TCCR2A |= (1 << WGM21) | (1 << WGM20);	//Fast PWM
	TCCR2B |= (1 << CS21) | (1 << CS20);	//Prescaler 64
	TIMSK2 |= (1 << TOIE2);		//Interrupt on overcurrentLed
	
	eeprom_busy_wait();
	maxBrightness = eeprom_read_byte(&eepromMaxBrightness);
	sei();
	eeprom_busy_wait();
	return eeprom_read_word(&eepromEffectNum);
}

void CharliPLexModifier(uint16_t modifierNum){
	//Effect modifier switch
	switch(modifierNum)
	{
		case KEY_BRIGHTNESS_UP:
		if (maxBrightness == 255)break;
		maxBrightness += 51;
		eeprom_busy_wait();
		eeprom_write_byte(&eepromMaxBrightness, maxBrightness);
		break;
		
		case KEY_BRIGHTNESS_DOWN:
		if (maxBrightness == 0)break;
		maxBrightness -= 51;
		eeprom_busy_wait();
		eeprom_write_byte(&eepromMaxBrightness, maxBrightness);
		break;
		case KEY_RESERVED:
		break;
	}
}

void CharliPlexEffect(uint16_t effectNum, uint8_t* sliderValues){
	//Effects copied from LedMatrix.c
	static uint8_t delta = 1;
	static uint8_t targetBrightness [2][3] = {{0,0,0},
											  {0,0,0}};					  
	static uint16_t	prevEffectNum = 0;
	static uint16_t pseudoRandom = 1;
	static uint8_t prevMaxBrightness = 0;

	uint8_t Custombrightness [2][3] = {{40,80,120},
									   {200,240,250}};
	bool effectChange = false;
	if (effectNum != prevEffectNum)
	{
		eeprom_busy_wait();
		eeprom_write_word(&eepromEffectNum, effectNum);
		prevEffectNum = effectNum;
		effectChange = true;
	}
	if (maxBrightness != prevMaxBrightness)
	{
		prevMaxBrightness = maxBrightness;
		effectChange = true;
	}

	switch(effectNum)
	{
		case KEY_EFFECT1:		//All off
			CounterReset(1);
			if (effectChange)
			{
				for (int i = 0;i < LED_COLUMN_SIZE;i++)
				{
					for (int x = 0;x < LED_ROW_SIZE;x++)
					{
						brightness[x][i] = 0x00;
					}
				}
			}
		break;
		
		case KEY_EFFECT2:		//All on with same brightness
			CounterReset(1);
			if (effectChange)
			{
				for (int i = 0;i < LED_COLUMN_SIZE;i++)
				{
					for (int x = 0;x < LED_ROW_SIZE;x++)
					{
						brightness[x][i] = maxBrightness;
					}
				}
			}
		break;
		
		case KEY_EFFECT3:		//Breathing from 0 to max
			if (effectChange)brightness[0][0] = maxBrightness/2;
			if (Counter(1) > 3)
			{
				CounterReset(1);
				if(brightness[0][0] == 0 || brightness[0][0] == maxBrightness) delta = -delta;
				if (maxBrightness != 0)brightness[0][0] += delta;	//if for edge case when maxBrightness is 0
				for (int i = 0;i < LED_COLUMN_SIZE;i++)
				{
					for (int x = 0;x < LED_ROW_SIZE;x++)
					{
						brightness[x][i] = brightness[0][0];
					}
				}
			}
		break;
		
		case KEY_EFFECT4:		//Slider position activated
			for (int i = 0;i < LED_ROW_SIZE;i++)
			{
				uint32_t sliderLeds = ((uint32_t)sliderValues[i] * (maxBrightness * 3)) / 100;
				for (int x = 0;x < LED_COLUMN_SIZE;x++)
				{
					if (sliderLeds > maxBrightness * (x + 1))brightness[i][x] = maxBrightness;
					else {
						brightness[i][x] = (uint8_t) sliderLeds - maxBrightness * x;
						break;
					}
				}
			}
		break;
		
		case KEY_EFFECT5:		//Custom brightness levels
			CounterReset(1);
			for (int x = 0;x < LED_ROW_SIZE;x++)
			{
				for (int i = 0;i < LED_COLUMN_SIZE;i++)
				{
					brightness[x][i] = Custombrightness[x][i];
				}
			}
		break;
		case KEY_EFFECT6:		//Random
			if (Counter(1) > 2)
			{
				CounterReset(1);
				for (int x = 0;x < LED_ROW_SIZE;x++)
				{
					for (int i = 0;i < LED_COLUMN_SIZE;i++)
					{
						pseudoRandom = (pseudoRandom * 156 + 53) % 0xFF;
						if(brightness[x][i] == targetBrightness[x][i])targetBrightness[x][i] = pseudoRandom % ((maxBrightness/2) + 1);
						if(brightness[x][i] > targetBrightness[x][i])brightness[x][i]--;
						else brightness[x][i]++;
					}
				}
			}
		break;
		case KEY_RESERVED:
		break;
	}
}

ISR(TIMER2_OVF_vect){
	uint8_t ledPins[] = {LEDA1, LEDB2, LED3};
	static uint8_t currentLed = 0;
	//Reseting pins to highZ at the start of each addressing
	DDRD &= ~((1 << ledPins[0]) | (1 << ledPins[1]) | (1 << ledPins[2]));
	PORTD &= ~((1 << ledPins[0]) | (1 << ledPins[1]) | (1 << ledPins[2]));
	TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0) | (1 << COM0B1) | (1 << COM0B0));
	
	//Offsetting array brightness to match LEDs in reality
	//Connected irl:			//Written in brightness:
	//{{D5, D4, D2},	<=		{{D4, D2, D3},
	// {D3, D8, D9}};			{D8, D9, D5}};
	uint8_t offset = (currentLed + 2) % 6;	
	uint8_t currentBrightness = brightness[offset < 3 ? 0 : 1][offset%3];
	OCR0B = OCR0A = currentBrightness;
	
	//Setting the pin configuration for each led separately because i can't figure any other way
	switch(currentLed){
		case 0:
			if (currentBrightness > 10)
			{
				DDRD |= (1 << ledPins[0]);
				PORTD |= (1 << ledPins[0]);
			
				DDRD |= (1 << ledPins[1]);
				TCCR0A |= (1 << COM0B1)  | (1 << COM0B0);
			}
		break;
		
		case 1:
			if (currentBrightness > 10)
			{
				DDRD |= (1 << ledPins[0]);
				PORTD &= ~(1 << ledPins[0]);
				
				DDRD |= (1 << ledPins[1]);
				TCCR0A |= (1 << COM0B1);
			}
		break;
		
		case 2:
			if (currentBrightness > 10)
			{
				DDRD |= (1 << ledPins[2]);
				PORTD |= (1 << ledPins[2]);
				
				DDRD |= (1 << ledPins[1]);
				TCCR0A |= (1 << COM0B1)  | (1 << COM0B0);
			}
		break;
		
		case 3:
			if (currentBrightness > 10)
			{
				DDRD |= (1 << ledPins[2]);
				PORTD &= ~(1 << ledPins[2]);
				
				DDRD |= (1 << ledPins[1]);
				TCCR0A |= (1 << COM0B1);
			}
		break;
		
		case 4:
			if (currentBrightness > 10)
			{
				DDRD |= (1 << ledPins[2]);
				PORTD |= (1 << ledPins[2]);
				
				DDRD |= (1 << ledPins[0]);
				TCCR0A |= (1 << COM0A1) | (1 << COM0A0);
			}
		break;
		
		case 5:
			if (currentBrightness > 10)
			{
				DDRD |= (1 << ledPins[2]);
				PORTD &= ~(1 << ledPins[2]);
				
				DDRD |= (1 << ledPins[0]);
				TCCR0A |= (1 << COM0A1);
			}
		break;
	}
	currentLed = (currentLed + 1) % 6;
	Counting(1);
}
