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

#include "ModPad.h"

// Buffer to hold the previously generated Keyboard HID report, for comparison purposes inside the HID class driver.
static uint8_t PrevHIDReportBuffer[MAX(sizeof(USB_ConsumerReport_Data_t), sizeof(USB_KeyReport_Data_t))];

/** LUFA HID Class driver interface configuration and state information. This structure is
 *  passed to all HID Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_HID_Device_t Device_HID_Interface =
{
	.Config =
	{
		.InterfaceNumber              = INTERFACE_ID_MacroPad,
		.ReportINEndpoint             =
		{
			.Address              = HID_IN_EPADDR,
			.Size                 = HID_EPSIZE,
			.Banks                = 1,
		},
		.PrevReportINBuffer           = PrevHIDReportBuffer,
		.PrevReportINBufferSize       = sizeof(PrevHIDReportBuffer),
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
	eventEffect = eeprom_read_byte(&eepromKeyProfile);
	effectNum = eeprom_read_word(&eepromEffectNum);
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
				eeprom_write_word(&eepromEffectNum, effectNum);
			}
			else if (keyMap.profiles[buttonStatus[0].row][buttonStatus[0].column] > KEY_RESERVED2)
			{
				effectModifier = keyMap.profiles[buttonStatus[0].row][buttonStatus[0].column];
			}
		}
		if (buttonStatus[0].duration == 0) effectStoper = 0;	//Enabling the effect and modifier change after key is no longer pressed 
		if (Counter() > 0xFF) eventEffect = 0xFF;
		LedMatrixEffect(eventEffect == 0xff ? effectNum : eventEffect, effectModifier, buttonStatus);
		effectModifier = KEY_RESERVED;
		
		HID_Device_USBTask(&Device_HID_Interface);
		USB_USBTask();
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
	LedMatrixInit();
	MatrixInit();
	USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
	eventEffect = USB_CONNECT;
	CounterReset();
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
	eventEffect = USB_DISCONNECT;
	CounterReset();
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
	bool ConfigSuccess = true;

	ConfigSuccess &= HID_Device_ConfigureEndpoints(&Device_HID_Interface);

	USB_Device_EnableSOFEvents();
	eventEffect = ConfigSuccess ? USB_CONNECT : USB_ERROR;
	CounterReset();
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
	HID_Device_ProcessControlRequest(&Device_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
	HID_Device_MillisecondElapsed(&Device_HID_Interface);
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
		CounterReset();	
	}
		//Key codes are separated in to global and consumer ones. Each one has its own report
	if (keyMap.profiles[buttonStatus[UsedKeyCodes].row][buttonStatus[UsedKeyCodes].column] <= 0xB1)
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
	return false;

	//if (UsedKeyCodes)
	//KeyboardReport->Modifier = 0xae;
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
			effectNum = FeatureReport->Value;
		break;
		case FEATR_BRIGHTNESS:
			effectModifier = FeatureReport->Value;
		break;
		case FEATR_PROFILE:
			eeprom_write_byte(&eepromKeyProfile, (uint8_t)FeatureReport->Value);
			getKeyMap((uint8_t)FeatureReport->Value);
		break;
		case FEATR_MAPPING:
			eeprom_write_byte(&eepromProfileSelect[FeatureReport->Mapping[1]][FeatureReport->Mapping[2]][FeatureReport->Mapping[3]],FeatureReport->Value);
			eventEffect = FeatureReport->Mapping[3] + FeatureReport->Mapping[2] * 4;
		break;
	}
	CounterReset();	
}           

Array_t getKeyMap(uint8_t keyProfile)
{
	eventEffect = keyProfile;
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

