#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pins.h"

void __no_inline_not_in_flash_func(core1_entry)()
{
    // 128KB of RAM in the Pico to be “mapped” in 16KB banks
    static uint8_t sram_data[128 * 1024];
    memset(sram_data, 0x00, sizeof(sram_data));  // Clear the SRAM

    // Four page registers for 4 pages of 16KB each
    // Each register selects which 16KB “block” is mapped.
    // With 128KB total, we have 8 blocks (0..7).
    // Initialize them to 0,1,2,3 so that at startup you see
    // each 16KB page mapped to a unique block for convenience.
    static uint8_t pageRegister[4] = {0, 1, 2, 3};

    // Continuously respond to the MSX bus
    while (true)
    {
        bool sltsl = !(gpio_get(PIN_SLTSL));  // Cartridge slot select ?????
        bool iorq  = !(gpio_get(PIN_IORQ));   // I/O request
        bool rd    = !(gpio_get(PIN_RD));     // Read strobe
        bool wr    = !(gpio_get(PIN_WR));     // Write strobe


        // Full 16-bit address (A0..A15)
        uint32_t addr_bus = gpio_get_all() & 0xFFFF;  
        // For an I/O operation, typically the low 8 bits are the port number.
        uint8_t  port     = (uint8_t)(addr_bus & 0xFF);

        // Cartridge is only active if SLTSL is asserted
        if (!sltsl)
            continue;


        // The MSX memory mapper uses I/O ports 0xFC..0xFF:
        //   - Writing to these ports sets the page register
        //   - Reading from these ports returns the current page register
        // Because we have only 128KB, we only need 3 bits in each register,
        // but we’ll read and write the entire byte from the MSX side
        // and simply mask off the extra bits.
        if (iorq)
        {
            // I/O Write: set page register
            if (wr && (port >= 0xFC && port <= 0xFF))
            {
                gpio_set_dir_in_masked(0xFF << 16); // Set data bus to input mode
                uint8_t data = (gpio_get_all() >> 16) & 0xFF;  // Get the written value on D0..D7
                uint8_t page = port - 0xFC;   // Which page register? port 0xFC => page 0, 0xFD => page 1, etc.
                pageRegister[page] = data & 0x07;  // Only 8 banks in 128KB
                while (!(gpio_get(PIN_WR))) {                 // Wait for WR to go high again (to complete the bus cycle)

                    tight_loop_contents();
                }
            }
            // I/O Read: read current page register
            else if (rd && (port >= 0xFC && port <= 0xFF))
            {
                gpio_set_dir_out_masked(0xFF << 16); // Set data bus to output mode
                uint8_t page = port - 0xFC;
                uint8_t data = pageRegister[page];
                gpio_put_masked(0xFF0000, data << 16); // Write the data to the data bus
                while (!(gpio_get(PIN_RD))) {                 // Wait for RD to go high again

                    tight_loop_contents();
                }
                gpio_set_dir_in_masked(0xFF << 16); // Set data bus to input mode
            }
        }
        else
        {
            // The MSX sees this slot as normal RAM. 
            // Physical address range: 0000h..FFFFh => 4 pages of 16KB each.
            // Identify which 16KB page we are in:
            //   page = 0 => 0x0000..0x3FFF
            //   page = 1 => 0x4000..0x7FFF
            //   page = 2 => 0x8000..0xBFFF
            //   page = 3 => 0xC000..0xFFFF
            // Then map that to sram_data[] offset.
            uint8_t page = (addr_bus >> 14) & 0x03;  // top 2 bits
            uint8_t bank = pageRegister[page];       // which 16KB block for this page

            // offset within that bank
            uint16_t offset_in_bank = addr_bus & 0x3FFF;
            // global offset = bank * 16KB + offset_in_bank
            uint32_t sram_offset = (bank * 0x4000) + offset_in_bank;
            if (rd)
            {
                gpio_set_dir_out_masked(0xFF << 16); // Set data bus to output mode
                uint8_t data = sram_data[sram_offset];  // Read from our local 128KB array
                gpio_put_masked(0xFF0000, data << 16); // Write the data to the data bus
                while (!(gpio_get(PIN_RD))) {     // Wait for RD to go high

                    tight_loop_contents();
                }
            }
            else if (wr)
            {
                gpio_set_dir_in_masked(0xFF << 16); // Set data bus to input mode
                uint8_t data = (gpio_get_all() >> 16) & 0xFF;  // Read the byte being written
                sram_data[sram_offset] = data;   // Store into our local 128KB array
                while (!(gpio_get(PIN_WR))) {  // Wait for WR to go high
                    tight_loop_contents();
                }
            }
        }
    }
}