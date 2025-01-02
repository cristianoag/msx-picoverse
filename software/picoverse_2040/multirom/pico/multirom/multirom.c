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
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "multirom.h"

// config area and buffer for the ROM data
#define ROM_START = 0x8000; // ROM data starts at __flash_binary_end + 32768 (MENU)  = __flash_binary_end + 0x8000h
static bool USE_SRAM_COPY = true; // Use SRAM copy of the ROM data

// Struct to hold both address and data
typedef struct {
    uint16_t address;
    uint8_t data;
} bus_data_t;

// Read both the address and data bus
static inline bus_data_t read_address_and_data_bus(void) {
    uint32_t gpio_values = gpio_get_all();
    bus_data_t result;
    result.address = gpio_values & 0x00FFFF;  // Lower 16 bits for address
    result.data = (gpio_values >> 16) & 0xFF; // Upper 8 bits for data
    return result;
}

/*
// Read the address bus from the MSX
static inline uint16_t read_address_bus(void) {
    // Return first 16 bits in the most efficient way
    return gpio_get_all() & 0x00FFFF;
}*/

// Write a byte to the data bus
static inline void write_data_bus(uint8_t data) 
{
    // Write the given byte to the given address
    gpio_put_masked(0xFF0000, data << 16);
}

// Read a byte from the data bus
static inline uint8_t read_data_bus(void) 
{
    // Read the data bus
    return (gpio_get_all() >> 16) & 0xFF;
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
    while (true) 
    {
        // Check control signals
        bool sltsl = (gpio_get(PIN_SLTSL) == 0); // Slot select (active low)
        bool rd = (gpio_get(PIN_RD) == 0);       // Read cycle (active low)
        bool wr = (gpio_get(PIN_WR) == 0);       // Write cycle (active low, not used)
        
        // MSX is requesting a memory read from this slot
        if (sltsl && rd && !wr) 
        {
            bus_data_t bus_values = read_address_and_data_bus();
            // Check if the address is within the ROM range
            if (bus_values.address >= 0x4000 && bus_values.address <= 0xBFFF) 
            {
                // Address is within the ROM range
                uint16_t flash_offset = (bus_values.address - 0x4000) + offset;
                uint8_t data = USE_SRAM_COPY ? rom_sram[(bus_values.address - 0x4000)] : rom[flash_offset];

                // Drive data bus to output mode
                set_data_bus_output();
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

// Load a simple 48KB Linear0 ROM into the MSX
// Those ROMs have three pages of 16Kb each in the following areas:
// 0x0000-0x3FFF, 0x4000-0x7FFF and 0x8000-0xBFFF
// AB is on 0x4000, 0x4001
// The ROM data is stored in the flash memory starting at __flash_binary_end
int loadrom_48k_Linear0(uint32_t offset, uint32_t size)
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
    while (true) 
    {
        // Check control signals
        bool sltsl = (gpio_get(PIN_SLTSL) == 0); // Slot select (active low)
        bool rd = (gpio_get(PIN_RD) == 0);       // Read cycle (active low)
        bool wr = (gpio_get(PIN_WR) == 0);       // Write cycle (active low, not used)
        
        // MSX is requesting a memory read from this slot
        if (sltsl && rd && !wr) 
        {
            //uint16_t addr = read_address_bus();
            bus_data_t bus_values = read_address_and_data_bus();
            // Check if the address is within the ROM range
            if (bus_values.address >= 0x0000 && bus_values.address <= 0xBFFF) 
            {
                // Address is within the ROM range
                uint16_t flash_offset = bus_values.address + offset;
                uint8_t data = USE_SRAM_COPY ? rom_sram[bus_values.address] : rom[flash_offset];

                // Drive data bus to output mode
                set_data_bus_output();
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

// Load a KonamiSCC ROM into the MSX
int loadrom_KonamiSCC(uint32_t offset, uint32_t size)
{
    //to be implemented
}

// Main function running on core 1
void core1_entry() {
    // Task for core 1
    // We may need a mutex depending of what kind of racing condition we may have
    // pending research
    while (true) 
    {
        // intercepts IORQ operation on port 0x20
        // if the value is 0x20, it means that the user selected a ROM to be loaded
        // data bus will have the index of the ROM to be loaded
        // the ROM will be loaded into the MSX and the MSX will be reseted to run the selected ROM

        // Check control signals
        bool iorq = (gpio_get(PIN_IORQ) == 0); // IORQ signal (active low)
        if (iorq) {
            //uint16_t addr = read_address_bus();
            set_data_bus_input();
            bus_data_t bus_values = read_address_and_data_bus();

            // Check if the address is 0x20
            if ((bus_values.address & 0xFF) == 0x20) 
            {
                // Set data bus to input mode to read data from MSX
                //set_data_bus_input();
                uint8_t rom_index = bus_values.data+1; // this is because the MSX menu which is the index 0 is never shown on the interface
                // Read the data bus to get the ROM index
                printf("Debug: Data bus value for addr 0x20 is %d\n", rom_index);

                // Implement the logic to load the selected ROM based on rom_index
                // ...

                // Reset the MSX to run the selected ROM
                // ...
            }
        }

    }
}


// Main function running on core 0
int main()
{
    printf("Debug: Starting the MSX PICOVERSE 2040 multirom firmware\n");
    // Set system clock to 240MHz
    set_sys_clock_khz(240000, true);
    // Initialize stdio
    stdio_init_all();
    // Initialize GPIO
    setup_gpio();

    // Start core 1
    multicore_launch_core1(core1_entry);

    printf("Debug: Loading the MSX Menu ROM\n");
    // The configuration area has (256 * (20 + 1 + 4 + 4)) = 7424 bytes (0x1d00h)
    // So the ROM data starts at __flash_binary_end & 0x1d0
    // the ROM reading is a blocking operation, so while the game is running, load_rom will be mapping addresses to the MSX
    // whithin the memory range for the ROM. core 1 runs a routine to control when the loadrom needs to leave the loop
    int ret = loadrom_32k(0x0000, 32768); //load the first 32KB ROM into the MSX

    if (ret != 0) {
        printf("Debug: Error loading ROM!\n");
    }

    return 0;
}
