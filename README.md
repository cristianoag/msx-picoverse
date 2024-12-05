# msx-picoverse
MSX PicoVerse - The MSX experience driven by the RaspBerry Pico

The MSX PicoVerse is an open-source initiative to implement multi-function cartridges for 
the MSX line of computers using variations of the Raspberry Pico development boards or the
RP2040/2350 integrated chips.

It is a project that aims to bring the MSX experience to the next level, with the possibility 
to load ROMs, emulate, or even create new hardware by using a software defined approach.

## Hardware

The available versions for the board are divided into two categories: 

### Boards based on the RP2040 chip

The RP2040 is a dual-core ARM Cortex-M0+ microcontroller with 264KB of SRAM and 2MB (or 16MB) 
of Flash memory. The chip offers a wide range of features, including 30 GPIO pins, 2 SPI 
controllers, 2 I2C controllers, 2 UART controllers, 3 12-bit ADCs, 16 PWM channels, 2 
USB controllers, and a variety of other interfaces.

The PicoVerse 2040 cartridges are based on development boards that expose 30 GPIO pins, and *WILL NOT
WORK* with conventional Raspberry Pi Pico dev boards that usually expose only 26 GPIO pins. At 
this moment the following boards are supported:

* [OLIMEX RP2040-PICO30](https://www.olimex.com/Products/MicroPython/RP2040-PICO30/open-source-hardware)
* [OLIMEX RP2040-PICO30-16](https://www.olimex.com/Products/MicroPython/RP2040-PICO30/open-source-hardware)

The OLIMEX boards are also open-source and can be built from the provided files available in the
GitHub at https://github.com/OLIMEX/RP2040-PICO30

### Boards based on the RP2350 chip

The RP2350 is a high-performance microcontroller developed by Raspberry Pi Ltd., introduced
in August 2024 as the successor to the RP2040. It features a unique dual-core, dual-architecture 
design, allowing to select between two Arm Cortex-M33 cores and two open-hardware Hazard3 
RISC-V cores, operating at up to 150 MHz. 

Key specifications include 520 KB of on-chip SRAM divided into ten independent banks, 
support for up to 16 MB of external QSPI flash or PSRAM, and a comprehensive set of 
features:

* Communication Interfaces: 2× UART, 2× SPI controllers, 2× I²C controllers.
* Analog and Digital I/O: 24 PWM channels, 4 or 8 ADC channels (depending on the package), and up to 48 GPIO pins.
* USB Support: USB 1.1 controller with host and device capabilities.
* Programmable I/O: 12 PIO state machines for flexible interfacing. 

The PicoVerse 2350 cartridges are based on development boards that expose 48 GPIO pins, and *WILL NOT
WORK* with conventional Raspberry Pi Pico2 dev boards that usually expose only 26 GPIO pins. At 
this moment the following boards are supported:

* [Pimoroni PGA2350](https://shop.pimoroni.com/products/pga2350?variant=42092629229651)

## Software

The software for the PicoVerse cartridges is being developed. The software will be fully 
open-sourced. The carts are also compatible with the MSX&#960; project.
## License 

![Open Hardware](images/ccans.png)

This work is licensed under a [Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License](http://creativecommons.org/licenses/by-nc-sa/4.0/).

* If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original.
* You may not use the material for commercial purposes.
* You must give appropriate credit, provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but not in any way that suggests the licensor endorses you or your use.

**ATTENTION**

This project was made for the retro community and not for commercial purposes. So only retro hardware forums and individual people can build this project.

THE SALE OF ANY PART OF THIS PROJECT WITHOUT EXPRESS AUTHORIZATION IS PROHIBITED!

