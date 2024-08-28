# ModPad
Goal of this project is a fully customizable macropad capable of connecting other modules for additional controls. ModPad is powered by [ATmega16u2](https://ww1.microchip.com/downloads/en/DeviceDoc/doc7799.pdf) running a custom firmware for controlling [LEDs](ModPad/LedMatrix.c) and [keys](ModPad/ButtonMatrix.c). [LUFA](https://github.com/abcminiuser/lufa) is handling USB communication on avr. For sending feature reports to the device [modpadctrl](https://github.com/M4rc311o/modpadctrl) application is used .
![ModPad IRL](Hardware/ModPad%201.0.png?raw=true "ModPad IRL")

## Why does this project exists?
Mainly because I wanted to do a bigger project with embedded systems and learn as I go through the development. So yeah I am not an experienced programmer or engineer. If you see something that's horribly wrong, please point it out. I would be glad to learn something new. I will try to create a kind of a "walkthrough" of how I learned to use some really useful tools/stuff that are used in this project. It's also kinda for me so that i don't forget how to use these things after a month. 
Tools and other stuff I had to learn to use for this project:
- DEBUGwire
- HID USB descriptors
- Memory structure of avrs
- Dfu-bootloader

## Usage and functions
Keys work like on a normal keyboard. So there is short press and long press for continuous sending. 
ModPad supports up to 4 profiles which are basically pages for custom key mapping. Any from the [supported key codes](ModPad/Includes/usb_hid_keys.h) can be mapped on any profile on any key. Profiles can be changed using dedicated button or by sending feature report. To keep track what profile is activated corresponding LED turns on for initial moments after profile change.
Led effects use individually programmable red LEDs. Current effects:
- Off
- Max brightness
- Breathing
- Button activated fade
- Custom brightness on individual keys
- Random

Effect can be changed using dedicated key codes which can be mapped to key or sent via feature report. Every effect's brightness is scaled using `maxBrightness` that can be changed the same way as effects.

## Sending commands to device
ModPad is capable of receiving feature reports from host to set some settings. My friend wrote a [modpadctrl](https://github.com/M4rc311o/modpadctrl) which is CLI application made for ModPad. It's also possible to send raw feature report to the device with application like [hidapitester](https://github.com/todbot/hidapitester). Here is the feature report structure:

| Byte | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
|:--:| :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
| Data | ReportID | CommandL | CommandH | ValueL | ValueH | Optional1 | Optional2 | Reserved |

`Command` and `Value` are 16-bit variables. The report uses little endian for 16-bit variables. That means value 0x0103 has to be sent like 0x03,0x01.

| Define | Command | Value | Optional1* | Optional2* |Description |
| -------- | -------- | -------- | :--------: | :--------: |-------- |
| FEATR_EFFECT | 0x01 | 0x101 - 0x109 | - | - |Changing LED effect |
| FEATR_BRIGHTNESS | 0x02 | 0x20a, 0x20b | - | - |Increase/decrease max brightness |
| FEATR_PROFILE | 0x03 | 0x00 - 0x03 | - | - |Changing active profile |
| FEATR_MAPPING | 0x04 |[key codes](ModPad/Includes/usb_hid_keys.h) | profile: 0x00 - 0x03 | key: 0x00 - 0x07 | Remapping a profile |

 *The `-` indicates that the command is not using this byte, and should be set to 0x00.
## Modules
The plan is to have different modules that will connect on the sides of ModPad with pogo pins. Communication will be done through SPI. Modules that came to mind are:
- Sliders
- Display
- Additional keys
