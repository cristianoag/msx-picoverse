#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "multirom.h"
#include "io.h"

// Global registers (emulating the Nextor driver's registers)
uint8_t control_reg = 0;  // Last value written to port 0x9E (control)
uint8_t data_reg = 0;     // Last SPI response from port 0x9F (data)
bool sd_changed = false;  // Disk-change flag (set externally when card is replaced)


void spi_initialize() {
    spi_init(SPI_PORT, 1000 * 1000); // Initialize SPI at 1 MHz
    gpio_set_function(SPI_SCK, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MISO, GPIO_FUNC_SPI);
    gpio_init(SPI_CS);
    gpio_set_dir(SPI_CS, GPIO_OUT);
    gpio_put(SPI_CS, 1); // Deselect microSD card
}

uint8_t spi_handle_control_register(uint8_t write_value, bool is_write) {

    uint8_t status = 2;

    return status;
}

void __not_in_flash_func(io_main)(){

    spi_initialize();


    while (true) {
        uint32_t gpiostates = gpio_get_all();
        uint8_t busdata = (gpiostates >> 16) & 0xFF;

        bool iorq  = !gpio_get(PIN_IORQ);
        bool sltsl = !gpio_get(PIN_SLTSL);

        if (iorq) { 
            uint8_t port = gpiostates & 0xFF;
            
            bool wr = !gpio_get(PIN_WR);
            bool rd = !gpio_get(PIN_RD);
            if (wr)
            {
                if (port == 0x9E)
                {
                    control_reg = busdata;

                    // Port 0x9E (Control Write): Bit0 controls SD card chip-select.
                    if ((control_reg & 0x01) == 0) {
                        // Assert SD card CS (active low)
                        gpio_put(SPI_CS, 0);
                    } else {
                        // Deassert SD card CS
                        gpio_put(SPI_CS, 1);
                    }
                    printf("MSX Write 0x9E: Control=0x%02x, SPI_CS %s\n", control_reg, ((control_reg & 0x01) == 0) ? "asserted" : "deasserted");
                }
                else if (port == 0x9F) 
                {
                    // Port 0x9F (Data Write): Send the byte over SPI.
                    uint8_t spi_tx = busdata;
                    uint8_t spi_rx = 0;

                    spi_write_read_blocking(SPI_PORT, &spi_tx, &spi_rx, 1);

                    data_reg = spi_rx;

                    //printf("MSX Write 0x9F: Data Sent=0x%02x, Received=0x%02x\n", data_reg, spi_handle_data_register(data_reg, true));
                    printf("MSX Write 0x9F: Data Sent=0x%02x, Received=0x%02x\n", busdata, spi_rx);

                }
                // Wait until the write strobe is released.
                while (!gpio_get(PIN_WR)) tight_loop_contents();

            }
            else if (rd)
            {
                // Read transaction: the MSX is reading from the port.
                uint8_t out_val;

                if (port == 0x9E)
                {
                    out_val = spi_handle_control_register(0, false);
                    printf("MSX Read 0x9E: Control=0x%02x\n", out_val);
                }
                else if (port == 0x9F)
                {
                    // Port 0x9F (Data Read): Return the last SPI response.
                    out_val = data_reg;
                }

                if ((port == 0x9E) || (port == 0x9F))
                {
                    // Set the data bus to output mode, write the data, and return to input mode.
                gpio_set_dir_out_masked(0xFF << 16); // Set data bus to output mode
                gpio_put_masked(0xFF0000, out_val); // Write the data to the data bus
                while (!gpio_get(PIN_RD)) tight_loop_contents();
                gpio_set_dir_in_masked(0xFF << 16); // Return data bus to input mode after cycle completes

                printf("MSX Read from 0x%02x: Output=0x%02x\n", (port == 0) ? 0x9E : 0x9F, out_val);
                }

            }
        }
        tight_loop_contents();
    }
}

