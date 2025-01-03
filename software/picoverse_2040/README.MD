# MSX PICOVERSE 2040 SOFTWARE PLAYGROUND

This page summarizes the software options currently available or in development for the PicoVerse 2040 cartridges. The software will be fully open-sourced and available for download in this repository.

## Software and Tools you may need to develop for the PicoVerse 2040

* [Visual Studio Code]( https://code.visualstudio.com/) - Code editor that can be used to develop software for the PicoVerse 2040 cartridges.
* [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) - The SDK is a collection of libraries and tools that are designed to make it easy to write programs for the RP2040 microcontroller.  
* [SDCC v3.9](http://sdcc.sourceforge.net/) - Small Device C Compiler. Standard C (ANSI C89, ISO C99, ISO C11) compiler that targets the Zilog Z80 based microcontrollers (z80, z180, gbz80, Rabbit 2000/3000, Rabbit 3000A, TLCS-90) for cross-development on Windows, macOS, and Linux. 
* [Fusion-C v1.3](https://github.com/ericb59/FUSION-C-1.3) - Fusion-C is a set of development tools for the MSX system that allows you to write MSX software in C language.
* [MYSYS2](https://www.msys2.org/) - A collection of GNU utilities and open-source software that provides functionality similar to a Linux distribution on Windows. If you are using Windows to develop for the cartridge.
* [GNU Make](https://www.gnu.org/software/make/) - A tool that controls the generation of executables and other non-source files of a program from the program's source files.
* [uf2conv.exe](https://github.com/microsoft/uf2/) - Converts a binary file to a UF2 file. This is used to convert resultant binary files to a UF2 file that can be flashed to the PICO board. 

## Software

### [testflash1](testflash1/)

This is small test program that demonstrates how to read the content of a ROM image that is appended to the end of the main program in the PICO flash memory. The ROM data is read from the flash memory and dumped to the serial console in hexdump format.

To use it you need to concatenate the ROM to the resultant bin file for the testflash1. Use the following commands to prepare the UF2 file:

```
copy /b testflash1.bin+romfile.rom testflash1-rom.bin
uf2conv.exe testflash1-rom.bin testflash1-rom.uf2
```

### [loadrom](loadrom/dist/)

This is a program to load a single non-megaROM ROM image from the flash memory of the PICO board. The ROM image is read from the flash memory and copied to the RAM of the PICO board. The ROM image is then executed. At this moment the tool can load and execute following ROM types:

* Plain 16KB ROMs
* Plain 32KB ROMs
* Linear0 48KB ROMs

To use it you need to concatenate the ROM to the resultant bin file for the loadrom. Use the following commands to prepare the UF2 file:

```
copy /b loadrom.bin+romfile.rom loadrom.bin
uf2conv.exe loadrom.bin loadrom.uf2
```

I have provided a BAT file that can be used to concatenate the ROM to the resultant bin file, and generate the UF2 file to program the PICO. The BAT file is named `build.bat` and you can use it as follows:

```
build.bat romfile.rom
```
It will also copy the UF2 file to the PICO drive if it is connected to the PC.

### [multirom](multirom/)

This is an alternative that allows the user to select a ROM image from a list of ROMs stored in the flash memory of the PICO board. The ROM image is read from the flash memory and copied to the RAM of the PICO board. The ROM image is then executed on the MSX. 

At this moment the tool can load and execute following ROM types:

* Plain 16KB ROMs
* Plain 32KB ROMs
* Linear0 48KB ROMs

```
In development
```

### [flashrom](flashrom/)

The Flashrom firmware allows loading ROMs from the Raspberry Pi Pico's flash memory. When you flash the cartridge with this firmware, a new drive will be created/appear on your PC when you connect the cartridge/pico to it via USB cable. The drive has the name PICOVERSE.

You can then push and hold the BOOTSEL button on the cartridge to format the drive and then drag and drop ROM files to it. The cartridge will then read the ROM files and make them available to the MSX computer user via a menu system. The user can then select the ROM to be loaded into the MSX computer.

![alt text](/images/flashrom1.jpg)

* FatFS - http://elm-chan.org/fsw/ff/
* PicoUSB Flash Drive - https://github.com/oyama/pico-usb-flash-drive
* SPIFTL - Embedded, Static Wear-Leveling FTL Library - https://github.com/earlephilhower/SPIFTL

```
In development
```