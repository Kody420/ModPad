/*
 * LedMatrix.c
 *
 * Created: 12.04.2024 22:07:18
 *  Author: Kody
 */ 
	//Includes:
		#include "LedMatrix.h"							 

	//Variables:
		LedMatrxPins_t pins;
		uint8_t brightness [LED_ROW_SIZE][LED_COLUMN_SIZE];
		uint8_t maxBrightness = 255;
		uint16_t EEMEM eepromEffectNum = KEY_RESERVED;
		uint8_t EEMEM eepromMaxBrightness = 255;

uint16_t LedMatrixInit(void)
{
	eeprom_busy_wait();
	uint16_t effectNum = eeprom_read_word(&eepromEffectNum);
	eeprom_busy_wait();
	maxBrightness = eeprom_read_byte(&eepromMaxBrightness);
	
	uint8_t columnPins[COLUMN_SIZE] = {COL_LED1, COL_LED2, COL_LED3, COL_LED4};
	uint8_t rowPins[ROW_SIZE] = {ROW_LED1, ROW_LED2};
		//Saving into struct and setting pins
	for (int i = 0;i < LED_COLUMN_SIZE;i++)
	{
		pins.ledCol[i] = columnPins[i];
		DDRD |= (1 << columnPins[i]);	
	}
	for (int i = 0;i < LED_ROW_SIZE;i++)
	{
		pins.ledRow[i] = rowPins[i];
		DDRD |= (1 << rowPins[0]);
		DDRC |= (1 << rowPins[1]);
		
		PORTD |= (1 << rowPins[0]);
		PORTC |= (1 << rowPins[1]);
	}
		//Synchronization of timers
	GTCCR |= (1 << TSM);
	GTCCR |= (1 << PSRSYNC);
	
		//Timer0 for first row
	TCCR0A |= (1 << WGM01) | (1 << WGM00) | (1 << COM0B1) | (1 << COM0B0);		//Fast PWM mode and set on match, clear at top
	TCCR0B |= (1 << CS02);		//prescaler clk/256 should be equivalent to 61 Hz refresh rate on each led
	TIMSK0 |= (1 << TOIE0);		//Turning on interrupts
	OCR0B = 0;
		//Timer1 for second row
	TCCR1A |= (1 << COM1B1) | (1 << COM1B0) | (1 << WGM10);		//Fast PWM mode and set on match, clear at top
	TCCR1B |= (1 << WGM12) | (1 << CS12);
	OCR1BL = 0;
	
		//Enable prescaler to start counting with both timers at the same time
	GTCCR &= ~(1 << TSM);
	return effectNum;
}

void LedMatrixModifier(uint16_t modifierNum)
{
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

void LedMatrixEvent(uint16_t eventNum)
{
	switch(eventNum)
	{
		case USB_CONNECT:
		
		break;
		
		case USB_DISCONNECT:
		
		break;
		
		case USB_ERROR:
		
		break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
			eventNum--;
			for (int i = 0;i < LED_COLUMN_SIZE;i++)
			{
				for (int x = 0;x < LED_ROW_SIZE;x++)
				{
					if (brightness[x][i] > 0)--brightness[x][i];
				}
			}
			brightness[eventNum >= 4 ? 1 : 0][eventNum%4] = maxBrightness;
		break;
		case KEY_RESERVED:
		break;
	}
}

void LedMatrixEffect(uint16_t effectNum, pressedButton_t* buttonStatus)
{
	static uint8_t delta = 1;
	static uint8_t targetBrightness [2][4] = {{0,0,0,0},
											  {0,0,0,0}};
	static uint16_t	prevEffectNum = 0;
	static uint8_t prevMaxBrightness = 0;
	static uint16_t pseudoRandom = 1;
	
	uint8_t activeLed = 0;
	uint8_t Custombrightness [2][4] = {{40,80,120,160},
									   {200,240,255,0}};
	bool effectChange = false;
	
	if (effectNum != prevEffectNum)
	{
		eeprom_busy_wait();
		if (prevEffectNum != 0)eeprom_write_word(&eepromEffectNum, effectNum);
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
		
		case KEY_EFFECT4:		//Button activated
			while (buttonStatus[activeLed].duration != 0)
			{
				brightness[buttonStatus[activeLed].row][buttonStatus[activeLed].column] = maxBrightness;
				activeLed++;
			}
			if (Counter(1) > 3)
			{
				CounterReset(1);
				for (int i = 0;i < LED_COLUMN_SIZE;i++)
				{
					for (int x = 0;x < LED_ROW_SIZE;x++)
					{
						if (brightness[x][i] > 0)brightness[x][i]--;
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

	//Refreshing of leds
ISR(TIMER0_OVF_vect){
	TCNT1L = 0xFF;
	Counting(1);
	LedRefresh();
}

void LedRefresh(void){
	static int activeColumn = 0;
	PORTD &= ~(1 << pins.ledCol[(activeColumn + LED_COLUMN_SIZE - 1) % LED_COLUMN_SIZE]);
	if (brightness[0][activeColumn] == 0)TCCR0A &= ~((1 << COM0B1) | (1 << COM0B0));
	else 
	{
		OCR0B = brightness[0][activeColumn];
		TCCR0A |= (1 << COM0B1) | (1 << COM0B0);
	}
	if (brightness[1][activeColumn] == 0)TCCR1A &= ~((1 << COM1B1) | (1 << COM1B0));
	else
	{
		OCR1BL = brightness[1][activeColumn];
		TCCR1A |= (1 << COM1B1) | (1 << COM1B0);
	}
	PORTD |= (1 << pins.ledCol[activeColumn]);
	activeColumn = (activeColumn + 1) % LED_COLUMN_SIZE;
}
