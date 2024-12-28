// MSX PICOVERSE PROJECT
// (c) 2024 Cristiano Goncalves
// (c) The Retro Hacker
//
// loadrom.c
//
// This is small test program that demonstrates how to load simple ROM images using the MSX PICOVERSE
// project. You need to concatenate the ROM image to the end of this program binary in order to load it.
// The program will then act as a simple ROM cartridge that responds to memory read requests from the MSX.
// 
// This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
//
// If you use, remix, transform, or build upon the material, you must distribute your contributions under the same 
// license as the original. You may not use the material for commercial purposes. You must give appropriate credit, 
// provide a link to the license, and indicate if changes were made. You may do so in any reasonable manner, but 
// not in any way that suggests the licensor endorses you or your use.
//
// ATTENTION
//
// This project was made for the retro community and not for commercial purposes. So only retro hardware forums and individual people can 
// build this project. If you are a company and want to build this project, please contact me first.

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"

// -----------------------
// ROM size
// We assume the ROM is exactly 32KB.
#define ROM_SIZE (32*1024)

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
// Your ROM data is concatenated immediately after this point.
extern unsigned char __flash_binary_end;

// Optionally copy the 32KB ROM into this SRAM buffer for faster access
static uint8_t rom_sram[ROM_SIZE];
static bool use_sram_copy = true;

// -----------------------
// Helper functions
// -----------------------
static inline uint16_t read_address_bus(void) {
    // Return first 16 bits in the most efficient way
    return gpio_get_all() & 0x00FFFF;
}

static inline void write_data_bus(uint8_t data) {
    // Write the given byte to the given address
    gpio_put_masked(0xFF0000, data << 16);
}

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

    // WAIT line
    gpio_init(PIN_WAIT);
    gpio_set_dir(PIN_WAIT, GPIO_OUT);
    gpio_put(PIN_WAIT, 0); // Wait until we are ready

    // The 32KB ROM is concatenated right after the main program binary.
    // __flash_binary_end points to the end of the program in flash memory.
    const uint8_t *rom = (const uint8_t *)&__flash_binary_end;

    // (Optional) Copy the 32KB appended ROM from Flash to SRAM
    if (use_sram_copy) {
        memcpy(rom_sram, rom, ROM_SIZE);
        //printf("Debug: Copied 32KB ROM to SRAM.\n");
    }

    //printf("Debug: Program started, ROM at %p, size=%d bytes\n", rom, ROM_SIZE);
    uint32_t cycle_count = 0;
    uint8_t data;

    gpio_put(PIN_WAIT, 1); // Lets go!

    // Set data bus to input mode
    set_data_bus_input();
    while (1) {
        // Check control signals
        bool sltsl = (gpio_get(PIN_SLTSL) == 0); // Slot select (active low)
        bool rd = (gpio_get(PIN_RD) == 0);       // Read cycle (active low)
        bool wr = (gpio_get(PIN_WR) == 0);       // Write cycle (active low, not used)

        if (sltsl && rd && !wr) {
            // MSX is requesting a memory read from this slot
            uint16_t addr = read_address_bus();
            
            if (addr >= 0x4000 && addr <= 0xBFFF) {
                // Drive data bus to output mode
                set_data_bus_output();

                // Address is within the ROM range
                uint16_t offset = addr - 0x4000;
                data = use_sram_copy ? rom_sram[offset] : rom[offset];
                
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

        cycle_count++;
        //tight_loop_contents();
    }

    return 0;
}
