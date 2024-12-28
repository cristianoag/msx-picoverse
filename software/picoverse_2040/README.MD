# MSX PICOVERSE SOFTWARE PLAYGROUND

This is a collection of software for the MSX PicoVerse project. The software is being developed for the RP2040 and RP2350 based cartridges.

## Tools you may need

* [uf2conv.exe](https://github.com/microsoft/uf2/) - Converts a binary file to a UF2 file. This is used to convert resultant binary files to a UF2 file that can be flashed to the PICO board. 

## testflash1

This is small test program that demonstrates how to read the contents of a ROM image that is appended to the end of the main program in the PICO flash memory. The ROM data is read from the flash memory and dumped to the serial console in hexdump format.

To use it you need to concatenate the ROM to the resultant bin file for the testflash1. Use the following commands to prepare the UF2 file:

```
copy /b testflash1.bin+romfile.rom testflash1-rom.bin
uf2conv.exe testflash1-rom.bin testflash1-rom.uf2
```

## loadrom

This is a program to load a ROM image from the flash memory of the PICO board. The ROM image is read from the flash memory and copied to the RAM of the PICO board. The ROM image is then executed. At this moment only 16KB or 32KB ROMs plain ROMs are supported.

```
copy /b loadrom.bin+romfile.rom loadrom.bin
uf2conv.exe loadrom.bin loadrom.uf2
```

## flashrom

The Flashrom firmware allows loading ROMs from the Raspberry Pi Pico's flash memory. When you flash the cartridge with this firmware, a new drive will be created/appear on your PC when you connect the cartridge/pico to it via USB cable. The drive has the name PICOVERSE.

You can then push and hold the BOOTSEL button on the cartridge to format the drive and then drag and drop ROM files to it. The cartridge will then read the ROM files and make them available to the MSX computer user via a menu system. The user can then select the ROM to be loaded into the MSX computer.

![alt text](images/flashrom1.jpg)

* FatFS - http://elm-chan.org/fsw/ff/
* PicoUSB Flash Drive - https://github.com/oyama/pico-usb-flash-drive
* SPIFTL - Embedded, Static Wear-Leveling FTL Library - https://github.com/earlephilhower/SPIFTL