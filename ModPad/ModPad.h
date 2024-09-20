/*
             LUFA Library
     Copyright (C) Dean Camera, 2021.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2021  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#ifndef _MODPAD_H_
#define _MODPAD_H_

	//Includes:
		#include <avr/io.h>	
		#include <avr/wdt.h>
		#include <avr/power.h>
		#include <avr/interrupt.h>
		#include <avr/eeprom.h>
		#include <stdbool.h>
		#include <string.h>

		#include "Descriptors.h"
		#include "ButtonMatrix.h"
		#include "LedMatrix.h"
		#include "SPIcom.h"
		
		#include <Includes/usb_hid_keys.h>
		#include <Includes/Delay.h>
		
		
		#include <lufa/LUFA/Drivers/USB/USB.h>
		#include <lufa/LUFA/Platform/Platform.h>
		
	//Macros:
		//Number of profiles 
		#define PROFILES 4
		
		//Commands for feature report
		#define FEATR_EFFECT 0x01
		#define FEATR_BRIGHTNESS 0x02
		#define FEATR_PROFILE 0x03
		#define FEATR_MAPPING 0x04
		//todo
		#define FEATR_PING 0x05
		#define FEATR_ACTIVE_PROFILES 0x06
			

		
	//Variables:
		typedef struct
		{
			uint16_t Command;
			uint16_t Value;
			uint8_t Optional[3];	// {Profile, Key}
		} ATTR_PACKED USB_FeatureReport_Data_t;
		
		typedef struct
		{
			uint8_t ConsumerKey;
		} ATTR_PACKED USB_ConsumerReport_Data_t;

		typedef struct
		{
			uint8_t Modifier;
			uint8_t KeyCode[3];
		} ATTR_PACKED USB_KeyReport_Data_t;
		
		typedef struct
		{
			uint8_t Value[3];
		} ATTR_PACKED USB_SliderReport_Data_t;
		
		typedef struct{
			uint16_t profiles[ROW_SIZE][COLUMN_SIZE];
		}Array_t;
	
		Array_t keyMap;
		pressedButton_t* buttonStatus;
		module modules[3];

		uint8_t eventEffect = 0;
		uint16_t effectNum = 0;
		uint16_t effectModifier = KEY_RESERVED;
		
	//EEPROM variables:
		uint16_t EEMEM eepromEffectNum = 0x101;
		uint8_t EEMEM eepromKeyProfile = 0;
		uint16_t EEMEM eepromProfileSelect[PROFILES][ROW_SIZE][COLUMN_SIZE] = {
			{
				{KEY_F13, KEY_F14, KEY_F15, KEY_F16},		//F13, F14, F15, F16
				{KEY_F17, KEY_F18, KEY_F19, KEY_F20}		//F17, F18, F19, F20
			},
			{
				{0x04, 0x05, 0x06, 0x07},		//a, b, c, d
				{0x08, 0x09, 0x0a, 0x0b}		//e, f, g, h
			},
			{
				{KEY_PREV_EFFECT, KEY_NEXT_EFFECT, KEY_BRIGHTNESS_UP, KEY_BRIGHTNESS_DOWN},
				{KEY_EFFECT1, KEY_EFFECT2, KEY_EFFECT3, KEY_EFFECT6}
			},
			{
				{KEY_MEDIA_PLAYPAUSE, KEY_MEDIA_MUTE, KEY_MEDIA_VOLUMEUP, KEY_PAUSE},
				{KEY_MEDIA_PREVIOUSSONG, KEY_MEDIA_NEXTSONG, KEY_MEDIA_VOLUMEDOWN, KEY_SCROLLLOCK}
			},
		};
		
	//Function Prototypes:
		void SetupHardware(void);

		void EVENT_USB_Device_Connect(void);
		void EVENT_USB_Device_Disconnect(void);
		void EVENT_USB_Device_ConfigurationChanged(void);
		void EVENT_USB_Device_ControlRequest(void);
		void EVENT_USB_Device_StartOfFrame(void);

		bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		                                         uint8_t* const ReportID,
		                                         const uint8_t ReportType,
		                                         void* ReportData,
		                                         uint16_t* const ReportSize);
		void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
		                                          const uint8_t ReportID,
		                                          const uint8_t ReportType,
		                                          const void* ReportData,
		                                          const uint16_t ReportSize);
												  
		Array_t getKeyMap(uint8_t keyMode);
		//uint16_t EffectChange(uint16_t Effect);
		//uint16_t ModifierChange(uint16_t Modifier);
#endif /* MODPAD_H_ */

