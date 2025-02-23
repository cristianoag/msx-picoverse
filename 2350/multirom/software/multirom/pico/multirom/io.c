#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hw_config.h"
#include "multirom.h"
#include "io.h"

// Global registers (emulating the Nextor driver's registers)
uint8_t ctrl_reg = 0;  // Last value written to port 0x9E (control)
uint8_t data_reg = 0;     // Last SPI response from port 0x9F (data)

void __not_in_flash_func(io_main2)(){
    
    sleep_ms(10000);

    
    // Initialize the microSD card
    sd_card_t *sd_card = sd_get_by_num(0);
    if (sd_card == NULL) {
        printf("Error: SD card not found\n");
        return;
    }

    uint8_t sdManuf = (uint8_t)ext_bits16(sd_card->state.CID, 127, 120); // Manufacturer ID 3 = Sandisk 

    printf("SDCard Manufacturer ID: 0x%02X\n", sdManuf);
    //printf("SDCard OEM ID: 0x%04X\n", sd_card->oem_id);
    //sd_card_detect(sd_card);
    //printf("SDCard Manufacturer ID: 0x%02X\n", sd_card->get_num_sectors);
    //printf("SDCard OEM ID: 0x%04X\n", sd_card->oem_id);
    
    

}


void __not_in_flash_func(io_main)(){

    BYTE const pdrv = 0;  // Physical drive number
    DSTATUS ds = 1; // Disk status (1 = not initialized)

    //sleep_ms(10000);
//    printf("MSX I/O emulation started\n");

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
                // Write transaction: the MSX is writing to the port.

                // Port 0x9E (Control Write): Set the control register.
                if (port == 0x9E)
                {
                    //printf("MSX Write 0x9E: Control=0x%02x\n", busdata);
                    //printf("BUSDATA == 0x01: %d\n", busdata == 1);
                    // 0x01 = SD card initialization
                    if (busdata == 0x01) {
                        if (ds & STA_NOINIT) {
                            // Initialize the SD card if it hasn't been initialized yet
                            ds = disk_initialize(pdrv);
                            if (ds & STA_NOINIT) {
                                //printf("Error: SD card initialization failed\n");
                                ctrl_reg = 0xFF; // Set control register to error state
                            }
                            else {
                                //printf("SD card initialized successfully\n");
                                ctrl_reg = 0x00; // Set control register to success state
                            }
                        }
                        else {
                            //printf("SD card already initialized\n");
                            ctrl_reg = 0x00; // Set control register to success state
                        }
                    }
                    
                    // 0x02 = SD card presence
                    if (busdata == 0x02) {
                        if (!(ds & STA_NOINIT)) {
                            //printf("MSX: SD card is present\n");
                            ctrl_reg = 0x00;
                        }
                        else {
                            //printf("MSX: SD card is not present or not initialized\n");
                            ctrl_reg = 0xFF; // Set control register to error state
                        }
                    }

                    // 0x03 = SD card manufacturer ID set
                    if (busdata == 0x03) {
                        if (!(ds & STA_NOINIT)) {

                            sd_card_t *sd_card = sd_get_by_num(0);
                            ctrl_reg = (uint8_t)ext_bits16(sd_card->state.CID, 127, 120);
                        }
                        else {
                           // printf("MSX: SD card is not present or not initialized\n");
                            ctrl_reg = 0xFF; // Set control register to error state
                        }
                    }

                    // 0x04 = SD card serial number
                    if (busdata == 0x04) {
                        if (!(ds & STA_NOINIT)) {
                            sd_card_t *sd_card = sd_get_by_num(0);
                            // in fact the serial for the microSD card is a 32bit number that goes from 
                            // 24 to 55. we are returning just the first least significant byte.
                            ctrl_reg = (uint8_t)ext_bits16(sd_card->state.CID, 31, 24);
                        }
                        else {
                            //printf("MSX: SD card is not present or not initialized\n");
                            ctrl_reg = 0xFF; // Set control register to error state
                        }
                    }

                    // 0x05 = SD card capacity (number of blocks), returned one byte per call (little-endian)
                    if (busdata == 0x05) {
                        // Use static variables to keep the state across calls.
                        static int capacity_byte_index = 0;
                        static DWORD capacity_cached = 0;

                        if (!(ds & STA_NOINIT)) {
                            if (capacity_byte_index == 0) {
                                // On the first call, query the SD card capacity.
                                DRESULT dr = disk_ioctl(pdrv, GET_SECTOR_COUNT, &capacity_cached);
                                printf("MSX: SD card capacity: %d\n", capacity_cached);
                                if (dr != RES_OK) {
                                    // If there is an error, signal error and reset index.
                                    ctrl_reg = 0xFF;
                                    capacity_byte_index = 0;
                                    break;
                                }
                            }
                            // Return one byte of the capacity (little-endian order)
                            ctrl_reg = (uint8_t)((capacity_cached >> (capacity_byte_index * 8)) & 0xFF);
                            capacity_byte_index = (capacity_byte_index + 1) % 4;
                        }
                        else {
                            // SD card not present or not initialized
                            ctrl_reg = 0xFF;
                        }
                    }
                }
                else if (port == 0x9F) // Port 0x9F (Data Write): Send the byte to the media
                {
                    
                    uint8_t spi_tx = busdata;
                    uint8_t spi_rx = 0;

                    //spi_rx = sfi_send_command(spi, spi_tx);
                    //spi_write_read_blocking(SPI_PORT, &spi_tx, &spi_rx, 1);
                    //spi_rx = sd_spi_write_read(sd_card, spi_tx);

                    data_reg = spi_rx;

                    //printf("MSX Write 0x9F: Data Sent=0x%02x, Received=0x%02x\n", data_reg, spi_handle_data_register(data_reg, true));
                    //printf("MSX Write 0x9F: Data Sent to microSD=0x%02x, Received=0x%02x\n", spi_tx, spi_rx);

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
                    out_val = ctrl_reg;
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

