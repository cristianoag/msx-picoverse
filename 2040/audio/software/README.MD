# MSX PICOVERSE 2040 AUDIO SOFTWARE PLAYGROUND

This page summarizes the software options currently available or in development for the PicoVerse 2040 Audio cartridge. 

## Software and Tools you may need to develop for the PicoVerse 2040

* [Visual Studio Code]( https://code.visualstudio.com/) - Code editor that can be used to develop software for the PicoVerse 2040 cartridges.
* [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) - The SDK is a collection of libraries and tools that are designed to make it easy to write programs for the RP2040 microcontroller.  
* [SDCC](http://sdcc.sourceforge.net/) - Small Device C Compiler. Standard C (ANSI C89, ISO C99, ISO C11) compiler that targets the Zilog Z80 based microcontrollers (z80, z180, gbz80, Rabbit 2000/3000, Rabbit 3000A, TLCS-90) for cross-development on Windows, macOS, and Linux. 
* [Fusion-C v1.3](https://github.com/ericb59/FUSION-C-1.3) - Fusion-C is a set of development tools for the MSX system that allows you to write MSX software in C language.
* [MYSYS2](https://www.msys2.org/) - A collection of GNU utilities and open-source software that provides functionality similar to a Linux distribution on Windows. If you are using Windows to develop for the cartridge.
* [GNU Make](https://www.gnu.org/software/make/) - A tool that controls the generation of executables and other non-source files of a program from the program's source files.
* [uf2conv.exe](https://github.com/microsoft/uf2/) - Converts a binary file to a UF2 file. This is used to convert resultant binary files to a UF2 file that can be flashed to the PICO board. 

## Software

### [musicplayer](musicplayer/)

This is a simple music player that plays music files stored in the flash memory of the PICO board. The music files are stored in the flash memory and are played using the UDA1334A I2S Stereo DAC. The music player supports the following file formats:

* MP3
* WAV