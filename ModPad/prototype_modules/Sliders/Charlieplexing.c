/*
 * Charlieplexing.c
 *
 * Created: 06.09.2024 16:36:15
 *  Author: Kody
 */ 

#include "Charlieplexing.h"

void CharliPlexInit(){
	//Timer0 setup
	TCCR0A |= (1 << WGM01) | (1 << WGM00);	//Fast PWM
	TCCR0B |= (1 << CS01) | (1 << CS00);	//Prescaler 64
	TIMSK0 |= (1 << TOIE0);		//Interrupt on overcurrentLed
	sei();
}

void CharliPlexEffect(uint16_t effectNum, uint8_t* sliderValues){
	static uint8_t delta = 1;
	static uint8_t maxBrightness = 254;
	//Connected:			//Written:
	//{{D5, D4, D2},		{{D4, D2, D3},
	// {D3, D8, D9}};	     {D8, D9, D5}};
	static uint8_t targetBrightness [2][3] = {{0,0,0},
											  {0,0,0}};
	static uint16_t	prevEffectNum = 0;
	static uint16_t pseudoRandom = 1;

	uint8_t Custombrightness [2][3] = {{40,80,120},
									   {200,240,250}};
	bool effectChange = false;
	if (effectNum != prevEffectNum)
	{
		prevEffectNum = effectNum;
		effectChange = true;
	}
		//Effect copied from 
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
				uint32_t sliderLeds = ((uint32_t)sliderValues[i]*762)/100;
				for (int x = 0;x < LED_COLUMN_SIZE;x++)
				{
					if (sliderLeds > 254*(x+1))brightness[i][x] = 254;
					else {
						brightness[i][x] = (uint8_t) sliderLeds - 254*x;
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
						if(brightness[x][i] == targetBrightness[x][i])targetBrightness[x][i] = pseudoRandom % ((maxBrightness/2)+1);
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

ISR(TIMER0_OVF_vect){
	uint8_t ledPins[] = {LEDA1, LEDB2, LED3};
	static uint8_t currentLed = 0;
	
	OCR0B = OCR0A = brightness[currentLed < 3 ? 0 : 1][currentLed%3];
	switch(currentLed){
	case 0:
		TCCR0A &= ~(1 << COM0A1);
		DDRD |= (1 << ledPins[0]);
		PORTD |= (1 << ledPins[0]);
		
		DDRD |= (1 << ledPins[1]);
		TCCR0A |= (1 << COM0B1);  
		
		DDRD &= ~(1 << ledPins[2]);
		PORTD &= ~(1 << ledPins[2]);
	break;
	case 2:
		DDRD |= (1 << ledPins[2]);
		PORTD |= (1 << ledPins[2]);

		PORTD &= ~(1 << ledPins[1]);
		
		DDRD &= ~(1 << ledPins[0]);
		PORTD &= ~(1 << ledPins[0]);
	break;
	case 4:
		TCCR0A &= ~(1 << COM0B1);  
		DDRD |= (1 << ledPins[2]);
		PORTD |= (1 << ledPins[2]);

		DDRD |= (1 << ledPins[0]);
		PORTD &= ~(1 << ledPins[0]);
		TCCR0A |= (1 << COM0A1);

		DDRD &= ~(1 << ledPins[1]);
		PORTD &= ~(1 << ledPins[1]);
	break;
	default:
		OCR0B = OCR0A = 0xff - brightness[currentLed < 3 ? 0 : 1][currentLed % 3];
		PORTD &= ~((1 << ledPins[0]) | (1 << ledPins[1]) | (1 << ledPins[2]));	
	}
	currentLed = (currentLed + 1) % 6;
	Counting(1);
}