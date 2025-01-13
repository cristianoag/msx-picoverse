// MSX PICOVERSE PROJECT
// (c) 2024 Cristiano Goncalves
// The Retro Hacker
//
// loadrom.c - Simple ROM loader for MSX PICOVERSE project - v1.0
//
// This is  small test program that demonstrates how to load simple ROM images using the MSX PICOVERSE project. 
// You need to concatenate the ROM image to the  end of this program binary in order  to load it.
// The program will then act as a simple ROM cartridge that responds to memory read requests from the MSX.
// 
// This work is licensed  under a "Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
// License". https://creativecommons.org/licenses/by-nc-sa/4.0/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/sync.h"

//#include "hardware/structs/bus_ctrl.h"

#define MAX_MEM_SIZE        (131072+29)    // Maximum memory size
#define ROM_NAME_MAX        20          // Maximum ROM name length
#define SIZE_CONFIG_RECORD  29          // Size of the configuration record in the ROM
#define PICO_FLASH_SPI_CLKDIV 2

// -----------------------
// User-defined pin assignments for the Raspberry Pi Pico
// -----------------------
// Address lines (A0-A15) as inputs from MSX
#define PIN_A0     0 
#define PIN_A1     1
#define PIN_A2     2
#define PIN_A3     3
#define PIN_A4     4
#define PIN_A5     5
#define PIN_A6     6
#define PIN_A7     7
#define PIN_A8     8
#define PIN_A9     9
#define PIN_A10    10
#define PIN_A11    11
#define PIN_A12    12
#define PIN_A13    13
#define PIN_A14    14
#define PIN_A15    15

// Data lines (D0-D7)
#define PIN_D0     16
#define PIN_D1     17
#define PIN_D2     18
#define PIN_D3     19
#define PIN_D4     20
#define PIN_D5     21
#define PIN_D6     22
#define PIN_D7     23

// Control signals
#define PIN_RD     24   // Read strobe from MSX
#define PIN_WR     25   // Write strobe from MSX
#define PIN_IORQ   26   // IO Request line from MSX
#define PIN_SLTSL  27   // Slot Select for this cartridge slot
#define PIN_WAIT    28  // WAIT line to MSX 
#define PIN_BUSSDIR 29  // Bus direction line 

// This symbol marks the end of the main program in flash.
// The ROM data is concatenated immediately after this point.
extern unsigned char __flash_binary_end;


// Optionally copy the ROM into this SRAM buffer for faster access
static uint8_t rom_sram[MAX_MEM_SIZE];

// The ROM is concatenated right after the main program binary.
// __flash_binary_end points to the end of the program in flash memory.
const uint8_t *rom = (const uint8_t *)&__flash_binary_end;

// Read the address bus from the MSX
static inline uint16_t __not_in_flash_func(read_address_bus)(void) {
    // Return first 16 bits in the most efficient way
    return gpio_get_all() & 0x00FFFF;
}

// Read a byte from the data bus
static inline uint8_t __not_in_flash_func(read_data_bus)(void) 
{
    // Read the data bus
    return (gpio_get_all() >> 16) & 0xFF;
}

// Write a byte to the data bus
static inline void __not_in_flash_func(write_data_bus)(uint8_t data) {
    // Write the given byte to the given address
    gpio_put_masked(0xFF0000, data << 16);
}

// Set the data bus to input mode
static inline void __not_in_flash_func(set_data_bus_input)(void) {
    // Set data lines to input (high impedance)
    gpio_set_dir(PIN_D0, GPIO_IN);
    gpio_set_dir(PIN_D1, GPIO_IN);
    gpio_set_dir(PIN_D2, GPIO_IN);
    gpio_set_dir(PIN_D3, GPIO_IN);
    gpio_set_dir(PIN_D4, GPIO_IN);
    gpio_set_dir(PIN_D5, GPIO_IN);
    gpio_set_dir(PIN_D6, GPIO_IN);
    gpio_set_dir(PIN_D7, GPIO_IN);
}

// Set the data bus to output mode
static inline void __not_in_flash_func(set_data_bus_output)(void)
{
    gpio_set_dir(PIN_D0, GPIO_OUT);
    gpio_set_dir(PIN_D1, GPIO_OUT);
    gpio_set_dir(PIN_D2, GPIO_OUT);
    gpio_set_dir(PIN_D3, GPIO_OUT);
    gpio_set_dir(PIN_D4, GPIO_OUT);
    gpio_set_dir(PIN_D5, GPIO_OUT);
    gpio_set_dir(PIN_D6, GPIO_OUT);
    gpio_set_dir(PIN_D7, GPIO_OUT);
}

// Initialize GPIO pins
static inline void setup_gpio()
{
    // address pins
    gpio_init(PIN_A0);  gpio_set_dir(PIN_A0, GPIO_IN);
    gpio_init(PIN_A1);  gpio_set_dir(PIN_A1, GPIO_IN);
    gpio_init(PIN_A2);  gpio_set_dir(PIN_A2, GPIO_IN);
    gpio_init(PIN_A3);  gpio_set_dir(PIN_A3, GPIO_IN);
    gpio_init(PIN_A4);  gpio_set_dir(PIN_A4, GPIO_IN);
    gpio_init(PIN_A5);  gpio_set_dir(PIN_A5, GPIO_IN);
    gpio_init(PIN_A6);  gpio_set_dir(PIN_A6, GPIO_IN);
    gpio_init(PIN_A7);  gpio_set_dir(PIN_A7, GPIO_IN);
    gpio_init(PIN_A8);  gpio_set_dir(PIN_A8, GPIO_IN);
    gpio_init(PIN_A9);  gpio_set_dir(PIN_A9, GPIO_IN);
    gpio_init(PIN_A10); gpio_set_dir(PIN_A10, GPIO_IN);
    gpio_init(PIN_A11); gpio_set_dir(PIN_A11, GPIO_IN);
    gpio_init(PIN_A12); gpio_set_dir(PIN_A12, GPIO_IN);
    gpio_init(PIN_A13); gpio_set_dir(PIN_A13, GPIO_IN);
    gpio_init(PIN_A14); gpio_set_dir(PIN_A14, GPIO_IN);
    gpio_init(PIN_A15); gpio_set_dir(PIN_A15, GPIO_IN);

    // data pins
    gpio_init(PIN_D0); 
    gpio_init(PIN_D1); 
    gpio_init(PIN_D2); 
    gpio_init(PIN_D3); 
    gpio_init(PIN_D4); 
    gpio_init(PIN_D5); 
    gpio_init(PIN_D6);  
    gpio_init(PIN_D7); 

    // Initialize control pins as input
    gpio_init(PIN_RD); gpio_set_dir(PIN_RD, GPIO_IN);
    gpio_init(PIN_WR); gpio_set_dir(PIN_WR, GPIO_IN);
    gpio_init(PIN_IORQ); gpio_set_dir(PIN_IORQ, GPIO_IN);
    gpio_init(PIN_SLTSL); gpio_set_dir(PIN_SLTSL, GPIO_IN);
    gpio_init(PIN_BUSSDIR); gpio_set_dir(PIN_BUSSDIR, GPIO_IN);
}


// Dump the ROM data in hexdump format
// debug function
void dump_rom_sram(uint32_t size)
{
    // Dump the ROM data in hexdump format
    for (int i = 0; i < size; i += 16) {
        // Print the address offset
        printf("%08x  ", i);

        // Print 16 bytes of data in hexadecimal
        for (int j = 0; j < 16 && (i + j) < size; j++) {
            printf("%02x ", rom_sram[i + j]);
        }

        // Add spacing if the last line has fewer than 16 bytes
        for (int j = size - i; j < 16 && i + j < size; j++) {
            printf("   ");
        }

        // Print the ASCII representation of the data
        printf(" |");
        for (int j = 0; j < 16 && (i + j) < size; j++) {
            char c = rom_sram[i + j];
            if (c >= 32 && c <= 126) {
                printf("%c", c);  // Printable ASCII character
            } else {
                printf(".");      // Non-printable character
            }
        }
        printf("|\n");
    }
}

void dump_rom(uint32_t size)
{
    // Dump the ROM data in hexdump format
    for (int i = 0; i < size; i += 16) {
        // Print the address offset
        printf("%08x  ", i);

        // Print 16 bytes of data in hexadecimal
        for (int j = 0; j < 16 && (i + j) < size; j++) {
            printf("%02x ", rom[i + j]);
        }

        // Add spacing if the last line has fewer than 16 bytes
        for (int j = size - i; j < 16 && i + j < size; j++) {
            printf("   ");
        }

        // Print the ASCII representation of the data
        printf(" |");
        for (int j = 0; j < 16 && (i + j) < size; j++) {
            char c = rom[i + j];
            if (c >= 32 && c <= 126) {
                printf("%c", c);  // Printable ASCII character
            } else {
                printf(".");      // Non-printable character
            }
        }
        printf("|\n");
    }
}


void __no_inline_not_in_flash_func(loadrom_plain32)(uint32_t offset, uint32_t size)
{
    // Set data bus to input mode
    set_data_bus_input();
    while (true) 
    {
        bool sltsl = (gpio_get(PIN_SLTSL) == 0); // Slot select (active low)
        bool rd = (gpio_get(PIN_RD) == 0);       // Read cycle (active low)

        if (sltsl) 
        {
            uint16_t addr = read_address_bus();
            if (addr >= 0x4000 && addr <= 0xBFFF) // Check if the address is within the ROM range
            {
                uint32_t rom_addr = offset + (addr - 0x4000); // Calculate flash address
                if (rd)
                {
                    set_data_bus_output(); // Drive data bus to output mode
                    write_data_bus(rom[rom_addr]);  // Drive data onto the bus
                    while (gpio_get(PIN_RD) == 0)  // Wait until the read cycle completes (RD goes high)
                    {
                        tight_loop_contents();
                    }
                    set_data_bus_input(); // Return data lines to input mode after cycle completes
                }
            } 
        } 
    }
}

// Load a simple 32KB ROM into the MSX
// They have two pages of 16Kb each in the following areas:
// 0x4000-0x7FFF and 0x8000-0xBFFF
// AB is on 0x0000, 0x0001
// The ROM data is stored in the flash memory starting at __flash_binary_end
void loadrom_plain32_sram(uint32_t offset, uint32_t size)
{
    //setup the rom_sram buffer for the 32KB ROM
    gpio_init(PIN_WAIT); gpio_set_dir(PIN_WAIT, GPIO_OUT);
    gpio_put(PIN_WAIT, 0); // Wait until we are ready to read the ROM
    memset(rom_sram, 0, MAX_MEM_SIZE); // Clear the SRAM buffer
    memcpy(rom_sram + 0x4000, rom + offset, size); //for 32KB ROMs we start at 0x4000
    gpio_put(PIN_WAIT, 1); // Lets go!

    // Set data bus to input mode
    set_data_bus_input();
    while (true) 
    {
        bool sltsl = (gpio_get(PIN_SLTSL) == 0); // Slot select (active low)
        bool rd = (gpio_get(PIN_RD) == 0);       // Read cycle (active low)

        if (sltsl) 
        {
            uint16_t addr = read_address_bus();
            if (addr >= 0x4000 && addr <= 0xBFFF) // Check if the address is within the ROM range
            {
                if (rd)
                {
                    set_data_bus_output(); // Drive data bus to output mode
                    write_data_bus(rom_sram[addr]);  // Drive data onto the bus
                    while (gpio_get(PIN_RD) == 0)  // Wait until the read cycle completes (RD goes high)
                    {
                        tight_loop_contents();
                    }
                    set_data_bus_input(); // Return data lines to input mode after cycle completes
                }
            } 
        } 
    }
}

// Load a simple 48KB Linear0 ROM into the MSX
// Those ROMs have three pages of 16Kb each in the following areas:
// 0x0000-0x3FFF, 0x4000-0x7FFF and 0x8000-0xBFFF
// AB is on 0x4000, 0x4001
// The ROM data is stored in the flash memory starting at __flash_binary_end
void __no_inline_not_in_flash_func(loadrom_linear48)(uint32_t offset, uint32_t size)
{
    set_data_bus_input();     // Set data bus to input mode
    while (true) 
    {
        // Check control signals
        bool sltsl = (gpio_get(PIN_SLTSL) == 0); // Slot select (active low)
        bool rd = (gpio_get(PIN_RD) == 0);       // Read cycle (active low)

        if (sltsl)
        {
            uint16_t addr = read_address_bus();
            if (addr >= 0x0000 && addr <= 0xBFFF) // Check if the address is within the ROM range
            {
                uint32_t rom_addr = offset + addr; // Calculate flash address
                if (rd)
                {
                    set_data_bus_output();  // Drive data bus to output mode
                    write_data_bus(rom[rom_addr]); // Drive data onto the bus
                    while (gpio_get(PIN_RD) == 0) // Wait until the read cycle completes (RD goes high)
                    {
                        tight_loop_contents();
                    }
                    set_data_bus_input(); // Return data lines to input mode after cycle completes
                }
            }
        }
    }
}

// Load a simple 48KB Linear0 ROM into the MSX
// Those ROMs have three pages of 16Kb each in the following areas:
// 0x0000-0x3FFF, 0x4000-0x7FFF and 0x8000-0xBFFF
// AB is on 0x4000, 0x4001
// The ROM data is stored in the flash memory starting at __flash_binary_end
void loadrom_linear48_sram(uint32_t offset, uint32_t size)
{

    gpio_init(PIN_WAIT); gpio_set_dir(PIN_WAIT, GPIO_OUT);
    gpio_put(PIN_WAIT, 0); // Wait until we are ready to read the ROM
    memset(rom_sram, 0, MAX_MEM_SIZE); // Clear the SRAM buffer
    memcpy(rom_sram, rom + offset, size);  // for 48KB Linear0 ROMs we start at 0x0000
    gpio_put(PIN_WAIT, 1); // Lets go!

    set_data_bus_input();     // Set data bus to input mode
    while (true) 
    {
        // Check control signals
        bool sltsl = (gpio_get(PIN_SLTSL) == 0); // Slot select (active low)
        bool rd = (gpio_get(PIN_RD) == 0);       // Read cycle (active low)

        if (sltsl)
        {
            uint16_t addr = read_address_bus();
            if (rd)
            {
                if (addr >= 0x0000 && addr <= 0xBFFF) // Check if the address is within the ROM range
                {
                    set_data_bus_output();  // Drive data bus to output mode
                    write_data_bus(rom_sram[addr]); // Drive data onto the bus
                    while (gpio_get(PIN_RD) == 0) // Wait until the read cycle completes (RD goes high)
                    {
                        tight_loop_contents();
                    }
                    set_data_bus_input(); // Return data lines to input mode after cycle completes
                }
            }
        }
    }
}


// Load a any Konami SCC ROM into the MSX
// The KonamiSCC ROM is divided into 8KB segments, managed by a memory mapper that allows dynamic switching of these segments into the MSX's address space
// Since the size of the mapper is 8Kb, the memory banks are:
//
// The memory banks are:
//
//	Bank 1: 4000h - 5FFFh
//	Bank 2: 6000h - 7FFFh
//	Bank 3: 8000h - 9FFFh
//	Bank 4: A000h - BFFFh
//
// And the address to change banks:
//
//	Bank 1: 5000h - 57FFh (5000h used)
//	Bank 2: 7000h - 77FFh (7000h used)
//	Bank 3: 9000h - 97FFh (9000h used)
//	Bank 4: B000h - B7FFh (B000h used)
void __no_inline_not_in_flash_func(loadrom_konamiscc)(uint32_t offset)
{
    gpio_init(PIN_WAIT); gpio_set_dir(PIN_WAIT, GPIO_IN);
    uint8_t bank_registers[4] = {0, 1, 2, 3}; // Initial banks 0-3 mapped

    set_data_bus_input();
    while (true) 
    {
        bool sltsl = (gpio_get(PIN_SLTSL) == 0); // Slot selected (active low)
        bool rd = (gpio_get(PIN_RD) == 0);       // Read cycle (active low)
        bool wr = (gpio_get(PIN_WR) == 0);       // Write cycle (active low)

        if (sltsl)
        {
            uint16_t addr = read_address_bus();
            if (addr >= 0x4000 && addr <= 0xBFFF) 
            {
                if (rd) 
                {
                    uint16_t bank_index = (addr - 0x4000) / 0x2000; // Calculate the bank index
                    uint16_t bank_offset = addr & 0x1FFF; // Calculate the offset within the bank
                    uint32_t rom_offset = (bank_registers[bank_index] * 0x2000) + bank_offset + offset; // Calculate the ROM offset
                    
                    gpio_set_dir(PIN_WAIT, GPIO_OUT);
                    gpio_put(PIN_WAIT, 0); // Wait until we are ready to read the ROM
                    uint8_t data = rom[rom_offset];
                    gpio_put(PIN_WAIT, 1); // Lets go!
                    set_data_bus_output();
                    write_data_bus(data); // Drive data onto the bus
                    while (gpio_get(PIN_RD) == 0) 
                    {
                        tight_loop_contents();
                    }
                    set_data_bus_input();
                } else if (wr) 
                {
                    // Handle writes to bank switching addresses
                    if (addr == 0x5000) {
                        bank_registers[0] = read_data_bus(); // Read the data bus and store in bank register
                    } else if (addr == 0x7000) {
                        bank_registers[1] = read_data_bus();
                    } else if (addr == 0x9000) {
                        bank_registers[2] = read_data_bus();
                    } else if (addr == 0xB000) {
                        bank_registers[3] = read_data_bus();
                    }

                    while (gpio_get(PIN_WR) == 0) 
                    {
                        tight_loop_contents();
                    }
                }
            }
        }
    }
}


// Load a 128KB Konami SCC ROM into the MSX
// The KonamiSCC ROM is divided into 8KB segments, managed by a memory mapper that allows dynamic switching of these segments into the MSX's address space
// Since the size of the mapper is 8Kb, the memory banks are:
//
// The memory banks are:
//
//	Bank 1: 4000h - 5FFFh
//	Bank 2: 6000h - 7FFFh
//	Bank 3: 8000h - 9FFFh
//	Bank 4: A000h - BFFFh
//
// And the address to change banks:
//
//	Bank 1: 5000h - 57FFh (5000h used)
//	Bank 2: 7000h - 77FFh (7000h used)
//	Bank 3: 9000h - 97FFh (9000h used)
//	Bank 4: B000h - B7FFh (B000h used)
void loadrom_konamiscc_sram(uint32_t offset, uint32_t size)
{
    gpio_init(PIN_WAIT); gpio_set_dir(PIN_WAIT, GPIO_OUT);
    gpio_put(PIN_WAIT, 0); // Wait until we are ready to read the ROM
    memset(rom_sram, 0, size+offset); // Clear the SRAM buffer
    memcpy(rom_sram, rom, size+offset);  // for 128KB KonamiSCC ROMs we start at 0x0000
    gpio_put(PIN_WAIT, 1); // Lets go!

    uint8_t bank_registers[4] = {0, 1, 2, 3}; // Initial banks 0-3 mapped

    set_data_bus_input();
    while (true) 
    {
        bool sltsl = (gpio_get(PIN_SLTSL) == 0); // Slot selected (active low)
        bool rd = (gpio_get(PIN_RD) == 0);       // Read cycle (active low)
        bool wr = (gpio_get(PIN_WR) == 0);       // Write cycle (active low)

        if (sltsl)
        {
            uint16_t addr = read_address_bus();
            if (addr >= 0x4000 && addr <= 0xBFFF) 
            {
                if (rd) 
                {
                    uint16_t bank_index = (addr - 0x4000) / 0x2000; // Calculate the bank index
                    uint16_t bank_offset = addr & 0x1FFF; // Calculate the offset within the bank
                    uint32_t rom_offset = (bank_registers[bank_index] * 0x2000) + bank_offset + offset; // Calculate the ROM offset
                    set_data_bus_output();
                    write_data_bus(rom_sram[rom_offset]); // Drive data onto the bus
                    while (gpio_get(PIN_RD) == 0) 
                    {
                        tight_loop_contents();
                    }
                    set_data_bus_input();
                } else if (wr) 
                {
                    // Handle writes to bank switching addresses
                    if (addr == 0x5000) {
                        bank_registers[0] = read_data_bus(); // Read the data bus and store in bank register
                    } else if (addr == 0x7000) {
                        bank_registers[1] = read_data_bus();
                    } else if (addr == 0x9000) {
                        bank_registers[2] = read_data_bus();
                    } else if (addr == 0xB000) {
                        bank_registers[3] = read_data_bus();
                    }
                }
            }
        }
    }
}

// Load a Konami (without SCC) ROM into the MSX
// The Konami (without SCC) ROM is divided into 8KB segments, managed by a memory mapper that allows dynamic switching of these segments into the MSX's address space
// Since the size of the mapper is 8Kb, the memory banks are:
//
// The memory banks are:
//  Bank 1: 4000h - 5FFFh
//	Bank 2: 6000h - 7FFFh
//	Bank 3: 8000h - 9FFFh
//	Bank 4: A000h - BFFFh
//
// And the address to change banks:
//
//	Bank 1: <none>
//	Bank 2: 6000h - 7FFFh (6000h used)
//	Bank 3: 8000h - 9FFFh (8000h used)
//	Bank 4: A000h - BFFFh (A000h used)
void loadrom_konami(uint32_t offset, uint32_t size)
{
    gpio_init(PIN_WAIT); gpio_set_dir(PIN_WAIT, GPIO_OUT);
    gpio_put(PIN_WAIT, 0); // Wait until we are ready to read the ROM
    memset(rom_sram, 0, MAX_MEM_SIZE); // Clear the SRAM buffer
    memcpy(rom_sram, rom + offset, size);  // for 12KB KonamiSCC ROMs we start at 0x0000
    gpio_put(PIN_WAIT, 1); // Lets go!

    uint8_t bank_registers[4] = {0, 1, 2, 3}; // Initial banks 0-3 mapped

    set_data_bus_input();
    while (true) 
    {
        bool sltsl = (gpio_get(PIN_SLTSL) == 0); // Slot selected (active low)
        bool rd = (gpio_get(PIN_RD) == 0);       // Read cycle (active low)
        bool wr = (gpio_get(PIN_WR) == 0);       // Write cycle (active low)

        if (sltsl) {
            uint16_t addr = read_address_bus();

            if (rd) {
                // Handle read requests within the ROM address range
                if (addr >= 0x4000 && addr <= 0xBFFF) {
                    // Determine the bank index based on the address
                    uint8_t bank_index = (addr - 0x4000) / 0x2000;
                    uint16_t bank_offset = addr & 0x1FFF;
                    uint32_t rom_offset = (bank_registers[bank_index] * 0x2000) + bank_offset;

                    // Set data bus to output mode and write the data
                    set_data_bus_output();
                    write_data_bus(rom_sram[rom_offset]);

                    // Wait for the read cycle to complete
                    while (gpio_get(PIN_RD) == 0) {
                        tight_loop_contents();
                    }

                    // Return data bus to input mode after the read cycle
                    set_data_bus_input();
                }
            } else if (wr) {
                // Handle writes to bank switching addresses
                if (addr == 0x6000) {
                    bank_registers[1] = read_data_bus();
                } else if (addr == 0x8000) {
                    bank_registers[2] = read_data_bus();
                } else if (addr == 0xA000) {
                    bank_registers[3] = read_data_bus();
                }
            }
        }
    }
}

// Load an ASCII8 ROM into the MSX
// The ASCII8 ROM is divided into 8KB segments, managed by a memory mapper that allows dynamic switching of these segments into the MSX's address space
// Since the size of the mapper is 8Kb, the memory banks are:
// Bank 1: 4000h - 5FFFh
// Bank 2: 6000h - 7FFFh
// Bank 3: 8000h - 9FFFh
// Bank 4: A000h - BFFFh
//
// And the address to change banks:
// Bank 1: 6000h - 67FFh (6000h used)
// Bank 2: 6800h - 6FFFh (6800h used)
// Bank 3: 7000h - 77FFh (7000h used)
// Bank 4: 7800h - 7FFFh (7800h used)
void loadrom_ascii8(uint32_t offset, uint32_t size)
{
    gpio_init(PIN_WAIT); gpio_set_dir(PIN_WAIT, GPIO_OUT);
    gpio_put(PIN_WAIT, 0); // Wait until we are ready to read the ROM
    memset(rom_sram, 0, MAX_MEM_SIZE); // Clear the SRAM buffer
    memcpy(rom_sram, rom + offset, size);  // for 12KB KonamiSCC ROMs we start at 0x0000
    gpio_put(PIN_WAIT, 1); // Lets go!

    uint8_t bank_registers[4] = {0, 1, 2, 3}; // Initial banks 0-3 mapped

    set_data_bus_input();
    while (true) 
    {
        bool sltsl = (gpio_get(PIN_SLTSL) == 0); // Slot selected (active low)
        bool rd = (gpio_get(PIN_RD) == 0);       // Read cycle (active low)
        bool wr = (gpio_get(PIN_WR) == 0);       // Write cycle (active low)

        if (sltsl) {
            uint16_t addr = read_address_bus();

            if (rd) {
                // Handle read requests within the ROM address range
                if (addr >= 0x4000 && addr <= 0xBFFF) {
                    // Determine the bank index based on the address
                    uint8_t bank_index = (addr - 0x4000) / 0x2000;
                    uint16_t bank_offset = addr & 0x1FFF;
                    uint32_t rom_offset = (bank_registers[bank_index] * 0x2000) + bank_offset;

                    // Set data bus to output mode and write the data
                    set_data_bus_output();
                    write_data_bus(rom_sram[rom_offset]);

                    // Wait for the read cycle to complete
                    while (gpio_get(PIN_RD) == 0) {
                        tight_loop_contents();
                    }

                    // Return data bus to input mode after the read cycle
                    set_data_bus_input();
                }
            } else if (wr) {
                // Handle writes to bank switching addresses
                if (addr == 0x6000) {
                    bank_registers[0] = read_data_bus(); // Read the data bus and store in bank register
                } else if (addr == 0x6800) {
                    bank_registers[1] = read_data_bus();
                } else if (addr == 0x7000) {
                    bank_registers[2] = read_data_bus();
                } else if (addr == 0x7800) {
                    bank_registers[3] = read_data_bus();
                }
            }
        }
    }
}


// Load an ASCII16 ROM into the MSX
// The ASCII16 ROM is divided into 16KB segments, managed by a memory mapper that allows dynamic switching of these segments into the MSX's address space
// Since the size of the mapper is 16Kb, the memory banks are:
// Bank 1: 4000h - 7FFFh
// Bank 2: 8000h - BFFFh
//
// And the address to change banks:
// Bank 1: 6000h - 67FFh (6000h used)
// Bank 2: 7000h - 77FFh (7000h and 77FFh used)
void loadrom_ascii16(uint32_t offset, uint32_t size)
{
    // Initialize the WAIT pin to control the MSX's WAIT signal
    gpio_init(PIN_WAIT);
    gpio_set_dir(PIN_WAIT, GPIO_OUT);
    gpio_put(PIN_WAIT, 0); // Signal the MSX to wait
    memcpy(rom_sram, rom + offset, size);
    gpio_put(PIN_WAIT, 1);

    uint8_t bank_registers[2] = {0, 1}; // Initial banks 0 and 1 mapped

    set_data_bus_input();
    while (true) {
        // Check control signals
        bool sltsl = (gpio_get(PIN_SLTSL) == 0); // Slot selected (active low)
        bool rd = (gpio_get(PIN_RD) == 0);       // Read cycle (active low)
        bool wr = (gpio_get(PIN_WR) == 0);       // Write cycle (active low)

        if (sltsl) {
            uint16_t addr = read_address_bus();

            if (rd) {
                // Handle read requests within the ROM address range
                if (addr >= 0x4000 && addr <= 0xBFFF) {
                    // Determine the bank index based on the address
                    uint8_t bank_index = (addr >= 0x8000) ? 1 : 0;
                    uint16_t bank_offset = addr & 0x3FFF;
                    uint32_t rom_offset = (bank_registers[bank_index] * 0x4000) + bank_offset;

                    // Set data bus to output mode and write the data
                    set_data_bus_output();
                    write_data_bus(rom_sram[rom_offset]);

                    // Wait for the read cycle to complete
                    while (gpio_get(PIN_RD) == 0) {
                        tight_loop_contents();
                    }

                    // Return data bus to input mode after the read cycle
                    set_data_bus_input();
                }
            } else if (wr) {
                // Handle write operations for bank switching
                uint8_t data = read_data_bus();

                // Update bank registers based on the specific switching addresses
                switch (addr) {
                    case 0x6000:
                        bank_registers[0] = data;
                        break;
                    case 0x7000:
                    case 0x77FF:
                        bank_registers[1] = data;
                        break;
                    default:
                        // Address does not correspond to bank switching
                        break;
                }
            }
        }
    }
}


// -----------------------
// Main program
// -----------------------
int __no_inline_not_in_flash_func(main)()
{
    // Set systm clock to 270MHz
    set_sys_clock_khz(270000, true);
    //
    // Initialize stdio
    stdio_init_all();
    // Initialize GPIO
    setup_gpio();

    // Detect the ROM type
    char rom_name[ROM_NAME_MAX];
    memcpy(rom_name, rom, ROM_NAME_MAX);
    uint8_t rom_type = rom[ROM_NAME_MAX];
    uint32_t rom_size;
    memcpy(&rom_size, rom + ROM_NAME_MAX + 1, sizeof(uint32_t));

    // Print the ROM name and type
    printf("ROM name: %s\n", rom_name);
    printf("ROM type: %d\n", rom_type);
    printf("ROM size: %d\n", rom_size);

    // Load the ROM based on the detected type
    // 1 - 16KB ROM
    // 2 - 32KB ROM
    // 3 - Konami SCC ROM
    // 4 - 48KB Linear0 ROM
    // 5 - ASCII8 ROM
    // 6 - ASCII16 ROM
    // 7 - Konami (without SCC) ROM
    switch (rom_type) 
    {
        case 1:
        case 2:
            loadrom_plain32(0x1d, rom_size);
            //loadrom_plain32_flash_cache(0x1d, rom_size);
            break;
        case 3:
            loadrom_konamiscc(0x1d);
            //loadrom_konamiscc_sram(0x1d, rom_size);
            //loadrom_konamiscc_test(0x1d, rom_size);
            break;
        case 4:
            loadrom_linear48(0x1d, rom_size);
            break;
        case 5:
            loadrom_ascii8(0x1d, rom_size);
            break;
        case 6:
            loadrom_ascii16(0x1d, rom_size);
            break;
        case 7:
            loadrom_konami(0x1d, rom_size);
            break;
        default:
            printf("Unknown ROM type: %d\n", 1);
            break;
    }
    return 0;
}
