#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hw_config.h"
#include "lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/sd_driver/SPI/sd_card_spi.h"
#include "lib/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src/sd_driver/sd_card.h"

#include "multirom.h"
#include "io.h"

// Global registers (emulating the Nextor driver's registers)
uint8_t control_reg = 0;  // Last value written to port 0x9E (control)
uint8_t data_reg = 0;     // Last SPI response from port 0x9F (data)
bool sd_changed = false;  // Disk-change flag (set externally when card is replaced)


int sfi_send_command(spi_inst_t *spi, uint8_t cmd)
{
    //printf("Sending SPI command 0x%02x...\n", cmd);
    //uint8_t cmd0[] = {0x40 | 0x00, 0x00, 0x00, 0x00, 0x00, 0x95};

    uint8_t response;

    // Select the SD card by pulling CS low
    gpio_put(SPI_CS, 0);
    spi_write_blocking(spi, &cmd, sizeof(cmd));

    //printf("SPI command sent. Waiting for response...\n");
    // Wait for a response (response should be 0x01 indicating idle state)
    for (int i = 0; i < 8; i++) {
        spi_read_blocking(spi, 0xFF, &response, 1);
        
        if (response != 0xff) {
            // Deselect the SD card by pulling CS high
            gpio_put(SPI_CS, 1);
            return response; // Success
        }
    }

    // Deselect the SD card by pulling CS high
    gpio_put(SPI_CS, 1);
    return 0xff; // Failure

}

uint8_t spi_handle_control_register() {

    uint8_t status = 0x0;

    return status;
}

void __not_in_flash_func(io_main)(){

    spi_inst_t *spi = SPI_PORT;
    spi_init(spi, 100 * 1000); // Initialize SPI at 100 kHz
    gpio_set_function(SPI_SCK, GPIO_FUNC_SPI); // SCK
    gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI); // MOSI
    gpio_set_function(SPI_MISO, GPIO_FUNC_SPI); // MISO

    // Initialize CS pin
    gpio_init(SPI_CS);
    gpio_set_dir(SPI_CS, GPIO_OUT);
    gpio_put(SPI_CS, 1); // Keep CS high

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

                    spi_rx = sfi_send_command(spi, spi_tx);
                    //spi_write_read_blocking(SPI_PORT, &spi_tx, &spi_rx, 1);

                    data_reg = spi_rx;

                    //printf("MSX Write 0x9F: Data Sent=0x%02x, Received=0x%02x\n", data_reg, spi_handle_data_register(data_reg, true));
                    printf("MSX Write 0x9F: Data Sent to microSD=0x%02x, Received=0x%02x\n", busdata, spi_rx);

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
                    out_val = spi_handle_control_register();
                    //printf("MSX Read 0x9E: Control=0x%02x\n", out_val);
                }
                else if (port == 0x9F)
                {
                    // Port 0x9F (Data Read): Return the last SPI response.
                    out_val = data_reg;
                }

                if ((port == 0x9E) || (port == 0x9F))
                {
                    gpio_set_dir(PIN_BUSSDIR, GPIO_OUT);
                    //printf("See the data bus: %d\n", out_val);
                    // Set the data bus to output mode, write the data, and return to input mode.
                    gpio_set_dir_out_masked(0xFF << 16); // Set data bus to output mode
                    gpio_put(PIN_BUSSDIR, 0); // Set the data bus to output mode
                    gpio_put_masked(0xFF0000, out_val << 16); // Write the data to the data bus
                    while (!gpio_get(PIN_RD)) tight_loop_contents();
                    gpio_put(PIN_BUSSDIR, 1); // Set the data bus to output mode
                    gpio_set_dir(PIN_BUSSDIR, GPIO_IN);
                    gpio_set_dir_in_masked(0xFF << 16); // Return data bus to input mode after cycle completes

                    //printf("MSX Read from 0x%02x: Output=0x%02x\n", port, out_val);
                }

            }
        }
        tight_loop_contents();
    }
}

