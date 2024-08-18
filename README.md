# ModPad
Goal of this project is a fully customizable macropad capable of connecting other modules for additional controls. ModPad is powered by [ATmega16u2](https://ww1.microchip.com/downloads/en/DeviceDoc/doc7799.pdf) running a custom firmware for controlling [LEDs](https://github.com/Kody420/MacroPad/blob/main/USBkeyboard/LedMatrix.c) and [keys](https://github.com/Kody420/MacroPad/blob/main/USBkeyboard/ButtonMatrix.c). [LUFA](https://github.com/abcminiuser/lufa) is used for handling USB communication on avr. For test sending feature reports to the device I use [hidapitester](https://github.com/todbot/hidapitester/tree/main?tab=readme-ov-file#usage).
![ModPad IRL](https://github.com/Kody420/MacroPad/blob/main/Hardware/ModPad%201.0.png?raw=true "ModPad IRL")

## Why does this project exists?
Mainly because I wanted to do a bigger project with embedded systems and learn as I go through the development. So yeah I am not an experienced programmer or engineer. If you see something that's horribly wrong, please point it out. I would be glad to learn something new. I will try to create a kind of a "walkthrough" of how I learned to use some really useful tools/stuff that are used in this project. It's also kinda for me so that i don't forget how to use these things after a month. 
Tools and other stuff I had to learn to use for this project:
- DEBUGwire
- HID USB descriptors
- Memory structure of avrs
- Dfu-bootloader

## Usage and functions
Keys work like on a normal keyboard. So there is short press and long press for continuous sending. 
ModPad supports up to 4 profiles which are basically pages for custom key mapping. Any from the [supported key codes](https://github.com/Kody420/MacroPad/blob/main/USBkeyboard/Includes/usb_hid_keys.h) can be mapped on any profile on any key. Profiles can be changed using dedicated button or with command using hidapitester. To keep track what profile is activated corresponding LED turns on for initial moments after profile change.
Led effects use individually programmable red LEDs. Current effects:
- Off
- Max brightness
- Breathing
- Button activated fade
- Custom brightness on individual keys
- Random

Effect can be changed using dedicated key codes which can be mapped to key or sent via hidapitester. Every effect's brightness is scaled using `maxBrightness` that can be changed the same way as effects.

## Sending commands to device
ModPad is capable of receiving feature reports from host to set some settings. Currently I am using [hidapitester](https://github.com/todbot/hidapitester/tree/main) to send reports. But there is a plan to create a custom application for that with my friend [M4rc311o](https://github.com/M4rc311o). The command for sending feature report looks like this:

    .\hidapitester.exe --vidpid 03EB/2066 --usagePage 0xFF --open --length 8 --send-feature 03, 7 bytes to send

### Explanation of arguments
For all command that hidapitester uses go [here](https://github.com/todbot/hidapitester/tree/main?tab=readme-ov-file#usage).

- `--vidpid` VendorID *03EB* and productID *2066* are identification numbers for the device. This vid and pid is taken from the LUFA keyboard demo and will be changed.
- `--usagePage` To not interfere with any USB key codes a vendor defined usage page *0xFF* is used.
- `--length` ReportID byte which is *03* and 7 data bytes.
- `--send-feature` Here is the place for all the bytes that are going to be sent. Each byte is separated by comma. The structure is as follows:

    | Byte | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
    |:--:| :--: | :--: | :--: | :--: | :--: | :--: | :--: | :--: |
    | Data | ReportID | CommandL | CommandH | ValueL | ValueH | Profile | Row | Column |

    - `Command` and `Value` are 16-bit variables. The hidapitester uses little endian for 16-bit variables. That means value 0x0103 has to be sent like 0x03,0x01.

    | Define | Command | Value | Description |
    | -------- | -------- | -------- | -------- |
    | FEATR_EFFECT | 0x01 | 0x101 - 0x109 | Changing LED effect |
    | FEATR_BRIGHTNESS | 0x02 | 0x20a, 0x20b | Increase/decrease max brightness |
    | FEATR_PROFILE | 0x03 | 0x00 - 0x03 | Changing active profile |
    | FEATR_MAPPING | 0x04 | [key codes](https://github.com/Kody420/MacroPad/blob/main/USBkeyboard/Includes/usb_hid_keys.h) | Uses last 3 bytes - profile, row, column |


## Modules
The plan is to have different modules that will connect on the sides of ModPad with pogo pins. Communication will be done through SPI. Modules that came to mind are:
- Sliders
- Display
- Additional keys
