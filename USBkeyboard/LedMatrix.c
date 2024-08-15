/*
 * LedMatrix.c
 *
 * Created: 12.04.2024 22:07:18
 *  Author: Kody
 */ 

#include "LedMatrix.h"							 

void LedMatrixInit(void)
{
	uint8_t columnPins[COLUMN_SIZE] = {COL_LED1, COL_LED2, COL_LED3, COL_LED4};
	uint8_t rowPins[ROW_SIZE] = {ROW_LED1, ROW_LED2};
		//Saving into struct
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
	}
		//Attempt at synchronization of timers
	GTCCR |= (1 << TSM);
	GTCCR |= (1 << PSRSYNC);
		//Timer0 for first row
	TCCR0A |= (1 << WGM01) | (1 << WGM00) | (1 << COM0B1) | (1 << COM0B0);		//Fast PWM mode and set on match, clear at top
	//TCCR0A |= (1 << WGM01) | (1 << WGM00) | (1 << COM0A1);	
	TCCR0B |= (1 << CS02);		//prescaler clk/256 should be equivalent to 61 Hz refresh rate on each led
	TIMSK0 |= (1 << TOIE0);		//Turning on interrupts
	OCR0B = 0;
	
		//Timer1 for second row
	TCCR1A |= (1 << COM1B1) | (1 << COM1B0) | (1 << WGM10);
	//TCCR1A |= (1 << COM1B1) | (1 << WGM10);
	TCCR1B |= (1 << WGM12) | (1 << CS12);
	//TIMSK1 |= (1 << TOIE1);		//Turning on interrupts
	OCR1BL = 0;
	
	GTCCR &= ~(1 << TSM);
}

uint16_t LedMatrixEffect(uint16_t effectNum, uint16_t effectModifier, pressedButton_t* buttonStatus)
{
	static uint8_t delta = 1;
	static uint8_t maxBrightness = 255;
	static uint8_t minBrightness = 0;
	static uint8_t targetBrightness [2][4] = {{0,0,0,0},
											  {0,0,0,0}};
	static uint32_t pseudoRandom = 1;
	
	uint8_t activeLed = 0;
	uint8_t Custombrightness [2][4] = {{40,80,120,160},
									   {200,240,255,0}};
		
	//Effect modifier switch
	switch(effectModifier)
	{
		case KEY_BRIGHTNESS_UP:
			if (maxBrightness == 255)break;
			maxBrightness += 51;
		break;
		
		case KEY_BRIGHTNESS_DOWN:
			if (maxBrightness == 0)break;
			maxBrightness -= 51;
		break;
		case KEY_RESERVED:
		break;
	}
		
	switch(effectNum)
	{
		case KEY_EFFECT1:		//All off
			CounterReset();
			for (int i = 0;i < LED_COLUMN_SIZE;i++)
			{
				for (int x = 0;x < LED_ROW_SIZE;x++)
				{
					brightness[x][i] = 0x00;
				}
			}
		break;
		
		case KEY_EFFECT2:		//All on with same brightness
			CounterReset();
			for (int i = 0;i < LED_COLUMN_SIZE;i++)
			{
				for (int x = 0;x < LED_ROW_SIZE;x++)
				{
					brightness[x][i] = maxBrightness;
				}
			}
		break;
		
		case KEY_EFFECT3:		//Breathing from min to max
			if (Counter() > 3)
			{
				CounterReset();
				if(brightness[0][0] <= minBrightness || brightness[0][0] >= maxBrightness) delta = -delta;
				brightness[0][0] += delta;
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
			while(buttonStatus[activeLed].duration != 0)
			{
				brightness[buttonStatus[activeLed].row][buttonStatus[activeLed].column] = maxBrightness;
				activeLed++;
			}
			if (Counter() > 3)
			{
				CounterReset();
				for (int i = 0;i < LED_COLUMN_SIZE;i++)
				{
					for (int x = 0;x < LED_ROW_SIZE;x++)
					{
						if (brightness[x][i] > 0)--brightness[x][i];
					}
				}
			}
		break;
		
		case KEY_EFFECT5:		//Custom brightness levels
			CounterReset();
			for (int x = 0;x < LED_ROW_SIZE;x++)
			{
				for (int i = 0;i < LED_COLUMN_SIZE;i++)
				{
					brightness[x][i] = Custombrightness[x][i];
				}
			}
		break;
		case KEY_EFFECT6:		//Random
		if (Counter() > 2)
		{
			CounterReset();
			for (int x = 0;x < LED_ROW_SIZE;x++)
			{
				for (int i = 0;i < LED_COLUMN_SIZE;i++)
				{
					pseudoRandom = (pseudoRandom * 15648891 + 534486) & 0xFFFFFFFF;
					if(brightness[x][i] == targetBrightness[x][i])targetBrightness[x][i] = (uint8_t)(pseudoRandom >> 16)%(maxBrightness/3);
					if(brightness[x][i] > targetBrightness[x][i])brightness[x][i]--;
					else brightness[x][i]++;
				}
			}
		}
		break;
		case USB_CONNECT:
			
		break;
		
		case USB_DISCONNECT:
			
		break;
		
		case USB_ERROR:
			
		break;
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			for (int i = 0;i < LED_COLUMN_SIZE;i++)
			{
				for (int x = 0;x < LED_ROW_SIZE;x++)
				{
					if (brightness[x][i] > 0)--brightness[x][i];
				}
			}
			brightness[effectNum >= 4 ? 1 : 0][effectNum%4] = maxBrightness;
		break;
		case KEY_RESERVED:
		break;
		//default:
	}
	return effectNum;
}

	//Refreshing of leds
ISR(TIMER0_OVF_vect){
	TCNT1L = 0xff;
	Counting();
	LedRefresh();
}

/*
	There is a problem that the OCR0x is being set on bottom in to the register. That means the brightness level is being set to the 
	OCR0x register for the next column. Result is everything is offset by one column. The fix is to move the currently written column
	by LED_COLUMN_SIZE - 1 so basically move back one column. (Cant move back by -1 cos for some reason it wont do modulo of negative number)
*/
void LedRefresh(void){
	static int activeColumn = 0;
	PORTD &= ~(1 << pins.ledCol[(activeColumn + LED_COLUMN_SIZE + 2) % LED_COLUMN_SIZE]);
	OCR0B = brightness[0][activeColumn];
	OCR1BL = brightness[1][activeColumn];
	PORTD |= (1 << pins.ledCol[(activeColumn + LED_COLUMN_SIZE - 1) % LED_COLUMN_SIZE]);
	activeColumn = (activeColumn + 1) % LED_COLUMN_SIZE;
}
