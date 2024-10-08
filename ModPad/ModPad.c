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
	//Includes:
		#include "ModPad.h"
	
	//Variables:
		Array_t keyMap;
		pressedButton_t* buttonStatus;
		module modules[3];

		uint8_t eventEffect = 0;
		uint16_t effectNum = 0;
		uint16_t effectModifier = 0;

	//EEPROM variables:
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

// Buffer to hold the previously generated Keyboard report, for comparison purposes inside the HID class driver.
static uint8_t PrevKeyboardReportBuffer[MAX(sizeof(USB_ConsumerReport_Data_t), sizeof(USB_KeyReport_Data_t))];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Keyboard_HID_Interface =
{
	.Config =
	{
		.InterfaceNumber              = INTERFACE_ID_Keyboard,
		.ReportINEndpoint             =
		{
			.Address              = KEYBOARD_IN_EPADDR,
			.Size                 = HID_EPSIZE,
			.Banks                = 1,
		},
		.PrevReportINBuffer           = PrevKeyboardReportBuffer,
		.PrevReportINBufferSize       = sizeof(PrevKeyboardReportBuffer),
	},
};


// Buffer to hold the previously generated Slider report, for comparison purposes inside the HID class driver.
static uint8_t PrevSliderReportBuffer[sizeof(USB_SliderReport_Data_t)];

USB_ClassInfo_HID_Device_t Slider_HID_Interface =
{
	.Config =
	{
		.InterfaceNumber              = INTERFACE_ID_Slider,
		.ReportINEndpoint             =
		{
			.Address              = SLIDER_IN_EPADDR,
			.Size                 = HID_EPSIZE,
			.Banks                = 1,
		},
		.PrevReportINBuffer           = PrevSliderReportBuffer,
		.PrevReportINBufferSize       = sizeof(PrevSliderReportBuffer),
	},
};


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{
		//Jumping to bootloader if profile button is pressed during startup or restart
	if (!(PINC & (1 << 4)))
	{
		cli();
			// Enable change of Interrupt Vectors
		MCUCR = (1<<IVCE);
			// Move interrupts to Boot Flash section
		MCUCR = (1<<IVSEL);
			// Jump to start of Boot Flash section
		asm volatile ("\tjmp 0x3000\n");
		while (1);
	}
	
	SetupHardware();
		//Initialization of main and global values
	buttonStatus = ReadKey();
	keyMap = getKeyMap(eeprom_read_byte(&eepromKeyProfile));
	eventEffect = eeprom_read_byte(&eepromKeyProfile) + 1;
	bool effectStoper = 0;
	
	GlobalInterruptEnable();

	for (;;)
	{
			//Effect and modifier are changed only once during button press
		if(buttonStatus[0].duration != 0 && effectStoper == 0)
		{
			effectStoper = 1;
			if (keyMap.profiles[buttonStatus[0].row][buttonStatus[0].column] < KEY_RESERVED2 && keyMap.profiles[buttonStatus[0].row][buttonStatus[0].column] > KEY_RESERVED)
			{
				if (keyMap.profiles[buttonStatus[0].row][buttonStatus[0].column] == KEY_NEXT_EFFECT)effectNum = ((effectNum - 0x101+1)%8)+0x101;
				else if (keyMap.profiles[buttonStatus[0].row][buttonStatus[0].column] == KEY_PREV_EFFECT)effectNum = ((effectNum - 0x101-1)%8)+0x101;
				else effectNum = keyMap.profiles[buttonStatus[0].row][buttonStatus[0].column];
			}
			else if (keyMap.profiles[buttonStatus[0].row][buttonStatus[0].column] > KEY_RESERVED2)
			{
				effectModifier = keyMap.profiles[buttonStatus[0].row][buttonStatus[0].column];
			}
		}
		if (buttonStatus[0].duration == 0) effectStoper = 0;	//Enabling the effect and modifier change after key is no longer pressed 
		if (eventEffect)
		{
			LedMatrixEvent(eventEffect);
			if (Counter(1) > 0xFF) eventEffect = 0x00;
		}
		else 
		{
			LedMatrixEffect(effectNum, buttonStatus);
			if (effectModifier)
			{
				LedMatrixModifier(effectModifier);
				effectModifier = 0;
			}
		}
		HID_Device_USBTask(&Keyboard_HID_Interface);
		HID_Device_USBTask(&Slider_HID_Interface); 
		USB_USBTask();
		//This should check how many times a poll for slider interface was initiated and then get data over SPI from all modules. But it doesn't work. Polling is set to 20 ms and 
		//even with 150 * 20 ms the sliders are really responsive. Only explanation is that the function CALLBACK_HID_Device_CreateHIDReport is called more often than 20 ms with slider
		//interface arguments.
		if (Counter(2) > 150)
		{
			SPIGetData(modules);
			CounterReset(2);
		}
	}
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware()
{
#if (ARCH == ARCH_AVR8)
	/* Disable watchdog if enabled by bootloader/fuses */
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	/* Disable clock division */
	//clock_prescale_set(clock_div_1); Bug in power.h that it's not taking the funciton or define when mcu is set to atmega16u2
	//The work around is to write directly to registers:
	CLKPR = (1 << CLKPCE);
	CLKPR = (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
	
#elif (ARCH == ARCH_XMEGA)
	/* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
	XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
	XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

	/* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
	XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
	XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

	/* Hardware Initialization */
	effectNum = LedMatrixInit();
	MatrixInit();
	SPIInit();
	USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	eventEffect = USB_CONNECT;
	CounterReset(1);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	eventEffect = USB_DISCONNECT;
	CounterReset(1);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Keyboard_HID_Interface);
	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Slider_HID_Interface);

	USB_Device_EnableSOFEvents();
	eventEffect = ConfigSuccess ? USB_CONNECT : USB_ERROR;
	CounterReset(1);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Keyboard_HID_Interface);
	HID_Device_ProcessControlRequest(&Slider_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Keyboard_HID_Interface);
	HID_Device_MillisecondElapsed(&Slider_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.
 *
 *  \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
 *  \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
 *  \param[out]    ReportData  Pointer to a buffer where the created report should be stored
 *  \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)
 *
 *  \return Boolean \c true to force the sending of the report, \c false to let the library determine if it needs to be sent
 */
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize)
{
	
	if (HIDInterfaceInfo == &Keyboard_HID_Interface)
	{
		uint8_t UsedKeyCodes = 0;
		UsedKeyCodes = 0;
		buttonStatus = ReadKey();
	
			//Getting the right keyMap based on how many times was button pressed
		if (ButtonPress())
		{
			uint8_t keyProfile = eeprom_read_byte(&eepromKeyProfile);
			keyProfile = (keyProfile + 1) % PROFILES;
			keyMap = getKeyMap(keyProfile);
			eeprom_write_byte(&eepromKeyProfile, keyProfile);
			CounterReset(1);	
		}
			//Key codes are separated in to global and consumer ones. Each one has its own report
		if (keyMap.profiles[buttonStatus[UsedKeyCodes].row][buttonStatus[UsedKeyCodes].column] <= 0xAF)
		{
			USB_KeyReport_Data_t* KeyboardReport = (USB_KeyReport_Data_t*)ReportData;
			*ReportSize = sizeof(USB_KeyReport_Data_t);
			*ReportID   = HID_REPORTID_KeyboardReport;
			while (buttonStatus[UsedKeyCodes].duration != 0)
			{
				KeyboardReport->KeyCode[UsedKeyCodes] = (uint8_t)keyMap.profiles[buttonStatus[UsedKeyCodes].row][buttonStatus[UsedKeyCodes].column];
				UsedKeyCodes++;
			}
		
		}
		else if (keyMap.profiles[buttonStatus[UsedKeyCodes].row][buttonStatus[UsedKeyCodes].column] <= 0xFF)
		{
			USB_ConsumerReport_Data_t* ConsumerReport = (USB_ConsumerReport_Data_t*)ReportData;
			*ReportSize = sizeof(USB_ConsumerReport_Data_t);
			*ReportID   = HID_REPORTID_ConsumerReport;
			if(buttonStatus[0].duration != 0)
			{
				ConsumerReport->ConsumerKey = (uint8_t)keyMap.profiles[buttonStatus[0].row][buttonStatus[0].column];
			}
		}
	}
	else if (HIDInterfaceInfo == &Slider_HID_Interface)
	{
		USB_SliderReport_Data_t* SliderReport = (USB_SliderReport_Data_t*)ReportData;
		*ReportSize = sizeof(USB_SliderReport_Data_t);
		for (int i = 0; i < NUM_OF_MODULES; i++)
		{
			if (modules[i].ID == 0x01)
			{
				for (int x = 0; x < 3; x++)SliderReport->Value[x] = modules[i].data[x];
			}
		}
		Counting(2);
	}
	return false;
}

/** HID class driver callback function for the processing of HID reports from the host.
 *
 *  \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
 *  \param[in] ReportID    Report ID of the received report from the host
 *  \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
 *  \param[in] ReportData  Pointer to a buffer where the received report has been stored
 *  \param[in] ReportSize  Size in bytes of the received HID report
 */
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
	USB_FeatureReport_Data_t* FeatureReport = (USB_FeatureReport_Data_t*)ReportData;
	switch(FeatureReport->Command)
	{
		case FEATR_EFFECT:
			if (FeatureReport->Optional[2] == 0)effectNum = FeatureReport->Value;
			else SPISendData(FEATR_EFFECT, FeatureReport->Value - 0xff, FeatureReport->Optional[2] - 1);
		break;
		case FEATR_BRIGHTNESS:
			
			if (FeatureReport->Optional[2] == 0)effectModifier = FeatureReport->Value;
			else SPISendData(FEATR_BRIGHTNESS, FeatureReport->Value - 0x1ff, FeatureReport->Optional[2] - 1);
		break;
		case FEATR_PROFILE:
			if (FeatureReport->Optional[2] == 0)
			{
				eeprom_write_byte(&eepromKeyProfile, (uint8_t)FeatureReport->Value);
				getKeyMap((uint8_t)FeatureReport->Value);
			}
			else SPISendData(FEATR_EFFECT, FeatureReport->Value, FeatureReport->Optional[2] - 1);
		break;
		case FEATR_MAPPING:
			eeprom_write_word(&eepromProfileSelect[FeatureReport->Optional[0]][FeatureReport->Optional[1] >= 4 ? 1 : 0][FeatureReport->Optional[1]%4],FeatureReport->Value);
			getKeyMap(FeatureReport->Optional[0]);
			eventEffect = FeatureReport->Optional[1];
		break;
	}	
}           

Array_t getKeyMap(uint8_t keyProfile)
{
	eventEffect = keyProfile + 1;
	CounterReset(1);
	uint16_t profileSelect[ROW_SIZE][COLUMN_SIZE];
	eeprom_read_block(profileSelect, &eepromProfileSelect[keyProfile], sizeof(profileSelect));
	for (int x = 0; x < ROW_SIZE; x++)
	{
		for (int i = 0; i < COLUMN_SIZE; i++)
		{
			keyMap.profiles[x][i] = profileSelect[x][i];
		}
	}
	return keyMap;
}