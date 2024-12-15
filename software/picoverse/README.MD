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

This is a program to load a ROM image from the flash memory of the PICO board. The ROM image is read from the flash memory and copied to the RAM of the PICO board. The ROM image is then executed. At this moment only 16Kb or 32Kb ROMs are supported.

```
copy /b loadrom.bin+romfile.rom loadrom.bin
uf2conv.exe loadrom.bin loadrom.uf2
```