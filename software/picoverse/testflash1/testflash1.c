// MSX PICOVERSE PROJECT
//
// This is small test program that demonstrates how to read the contents of a
// ROM image that is appended to the end of the main program in the PICO flash memory.
// The ROM data is read from the flash memory and dumped to the serial console in hexdump 
// format.
// Author: Cristiano Goncalves

#include <stdio.h>
#include <hardware/flash.h>
#include "pico/stdlib.h"

// This is the size of the KMARE test ROM data in bytes.
#define ROM_SIZE (32*1024)

// This symbol marks the end of the main program in flash.
// The ROM data is concatenated immediately after this point.
extern unsigned char __flash_binary_end;

int main()
{
    // Initialize the pico
    stdio_init_all();

    // Get a pointer to the start of the ROM data
    const uint8_t *rom = (const uint8_t *)&__flash_binary_end;
    
    while (true) {
        // Print the size of the ROM data
        printf("PROGRAM SIZE=%d\n", (int)rom);
        printf("Dumping ROM contents in hexdump format:\n");

        // Dump the ROM data in hexdump format
        for (int i = 0; i < ROM_SIZE; i += 16) {
            // Print the address offset
            printf("%08x  ", i);

            // Print 16 bytes of data in hexadecimal
            for (int j = 0; j < 16 && (i + j) < ROM_SIZE; j++) {
                printf("%02x ", rom[i + j]);
            }

            // Add spacing if the last line has fewer than 16 bytes
            for (int j = ROM_SIZE - i; j < 16 && i + j < ROM_SIZE; j++) {
                printf("   ");
            }

            // Print the ASCII representation of the data
            printf(" |");
            for (int j = 0; j < 16 && (i + j) < ROM_SIZE; j++) {
                char c = rom[i + j];
                if (c >= 32 && c <= 126) {
                    printf("%c", c);  // Printable ASCII character
                } else {
                    printf(".");      // Non-printable character
                }
            }
            printf("|\n");
        }

        printf("\nDump complete. Sleeping before the next iteration...\n");
        sleep_ms(5000); // Pause for 5 seconds before repeating
    }

    return 0;
}
