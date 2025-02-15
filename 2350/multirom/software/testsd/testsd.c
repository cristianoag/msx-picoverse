

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "diskio.h" 

int main() {
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
