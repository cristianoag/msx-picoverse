
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "diskio.h"
#include "multirom.h"
#include "io.h"

uint8_t __not_in_flash_func(msx_read_data)(void) 
{
    return (gpio_get_all() >> 16) & 0xFF;
}

void __not_in_flash_func(io_main)() {
    sleep_ms(10000);
    printf("Initializing the microSD card\n");

    BYTE const pdrv = 0;                // Physical drive number, we only have one microSD
    DSTATUS ds = disk_initialize(pdrv); // Initialize the microSD card

    if (0 == (ds & STA_NOINIT)) {
        printf("disk_initialize(%d) succeeded\n", pdrv);
    } else {
        panic("disk_initialize(%d) failed\n", pdrv);
    }

    // Get the number of sectors on the drive
    // not required now, but just to make sure the microSD is working ok
    DWORD sz_drv = 0;
    DRESULT dr = disk_ioctl(pdrv, GET_SECTOR_COUNT, &sz_drv);

    if (RES_OK == dr) {
        printf("disk_ioctl(%d, GET_SECTOR_COUNT, &sz_drv) succeeded\n", pdrv);
        printf("Number of sectors on the drive: %d\n", sz_drv);
    } else {
        panic("disk_ioctl(%d, GET_SECTOR_COUNT, &sz_drv) failed\n", pdrv);
    }

    // Create a type for a 512-byte block
    typedef char block_t[512];

    // State variables for accumulating the sector address
    bool waiting_for_address = false;
    uint8_t pending_cmd = 0;
    uint32_t sector_addr = 0;
    int byte_count = 0;

    // New state variables for transfer phase after address has been received
    bool waiting_for_count = false;  // Waiting for the next data port access to get count
    bool in_transfer = false;        // Transfer in progress (read or write)
    bool transfer_is_read = false;   // true for read, false for write
    uint8_t remaining_sectors = 0;   // How many sectors to transfer
    uint16_t transfer_idx = 0;       // Byte index within the current sector
    uint8_t block_buffer[512];       // Temporary buffer for one sector

    while (true) {
        bool iorq = !gpio_get(PIN_IORQ);
        if (iorq) {
            uint32_t gpiostates = gpio_get_all();
            uint8_t port = gpiostates & 0xFF;

            if (port == PORT_CMD) {
                // Read command from upper byte of GPIO states
                uint8_t cmd = (gpiostates >> 16) & 0xFF;
                printf("Command port called with command: %d\n", cmd);

                switch (cmd) {
                    case CMD_NEXTOR_READ_SECTOR:
                    case CMD_NEXTOR_WRITE_SECTOR:
                        // Begin accumulating 4 bytes for the sector address
                        pending_cmd = cmd;
                        waiting_for_address = true;
                        waiting_for_count = false;
                        in_transfer = false;
                        sector_addr = 0;
                        byte_count = 0;
                        printf("Awaiting 4 data bytes for sector address...\n");
                        break;
                    default:
                        // Unknown command; reset any pending address read
                        waiting_for_address = false;
                        waiting_for_count = false;
                        in_transfer = false;
                        byte_count = 0;
                        break;
                }
            }
            else if (port == PORT_DATA) {
                // Check if we are still accumulating the 4-byte sector address.
                if (waiting_for_address) {
                    // Each data port access gives one byte (read from upper part)
                    uint8_t data = (gpiostates >> 16) & 0xFF;
                    sector_addr |= ((uint32_t)data << (8 * byte_count));
                    byte_count++;
                    printf("Received data byte %d: 0x%02X (sector_addr now 0x%08X)\n",
                           byte_count, data, sector_addr);

                    if (byte_count >= 4) {
                        waiting_for_address = false;
                        // Next, expect the sector count coming through the data port.
                        waiting_for_count = true;
                        printf("Sector address complete. Awaiting 1 data byte for sector count...\n");
                    }
                }
                // Now, if we are waiting for the count (number of sectors)
                else if (waiting_for_count) {
                    uint8_t count = (gpiostates >> 16) & 0xFF;
                    waiting_for_count = false;
                    remaining_sectors = count;

                    if (pending_cmd == CMD_NEXTOR_READ_SECTOR) {
                        transfer_is_read = true;
                        in_transfer = true;
                        // Read the first sector into the block buffer.
                        DRESULT dr = disk_read(0, block_buffer, sector_addr, 1);
                        if (dr != RES_OK) {
                            panic("disk_read failed at sector %u", sector_addr);
                        }
                        transfer_idx = 0;
                        printf("Nextor Read Sector: Sector address = %u, Sector count = %u\n", sector_addr, count);
                    }
                    else if (pending_cmd == CMD_NEXTOR_WRITE_SECTOR) {
                        transfer_is_read = false;
                        in_transfer = true;
                        transfer_idx = 0;
                        printf("Nextor Write Sector: Sector address = %u, Sector count = %u\n", sector_addr, count);
                    }
                }
                // If a transfer is already in progress
                else if (in_transfer) {
                    if (transfer_is_read) {
                        // In read mode, each data port access sends one byte of the data.
                        uint8_t out_byte = block_buffer[transfer_idx++];
                        printf("Output data byte: 0x%02X\n", out_byte);

                        if (transfer_idx >= 512) {
                            remaining_sectors--;
                            sector_addr++; // Advance to next sector
                            if (remaining_sectors > 0) {
                                // Read next sector data into buffer.
                                DRESULT dr = disk_read(0, block_buffer, sector_addr, 1);
                                if (dr != RES_OK) {
                                    panic("disk_read failed at sector %u", sector_addr);
                                }
                                transfer_idx = 0;
                            }
                            else {
                                in_transfer = false;
                                printf("Completed reading all sectors\n");
                            }
                        }
                    }
                    else {
                        // Write mode: accumulate each incoming byte until one sector (512 bytes) is received.
                        uint8_t in_byte = (gpiostates >> 16) & 0xFF;
                        block_buffer[transfer_idx++] = in_byte;
                        printf("Received write data byte %d: 0x%02X\n", transfer_idx, in_byte);

                        if (transfer_idx >= 512) {
                            // Write the completed sector to disk.
                            DRESULT dr = disk_write(0, block_buffer, sector_addr, 1);
                            if (dr != RES_OK) {
                                panic("disk_write failed at sector %u", sector_addr);
                            }
                            sector_addr++; // Advance to next sector
                            remaining_sectors--;
                            transfer_idx = 0;
                            if (remaining_sectors == 0) {
                                dr = disk_ioctl(0, CTRL_SYNC, 0);
                                if (dr != RES_OK) {
                                    panic("disk_ioctl CTRL_SYNC failed");
                                }
                                in_transfer = false;
                                printf("Completed writing all sectors\n");
                            }
                        }
                    }
                }
                else {
                    // Data port activity without an active command, address accumulation or transfer
                    printf("Data port called but no pending command\n");
                }
            }
        }
    }
}


void microsd_test() {
    stdio_init_all();

    sleep_ms(10000); 

    printf("Hello, world!\n");

    BYTE const pdrv = 0;  // Physical drive number
    DSTATUS ds = disk_initialize(pdrv);

    if (0 == (ds & STA_NOINIT)) {
        printf("disk_initialize(%d) succeeded\n", pdrv);
    } else {
        panic("disk_initialize(%d) failed\n", pdrv);
    }

    // Get the number of sectors on the drive
    DWORD sz_drv = 0;
    DRESULT dr = disk_ioctl(pdrv, GET_SECTOR_COUNT, &sz_drv);

    if (RES_OK == dr) {
        printf("disk_ioctl(%d, GET_SECTOR_COUNT, &sz_drv) succeeded\n", pdrv);
        printf("Number of sectors on the drive: %d\n", sz_drv);
    } else {
        panic("disk_ioctl(%d, GET_SECTOR_COUNT, &sz_drv) failed\n", pdrv);
    }
    
    // Create a type for a 512-byte block
    typedef char block_t[512];

    // Define a couple of blocks
    block_t blocks[2];

    // Initialize the blocks
    snprintf(blocks[0], sizeof blocks[0], "Hello");
    snprintf(blocks[1], sizeof blocks[1], "World!");

    LBA_t const lba = 0; // Arbitrary

    // Write the blocks
    dr = disk_write(pdrv, (BYTE*)blocks[lba], lba, count_of(blocks));

    if (RES_OK == dr) {
        printf("disk_write(%d, (BYTE*)blocks[%d], %d, %d) succeeded\n", pdrv, lba, lba, count_of(blocks));
    } else {
        panic("disk_write(%d, (BYTE*)blocks[%d], %d, %d) failed\n", pdrv, lba, lba, count_of(blocks));
    }

    // Sync the disk
    dr = disk_ioctl(pdrv, CTRL_SYNC, 0);

    if (RES_OK == dr) {
        printf("disk_ioctl(%d, CTRL_SYNC, 0) succeeded\n", pdrv);
    } else {
        panic("disk_ioctl(%d, CTRL_SYNC, 0) failed\n", pdrv);
    }

    memset(blocks, 0xA5, sizeof blocks);

    // Read the blocks
    dr = disk_read(pdrv, (BYTE*)blocks[lba], lba, count_of(blocks));

    if (RES_OK == dr) {
        printf("disk_read(%d, (BYTE*)blocks[%d], %d, %d) succeeded\n", pdrv, lba, lba, count_of(blocks));
    } else {
        panic("disk_read(%d, (BYTE*)blocks[%d], %d, %d) failed\n", pdrv, lba, lba, count_of(blocks));
    }

    //Verify the data
    if (strcmp(blocks[0], "Hello") == 0) {
        printf("blocks[0] == \"Hello\"\n");
    } else {
        panic("blocks[0] != \"Hello\"\n");
    }

    if (strcmp(blocks[1], "World!") == 0) {
        printf("blocks[1] == \"World!\"\n");
    } else {
        panic("blocks[1] != \"World!\"\n");
    }

    printf("Goodbye, world!\n");
    for (;;);
}