#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hw_config.h"
#include "multirom.h"
#include "io.h"

void __not_in_flash_func(io_main)(){

    uint8_t  data_buffer[512];
    uint16_t data_to_send = 0;
    uint16_t data_to_receive = 0;
    uint16_t data_byte_index = 0;

    uint8_t ctrl_to_receive = 0;
    uint32_t block_address = 0;
    uint8_t block_number = 0;

    bool block_read = false;
    bool block_write = false;

    uint8_t ctrl_reg = 0;  // Last value written to port 0x9E (control)
    uint8_t data_reg = 0;     // Last SPI response from port 0x9F (data)

    BYTE const pdrv = 0;  // Physical drive number
    DSTATUS ds = 1; // Disk status (1 = not initialized)

    while (true) {
        
        

        bool iorq  = !gpio_get(PIN_IORQ);
        bool sltsl = !gpio_get(PIN_SLTSL);

        if ((iorq) && (!sltsl)){ 
            uint32_t gpiostates = gpio_get_all();
            uint8_t busdata = (gpiostates >> 16) & 0xFF;
            uint8_t port = gpiostates & 0xFF;
            
            bool wr = !gpio_get(PIN_WR);
            bool rd = !gpio_get(PIN_RD);
            if (wr)
            {
                // Write transaction: the MSX is writing to the port.

                // Port 0x9E (Control Write): Set the control register.
                if (port == 0x9E)
                {
                    // this is to receive the address to read/write from/to the SD card from cmd_06 and cmd_08
                    // when called first time, next 4 writes will have the 32 bit address of the block to read/write
                    if (ctrl_to_receive > 0) // here we need to receive the address to read/write from/to the SD card
                    {
                        // On the next calls, receive the address to read from the SD card
                        // and set the data_to_send to 512 bytes (4096 bits)
                        block_address = (block_address << 8) | busdata; // Shift left and add the new byte
                        ctrl_to_receive--;
                        if (ctrl_to_receive == 1) {
                            block_number = busdata;
                            printf("Number of blocks to read/write: %d\n", block_number);
                        }
                        if (ctrl_to_receive == 0) {
                                //printf("MSX: SD card block address: %d\n", block_address);
                                block_read = true;
                                block_write = true;
                        }
                    }

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

                    // 0x03 = SD card manufacturer ID 
                    if (busdata == 0x03) {
                        if (!(ds & STA_NOINIT)) {
                            sd_card_t *sd_card = sd_get_by_num(0);
                            ctrl_reg = (uint8_t)ext_bits16(sd_card->state.CID, 127, 120);
                            //printf("MSX: SD card manufacturer ID: 0x%02X\n", ctrl_reg);
                        }
                        else {
                           // printf("MSX: SD card is not present or not initialized\n");
                            ctrl_reg = 0xFF; // Set control register to error state
                        }
                    }

                    // 0x04 = SD card serial number
                    if (busdata == 0x04) {
                        if (!(ds & STA_NOINIT)) {
                            if (data_to_send == 0) {
                                // On the first call, query the SD card serial number and store on the data buffer
                                // set the data_to_send to 4 bytes (32 bits)
                                memset(data_buffer, 0, 32);
                                sd_card_t *sd_card = sd_get_by_num(0);
                                DWORD serial = ext_bits16(sd_card->state.CID, 55, 24);
                                //printf("MSX: SD card serial number: %d\n", serial);
                                memcpy(data_buffer, &serial, 4);
                                data_to_send = 4;
                                data_byte_index = 0;
                            }

                        }
                        else {
                            //printf("MSX: SD card is not present or not initialized\n");
                            ctrl_reg = 0xFF; // Set control register to error state
                        }
                    }

                    // 0x05 = SD card capacity (number of blocks), returned one byte per call (little-endian)
                    if (busdata == 0x05) {
                        // Use static variables to keep the state across calls.
                        DWORD capacity = 0;

                        if (!(ds & STA_NOINIT)) {
                            if (data_to_send == 0) {
                                memset(data_buffer, 0, 32);
                                // On the first call, query the SD card capacity and store on the data buffer
                                // set the data_to_send to 4 bytes (32 bits)
                                DRESULT dr = disk_ioctl(pdrv, GET_SECTOR_COUNT, &capacity); // Get the capacity of the SD card
                                //printf("MSX: SD card capacity: %d\n", capacity); 
                                if (dr != RES_OK) {
                                    // If there is an error, signal error and reset index.
                                    ctrl_reg = 0xFF;
                                    data_to_send = 0;
                                    break;
                                }
                                else {
                                    //printf("Sending 4 bytes of SD card capacity: %d\n", capacity);
                                    data_to_send = 4;        // 4 bytes (32 bits)
                                    data_byte_index = 0;     // Reset index
                                    memcpy(data_buffer, &capacity, 4); // Copy capacity to data buffer
                                    //for (int i = 0; i < 4; i++) {
                                    //    printf("data_buffer[%d]: %d\n", i, data_buffer[i]);
                                   // }
                                }
                            }

                        }
                        else {
                            // SD card not present or not initialized
                            ctrl_reg = 0xFF;
                        }
                    }

                    // 0x06 = Read an specific SD card block with 512 bytes in size
                    // when called first time, next 4 writes will have the 32 bit address of the block to read
                    // then, the next 512 reads will return the data from the block
                    if (busdata == 0x06) {
                        if (!(ds & STA_NOINIT)) {
                            if (!block_read) {
                                // On the first call, set the ctrl_to_receive to 4 as we are expecting 4 bytes (32 bits)
                                // for the address of the block to read from the SD card
                                // set the data_to_send to 4 bytes (32 bits)
                                ctrl_to_receive = 4;
                            }
                            else // here we need to read the data from the SD card to the buffer
                            {
                                // On the next call, read the data from the SD card to the buffer and set the data_to_send to 512 bytes (4096 bits)
                                //memset(data_buffer, 0, 512); // Clear data buffer
                                DRESULT dr = disk_read(pdrv, (BYTE*)data_buffer, block_address, 1); // Read one sector from the SD card
                                if (dr != RES_OK) {
                                    // If there is an error, signal error and reset index.
                                    ctrl_reg = 0xFF;
                                    data_to_send = 0;
                                    break;
                                }
                                else {
                                    data_to_send = 512; // Set data to send to 512 bytes (4096 bits)
                                    data_byte_index = 0; // Reset index
                                    block_read = false; // Reset block read flag

                                    // debug print the data buffer
                                    //printf("MSX: SD card block data for block %d:\n", block_address);
                                    /*for (int i = 0; i < 512; i += 16) {
                                        // Print the address (in hexadecimal, 4 digits)
                                       printf("%04X: ", i);
                                        // Print 16 bytes per line
                                        for (int j = 0; j < 16; j++) {
                                                printf("%02X ", data_buffer[i + j]);
                                        }
                                        printf("\n");
                                    }*/

                                }
                            }
                        }
                        else {
                            // SD card not present or not initialized
                            ctrl_reg = 0xFF;
                        }
                        
                    }

                    // 0x07 = Read the next card block with 512 bytes in size
                    // can only be executed after the 0x06 command
                    if (busdata == 0x07) {
                        if (!(ds & STA_NOINIT)) {
                            //memset(data_buffer, 0, 512);
                            block_address++;
                            DRESULT dr = disk_read(pdrv, (BYTE*)data_buffer, block_address, 1); // Read one sector from the SD card
                            if (dr != RES_OK) {
                                // If there is an error, signal error and reset index.
                                ctrl_reg = 0xFF;
                                data_to_send = 0;
                                break;
                            }
                            else {
                                data_to_send = 512; // Set data to send to 512 bytes (4096 bits)
                                data_byte_index = 0; // Reset index

                                 // debug print the data buffer
                                 //printf("MSX: SD card block data for block %d:\n", block_address);
                                 //for (int i = 0; i < 512; i += 16) {
                                     // Print the address (in hexadecimal, 4 digits)
                                     //printf("%04X: ", i);
                                     // Print 16 bytes per line
                                     //for (int j = 0; j < 16; j++) {
                                     //        printf("%02X ", data_buffer[i + j]);
                                    // }
                                    // printf("\n");
                                // }
                            }
                        }
                        else {
                            // SD card not present or not initialized
                            ctrl_reg = 0xFF;
                        }
                    }

                    // 0x08 = Write a 512 byte block to the SD card
                    if (busdata == 0x08) {
                        if (!(ds & STA_NOINIT)) {
                            if (!block_write) {
                                // On the first call, set the ctrl_to_receive to 4 as we are expecting 4 bytes (32 bits)
                                // for the address of the block to write to the SD card
                                // set the data_to_send to 4 bytes (32 bits)
                                //printf("First call, will collect the address to write to the SD card\n");
                                ctrl_to_receive = 4;
                            }
                            else
                            {
                                //printf("Lets write the sector to the microSD card\n");
                                //printf("Now you need to transfer the buffer using port 0x9f\n");
                                block_write = true;
                                data_to_receive = 512; // Set data to send to 512 bytes (4096 bits)
                                data_byte_index = 0; // Reset index
                            }

                        }
                        else {
                            // SD card not present or not initialized
                            ctrl_reg = 0xFF;
                        }
                    }
                    
                }
                else if (port == 0x9F) // Port 0x9F (Data Write): Send the byte to the media
                {
                    if (!(ds & STA_NOINIT)) {
                        //we are receiving an out on port 0x9f to receive data from the MSX and write to SD card
                        if (data_to_receive > 0) {
                            data_buffer[data_byte_index] = busdata; // Store the data in the buffer
                            //printf("Index: %d, Data: 0x%02x\n", data_byte_index, busdata);
                            data_byte_index++; // Increment the buffer index
                            data_to_receive--; // Decrement the data to receive
                        }

                        // if we don't have any more data to receive, and the buffer is full, write the block to the SD card
                        if ((data_to_receive == 0) && (block_write)) {
                                printf("MSX: Writing block %d to SD card\n", block_address);
                                /*printf("Data buffer to write to SD card:\n");
                                for (int i = 0; i < 512; i += 16) {
                                        // Print the address (in hexadecimal, 4 digits)
                                       printf("%04X: ", i);
                                        // Print 16 bytes per line
                                        for (int j = 0; j < 16; j++) {
                                                printf("%02X ", data_buffer[i + j]);
                                        }
                                        printf("\n");
                                    }*/
                                DRESULT dr = disk_write(pdrv, (BYTE*)data_buffer, block_address, 1); // Write one sector to the SD card
                                if (dr != RES_OK) {
                                    // If there is an error, signal error and reset index.
                                    ctrl_reg = 0xFF;
                                }
                                block_write = false; // Reset block read flag
                            }
                        
                        //printf("MSX Write 0x9F: Data Sent=0x%02x, Received=0x%02x\n", data_reg, spi_handle_data_register(data_reg, true));
                        //printf("MSX Write 0x9F: Data Sent to microSD=0x%02x, Received=0x%02x\n", spi_tx, spi_rx);
                    }

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
                    if (data_to_send > 0) {
                        // Return the next byte of the capacity (little-endian order)
                        out_val = data_buffer[data_byte_index];
                        data_byte_index++;
                        data_to_send--;
                        //printf("MSX Read 0x9E: Control=0x%02x\n", out_val);
                    }
                    else {
                        // No extra data to send, return just the control register value.
                        out_val = ctrl_reg;
                    }
                    //printf("MSX Read 0x9E: Control=0x%02x\n", out_val);
                }
                else if (port == 0x9F)
                {
                    if (data_to_send > 0) {
                        // Return the next byte of the data buffer
                        out_val = data_buffer[data_byte_index];
                        data_byte_index++;
                        data_to_send--;
                    }
                    else {
                        // No extra data to send, return just the last SPI response.
                        out_val = data_reg;
                    }
                }

                if ((port == 0x9E) || (port == 0x9F))
                {
                    gpio_set_dir_out_masked(0xFF << 16); // Set data bus to output mode
                    gpio_put_masked(0xFF0000, out_val << 16); // Write the data to the data bus
                    while (!gpio_get(PIN_RD)) tight_loop_contents();
                    gpio_set_dir_in_masked(0xFF << 16); // Return data bus to input mode after cycle completes

                }

            }
        }
        tight_loop_contents();
    }
}

