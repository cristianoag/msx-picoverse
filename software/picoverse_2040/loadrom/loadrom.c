// MSX PICOVERSE PROJECT
// (c) 2024 Cristiano Goncalves
// The Retro Hacker
//
// loadrom.c - Simple ROM loader for MSX PICOVERSE project - v1.0
//
// This is  small test program that demonstrates how to  load simple ROM images  using the MSX  PICOVERSE
// project. You need to concatenate the ROM image to the  end of this program binary in order  to load it.
// The program will then act as a simple ROM cartridge that responds to memory read requests from the MSX.
// 
// This work is licensed  under a "Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
// License". https://creativecommons.org/licenses/by-nc-sa/4.0/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"

// -----------------------
// Maximum ROM Size
// 128KB is the maximum size of a ROM cartridge supported by this tool.
// -----------------------
#define MAX_ROM_SIZE (48*1024)

// -----------------------
// User-defined pin assignments
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
static uint8_t rom_sram[MAX_ROM_SIZE];
static bool USE_SRAM_COPY = true;
// The ROM is concatenated right after the main program binary.
// __flash_binary_end points to the end of the program in flash memory.
const uint8_t *rom = (const uint8_t *)&__flash_binary_end;

// Read the address bus from the MSX
static inline uint16_t read_address_bus(void) {
    // Return first 16 bits in the most efficient way
    return gpio_get_all() & 0x00FFFF;
}

// Write a byte to the data bus
static inline void write_data_bus(uint8_t data) {
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

// Detect the size of the ROM by scanning for a sequence of 16 zero bytes
static uint32_t detect_rom_size(const uint8_t *rom, size_t max_size) {
    // Start scanning for 16 consecutive zero bytes
    for (uint32_t i = 0; i <= max_size - 16; i++) {
        bool all_zero = true;
        for (uint32_t j = 0; j < 16; j++) {
            if (rom[i + j] != 0) {
                all_zero = false;
                break;
            }
        }
        if (all_zero) {
            return i; // ROM size is up to the start of the zero sequence
        }
    }
    return max_size; // If no sequence of 16 zeros found, assume full size
}

// Detect the ROM type by analyzing the AB signature at 0x0000 and 0x0001 or 0x4000 and 0x4001
static uint32_t detect_rom_type(const uint8_t *rom, size_t max_size) {
    // Check if the ROM has the signature "AB" at 0x0000 and 0x0001
    // Those are the cases for 16KB and 32KB ROMs
    if (rom[0] == 'A' && rom[1] == 'B') {
        return 32768;     // I can return 32768 for both
    }
    // Check if the ROM has the signature "AB" at 0x4000 and 0x4001
    if (rom[0x4000] == 'A' && rom[0x4001] == 'B') {
        return 49152; // 48KB
    }
    return max_size; // If no signature found, assume full size
}

// Load a simple 32KB ROM into the MSX
// They have two pages of 16Kb each in the following areas:
// 0x4000-0x7FFF and 0x8000-0xBFFF
// AB is on 0x0000, 0x0001
// The ROM data is stored in the flash memory starting at __flash_binary_end
int loadrom_32k(uint32_t offset, uint32_t size)
{
    uint32_t byte_counter = 0;
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
                byte_counter++;

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
            //printf ("%d bytes read\n", byte_counter);
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
            uint16_t addr = read_address_bus();
            // Check if the address is within the ROM range
            if (addr >= 0x0000 && addr <= 0xBFFF) 
            {
                // Address is within the ROM range
                uint16_t flash_offset = addr + offset;
                uint8_t data = USE_SRAM_COPY ? rom_sram[addr] : rom[flash_offset];

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

// -----------------------
// Main program
// -----------------------
int main()
{
    // Set system clock to 240MHz
    set_sys_clock_khz(240000, true);
    // Initialize stdio
    stdio_init_all();
    // Initialize GPIO
    setup_gpio();

    uint32_t rom_size = detect_rom_type(rom, MAX_ROM_SIZE);

    switch (rom_size)
    {
        case 16384:
            printf("Detected 16KB ROM\n");
            loadrom_32k(0x0000, rom_size);
            break;
        case 32768: // 32KB ROM
            printf("Detected 32KB ROM\n");
            loadrom_32k(0x0000, rom_size);
            printf("32KB ROM loaded!\n");
            break;
        case 49152: // 48KB ROM
            printf("Detected 48KB ROM\n");
            loadrom_48k_Linear0(0x0000, rom_size);
            break;
        default:
            printf("Unsupported ROM!\n");
            break;
    }

    return 0;
}
