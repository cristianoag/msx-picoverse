// MSX PICOVERSE PROJECT
// (c) 2024 Cristiano Goncalves
// The Retro Hacker
//
// multirom.c - This is the Raspberry Pico firmware that will be used to load ROMs into the MSX
//
// This firmware is responsible for loading the multirom menu and the ROMs selected by the user into the MSX. When flashed through the 
// multirom tool, it will be stored on the pico flash memory followed by the configuration area and all the ROMs processed by the 
// multirom tool. The sofware in this firmware will load the first 32KB ROM that contains the menu into the MSX and it will allow the user
// to select a ROM to be loaded into the MSX. The selected ROM will be loaded into the MSX and the MSX will be reseted to run the selected ROM.
//
// This work is licensed  under a "Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
// License". https://creativecommons.org/licenses/by-nc-sa/4.0/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "multirom.h"

// config area and buffer for the ROM data
#define ROM_START = 0x1D00;
static bool USE_SRAM_COPY = true;

// Read the address bus from the MSX
static inline uint16_t read_address_bus(void) {
    // Return first 16 bits in the most efficient way
    return gpio_get_all() & 0x00FFFF;
}

// Write a byte to the data bus
static inline void write_data_bus(uint8_t data) 
{
    // Write the given byte to the given address
    gpio_put_masked(0xFF0000, data << 16);
}

// Set the data bus to input mode
static inline void set_data_bus_input(void) {
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
static inline void set_data_bus_output(void)
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
    //Dump rom_sram in the hexdump format
    printf("Dumping the first 32KB of the ROM\n");
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

// Load a simple 32KB ROM into the MSX
// 32KB ROMs are the most common ROMs for the MSX and are mapped to the 0x4000-0xBFFF range
// They have two pages of 16Kb each in the following areas:
// 0x4000-0x7FFF and 0x8000-0xBFFF
// The ROM data is stored in the flash memory starting at __flash_binary_end + 0x1D00
int loadrom_32k(uint32_t offset, uint32_t size)
{
    // if SRAM copy is enabled, copy the ROM data to the SRAM buffer
    if (USE_SRAM_COPY) {
       gpio_init(PIN_WAIT); gpio_set_dir(PIN_WAIT, GPIO_OUT);
       gpio_put(PIN_WAIT, 0); // Wait until we are ready to read the ROM
       memcpy(rom_sram, rom + offset, size);
       gpio_put(PIN_WAIT, 1); // Lets go!
    }

    // Set data bus to input mode
    set_data_bus_input();
    while (1) 
    {
        // Check control signals
        bool sltsl = (gpio_get(PIN_SLTSL) == 0); // Slot select (active low)
        bool rd = (gpio_get(PIN_RD) == 0);       // Read cycle (active low)
        bool wr = (gpio_get(PIN_WR) == 0);       // Write cycle (active low, not used)
        
        // MSX is requesting a memory read from this slot
        if (sltsl && rd && !wr) 
        {
            //printf("Debug: sltsl=%d rd=%d wr=%d\n", sltsl, rd, wr);
            uint16_t addr = read_address_bus();
            // Check if the address is within the ROM range
            if (addr >= 0x4000 && addr <= 0xBFFF) 
            {
                // Drive data bus to output mode
                set_data_bus_output();

                // Address is within the ROM range
                uint16_t flash_offset = (addr - 0x4000) + offset;
                uint8_t data = USE_SRAM_COPY ? rom_sram[(addr - 0x4000)] : rom[flash_offset];

                // Drive data onto the bus
                write_data_bus(data);

                // Wait until the read cycle completes (RD goes high)
                while (gpio_get(PIN_RD) == 0) {
                    tight_loop_contents();
                   }

                // Return data lines to input mode after cycle completes
                set_data_bus_input();
            }
        } else {
            // Not a read cycle - ensure data bus is not driven
            set_data_bus_input();
        }
    }
    return 0;
}

// Load a 16KB ROM into the MSX
int loadrom_16k(uint32_t offset, uint32_t size)
{
    //to be implemented
}

// Load a Linear0 ROM into the MSX
int loadrom_Linear0(uint32_t offset, uint32_t size)
{
    //to be implemented
}

// Load a KonamiSCC ROM into the MSX
int loadrom_KonamiSCC(uint32_t offset, uint32_t size)
{
    //to be implemented
}

// Main function
int main()
{
    printf("Debug: Starting the MSX PICOVERSE 2040 multirom firmware\n");
    // Set system clock to 240MHz
    set_sys_clock_khz(240000, true);
    // Initialize stdio
    stdio_init_all();
    // Initialize GPIO
    setup_gpio();

    printf("Debug: Loading the MSX Menu ROM\n");
    // The configuration area has (256 * (20 + 1 + 4 + 4)) = 7424 bytes (0x1d00h)
    // So the ROM data starts at __flash_binary_end & 0x1d0
    int ret = loadrom_32k(0x1d00, 32768); //load the first 32KB ROM into the MSX
    printf("Debug: MSX Menu ROM loaded\n");

    if (ret != 0) {
        printf("Debug: Error loading ROM!\n");
    }

    return 0;
}
