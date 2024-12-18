// MSX PICOVERSE PROJECT
//
// This is small test program that demonstrates how to load simple ROM images using the MSX PICOVERSE
// project. You need to concatenate the ROM image to the end of this program binary in order to load it.
// The program will then act as a simple ROM cartridge that responds to memory read requests from the MSX.
// The ROM image is assumed to be exactly 32KB in size (at this moment).
// Author: Cristiano Goncalves

#include <stdio.h>
#include "pico/stdlib.h"
#include "loadrom.h"

// -----------------------
// ROM size
// We assume the ROM is exactly 32KB.
#define ROM_SIZE (32*1024)

// This symbol marks the end of the main program in flash.
// Your ROM data is concatenated immediately after this point.
extern unsigned char __flash_binary_end;

// -----------------------
// Helper functions
// -----------------------
static inline uint16_t read_address_bus(void) {
    uint16_t addr = 0;
    addr |= gpio_get(PIN_A0)  << 0;
    addr |= gpio_get(PIN_A1)  << 1;
    addr |= gpio_get(PIN_A2)  << 2;
    addr |= gpio_get(PIN_A3)  << 3;
    addr |= gpio_get(PIN_A4)  << 4;
    addr |= gpio_get(PIN_A5)  << 5;
    addr |= gpio_get(PIN_A6)  << 6;
    addr |= gpio_get(PIN_A7)  << 7;
    addr |= gpio_get(PIN_A8)  << 8;
    addr |= gpio_get(PIN_A9)  << 9;
    addr |= gpio_get(PIN_A10) << 10;
    addr |= gpio_get(PIN_A11) << 11;
    addr |= gpio_get(PIN_A12) << 12;
    addr |= gpio_get(PIN_A13) << 13;
    addr |= gpio_get(PIN_A14) << 14;
    addr |= gpio_get(PIN_A15) << 15;
    return addr;
}

static inline void set_data_bus_output(uint8_t data) {
    // Set data lines to output and write the given byte
    gpio_set_dir(PIN_D0, GPIO_OUT);
    gpio_set_dir(PIN_D1, GPIO_OUT);
    gpio_set_dir(PIN_D2, GPIO_OUT);
    gpio_set_dir(PIN_D3, GPIO_OUT);
    gpio_set_dir(PIN_D4, GPIO_OUT);
    gpio_set_dir(PIN_D5, GPIO_OUT);
    gpio_set_dir(PIN_D6, GPIO_OUT);
    gpio_set_dir(PIN_D7, GPIO_OUT);

    gpio_put(PIN_D0, (data & 0x01) ? 1 : 0);
    gpio_put(PIN_D1, (data & 0x02) ? 1 : 0);
    gpio_put(PIN_D2, (data & 0x04) ? 1 : 0);
    gpio_put(PIN_D3, (data & 0x08) ? 1 : 0);
    gpio_put(PIN_D4, (data & 0x10) ? 1 : 0);
    gpio_put(PIN_D5, (data & 0x20) ? 1 : 0);
    gpio_put(PIN_D6, (data & 0x40) ? 1 : 0);
    gpio_put(PIN_D7, (data & 0x80) ? 1 : 0);
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

// -----------------------
// Main program
// -----------------------
int main()
{
    stdio_init_all();
    
    sleep_ms(2000); // Give time for USB to enumerate

    // Initialize address pins as input
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

    // Initialize data pins as input (high-Z)
    gpio_init(PIN_D0);
    gpio_init(PIN_D1);
    gpio_init(PIN_D2);
    gpio_init(PIN_D3);
    gpio_init(PIN_D4);
    gpio_init(PIN_D5);
    gpio_init(PIN_D6);
    gpio_init(PIN_D7);

    set_data_bus_input();

    // Initialize control pins as input
    gpio_init(PIN_RD);    gpio_set_dir(PIN_RD, GPIO_IN);
    gpio_init(PIN_WR);    gpio_set_dir(PIN_WR, GPIO_IN);
    gpio_init(PIN_IORQ);  gpio_set_dir(PIN_IORQ, GPIO_IN);
    gpio_init(PIN_SLTSL); gpio_set_dir(PIN_SLTSL, GPIO_IN);
    gpio_init(PIN_BUSSDIR); gpio_set_dir(PIN_BUSSDIR, GPIO_IN);

    // WAIT line
    gpio_init(PIN_WAIT);
    gpio_set_dir(PIN_WAIT, GPIO_OUT);
    gpio_put(PIN_WAIT, 1); // Default no wait

    // The 32KB ROM is concatenated right after the main program binary.
    // __flash_binary_end points to the end of the program in flash memory.
    const uint8_t *rom = (const uint8_t *)&__flash_binary_end;
    printf("Debug: Program started, ROM at %p, size=%d bytes\n", rom, ROM_SIZE);
    uint32_t cycle_count = 0;
    uint8_t data;

    while (1) {
        // Check control signals
        bool sltsl = (gpio_get(PIN_SLTSL) == 0); // Slot select (active low)
        bool rd = (gpio_get(PIN_RD) == 0);       // Read cycle (active low)
        bool wr = (gpio_get(PIN_WR) == 0);       // Write cycle (active low, not used)

        if (sltsl && rd && !wr) {
            // MSX is requesting a memory read from this slot
            uint16_t addr = read_address_bus();

            if (addr >= 0x4000 && addr <= 0xBFFF) {
                // Address is within the ROM range
                uint16_t offset = addr - 0x4000;
                data = rom[offset];
                printf("Debug: RD cycle - SLTSL=%d RD=%d WR=%d ADDR=0x%04X DATA=0x%02X\n",
                    sltsl, rd, wr, addr, data);
                // Drive data onto the bus
                set_data_bus_output(data);

                 // Optionally manipulate WAIT if needed for timing, e.g.:
                //gpio_put(PIN_WAIT, 0); // Add wait state if necessary

                // Wait until the read cycle completes (RD goes high)
                while (gpio_get(PIN_RD) == 0) {
                 tight_loop_contents();
                }

                // Return data lines to input mode after cycle completes
                set_data_bus_input();

                // Release WAIT line if used
                //gpio_put(PIN_WAIT, 1);
            }

        } else {
            // Not a read cycle - ensure data bus is not driven
            set_data_bus_input();
        }

        cycle_count++;

        tight_loop_contents();
    }

    return 0;
}
