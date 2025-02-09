# MSX PICOVERSE 2350 MULTIROM SOFTWARE PLAYGROUND

This page summarizes the software options currently available or in development for the PicoVerse 2350 MultiRom cartridge. 

## Software and Tools you may need to develop for the PicoVerse 2350

* [Visual Studio Code]( https://code.visualstudio.com/) - Code editor that can be used to develop software for the PicoVerse 2040 cartridges.
* [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) - The SDK is a collection of libraries and tools that are designed to make it easy to write programs for the RP2040 microcontroller.  
* [SDCC](http://sdcc.sourceforge.net/) - Small Device C Compiler. Standard C (ANSI C89, ISO C99, ISO C11) compiler that targets the Zilog Z80 based microcontrollers (z80, z180, gbz80, Rabbit 2000/3000, Rabbit 3000A, TLCS-90) for cross-development on Windows, macOS, and Linux. 
* [Fusion-C v1.3](https://github.com/ericb59/FUSION-C-1.3) - Fusion-C is a set of development tools for the MSX system that allows you to write MSX software in C language.
* [MYSYS2](https://www.msys2.org/) - A collection of GNU utilities and open-source software that provides functionality similar to a Linux distribution on Windows. If you are using Windows to develop for the cartridge.
* [GNU Make](https://www.gnu.org/software/make/) - A tool that controls the generation of executables and other non-source files of a program from the program's source files.
* [uf2conv.exe](https://github.com/microsoft/uf2/) - Converts a binary file to a UF2 file. This is used to convert resultant binary files to a UF2 file that can be flashed to the PICO board. 

## Software

### [loadrom](loadrom/)

This is a firmware to load a single ROM image (up to 10MB) from the flash memory of the PICO 2350 board. The ROM image is read directly from the flash memory and then executed. At this moment the tool can load and execute ROMs of the following types:

* Plain16  
* Plain32  
* Linear0  
* Konami SCC  
* Konami (without SCC)  
* ASCII8  
* ASCII16 
* NEO-8 
* NEO-16

To use you need to run loadrom.exe passing the ROM file as a parameter. The tool will read the ROM file, detect the mapper used using a heuristic approach, and then create a UF2 file that can be used to program the PICO board. 
```
loadrom.exe romfile.rom
```
After the tool execution, you need to connect your cartridge to the PC via the USB-C cable with the BOOTSEL button pushed, and copy the loadrom.uf2 file to drive that will be created in your operating system. The ROM will be loaded and executed on the MSX when the cartridge is inserted to the slot.

### [multirom](multirom/)

This is an alternative firmware that allows the selection of a ROM image from a list of ROMs stored in the flash memory of the PICO 2350 board. The ROM image is read directly from the Raspberry Pi Pico flash memory and executed on the MSX. 

At this moment the tool can load and execute following ROM types:

* Plain16  
* Plain32  
* Linear0  
* Konami SCC  
* Konami (without SCC)  
* ASCII8  
* ASCII16  
* NEO-8 
* NEO-16
  
To use it you need to run the multirom.exe tool in the same folder as the ROM files. Please rename the ROM files with names with up to 20 characters in the way you want them to appear in the menu. The tool will read the ROM files, detect their type and create a UF2 file that can be used to program the PICO board. 

```
multirom.exe
```
Then connect your cartridge to the PC via the USB-C cable and copy the multirom.uf2 file to the PICO board. The menu will be executed and you can select the ROM to be loaded to the MSX.
