// MSX PICOVERSE PROJECT
// (c) 2024 Cristiano Goncalves
// The Retro Hacker
//
// loadrom.c - Windows console application to create a loadrom UF2 file for the MSX PICOVERSE 2040
//
// This program creates a UF2 file to program the Raspberry Pi Pico with the MSX PICOVERSE 2040 loadROM firmware. The UF2 file is
// created with the combined PICO firmware binary file, the configuration file and the ROM file. The configuration file contains the
// information of the ROM file processed by the tool so the MSX can have the required information to load the ROM and execute.
// 
// The configuration record has the following structure:
//  game - Game name                            - 20 bytes (padded by 0x00)
//  mapp - Mapper code                          - 01 byte  (0x01: 16KB, 0x02: 32KB, 0x03: KonamiSCC, 0x04: Linear0)
//  size - Size of the ROM in bytes             - 04 bytes 
//  offset - Offset of the game in the flash    - 04 bytes 
//
// This work is licensed  under a "Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License. 
// https://creativecommons.org/licenses/by-nc-sa/4.0/
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <dirent.h>
#include "uf2format.h"

#define CONFIG_FILE     "loadrom.cfg"          // this is the 29 bytes file with the information about the ROM to load
#define COMBINED_FILE   "loadrom.cmb"          // this is the final binary file with the firmware, configuration and ROM
#define PICOFIRMWARE    "loadrom.bin"          // this is the Raspberry PI Pico firmware binary file
#define UF2FILENAME     "loadrom.uf2"          // this is the UF2 file to program the Raspberry Pi Pico

#define MAX_FILE_NAME_LENGTH    20             // Maximum length of a ROM name
#define MIN_ROM_SIZE            8192           // Minimum size of a ROM file
#define MAX_ROM_SIZE            131072         // Maximum size of a ROM file
#define FLASH_START             0x10000000     // Start of the flash memory on the Raspberry Pi Pico


uint32_t file_size(const char *filename);
uint8_t detect_rom_type(const char *filename);
void write_padding(FILE *file, size_t current_size, size_t target_size, uint8_t padding_byte);
void create_uf2_file(const char *combined_filename, const char *uf2_filename);

const char *rom_types[] = {
    "Unknown ROM type", // Default for invalid indices
    "Plain16",          // Index 1
    "Plain32",          // Index 2
    "KonamiS",          // Index 3
    "Linear0",          // Index 4
    "ASCII8",           // Index 5
    "ASCII16",          // Index 6
    "Konami"            // Index 7

};

// Function to get the size of a file
// Parameters:
// filename - Name of the file
// Returns:
// Size of the file in bytes
uint32_t file_size(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return 0;
    }
    fseek(file, 0, SEEK_END);
    uint32_t size = ftell(file);
    fseek(file, 0L, SEEK_SET);
    fclose(file);
    return size;
}

// Detect the ROM type by analyzing the AB signature at 0x0000 and 0x0001 or 0x4000 and 0x4001
uint8_t detect_rom_type(const char *filename) {
    
    size_t size = file_size(filename);
    
    if (size > MAX_ROM_SIZE || size < MIN_ROM_SIZE) {
        return 0; // unknown mapper
    }

    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Failed to open ROM file");
        return 0; // unknown mapper
    }

    uint8_t rom[MAX_ROM_SIZE];
    fread(rom, 1, size, file);
    fclose(file);
    
    // Check if the ROM has the signature "AB" at 0x0000 and 0x0001
    // Those are the cases for 16KB and 32KB ROMs
    if (rom[0] == 'A' && rom[1] == 'B' && size == 16384) {
        return 1;     // Plain 16KB 
    }
    if (rom[0] == 'A' && rom[1] == 'B' && size == 32768) {
        return 2;     // Plain 32KB 
    }
    // Check if the ROM has the signature "AB" at 0x4000 and 0x4001
    // That is the case for 48KB ROMs with Linear page 0 config
    if (rom[0x4000] == 'A' && rom[0x4001] == 'B') {
        return 4; // Linear0 48KB
    }

    // Konami SCC
    for (size_t i = 0; i < size - 16; i++) {
        if (rom[i] == 0x50 && rom[i + 1] == 0x50 && rom[i + 2] == 0x50) {
            return 3;
        }
    }

    // ASCII 8KB
    if (rom[0] == 'A' && rom[1] == 'S' && rom[2] == 'C' && rom[3] == '8') {
        return 5;
    }

    // ASCII 16KB
    if (rom[0] == 'A' && rom[1] == 'S' && rom[2] == 'C' && rom[3] == '1') {
        return 6;
    }

    // Konami without SCC
    if (rom[0] == 'K' && rom[1] == 'O' && rom[2] == 'N' && rom[3] == 'A') {
        return 7;
    }

    return 0; 
}


// create_uf2_file - Create the UF2 file
// This function will create the UF2 file with the firmware, menu and ROM files
void create_uf2_file(const char *combined_filename, const char *uf2_filename) {
    
    
    // Get the size of the combined file
    uint32_t sz = file_size(combined_filename);

    // Open the combined file
    FILE *combined_file = fopen(combined_filename, "rb");
    if (!combined_file) {
        perror("Failed to open the combined file");
        return;
    }

    // Create/Open for writing the UF2 file
    FILE *uf2_file = fopen(uf2_filename, "wb");
    if (!uf2_file) {
        perror("Failed to create UF2 file");
        fclose(combined_file);
        return;
    }

    UF2_Block bl;
    memset(&bl, 0, sizeof(bl));

    bl.magicStart0 = UF2_MAGIC_START0;
    bl.magicStart1 = UF2_MAGIC_START1;
    bl.flags = 0x00002000;
    bl.magicEnd = UF2_MAGIC_END;
    bl.targetAddr = FLASH_START;
    bl.numBlocks = (sz + 255) / 256;
    bl.payloadSize = 256;
    bl.fileSize = 0xe48bff56;
    
    int numbl = 0;
    while (fread(bl.data, 1, bl.payloadSize, combined_file)) {
        bl.blockNo = numbl++;
        fwrite(&bl, 1, sizeof(bl), uf2_file);
        bl.targetAddr += bl.payloadSize;
        // clear for next iteration, in case we get a short read
        memset(bl.data, 0, sizeof(bl.data));
    }
    fclose(uf2_file);
    fclose(combined_file);
    printf("\nSuccessfully wrote %d blocks to %s.\n", numbl, uf2_filename);
    //printf("%s file created successfully.\n", UF2FILENAME);

}

// Main function
int main(int argc, char *argv[])
{
    printf("MSX PICOVERSE 2040 LoadROM UF2 Creator v1.0\n");
    printf("(c) 2024 The Retro Hacker\n\n");

    if (argc < 2) {
        printf("Usage: loadrom <romfile>\n");
        return 1;
    }

    // Open the PICO firmware binary file
    FILE *input_file = fopen(PICOFIRMWARE, "rb");
    if (!input_file) {
        perror("Failed to open PICO firmware binary file");
        return 1;
    }

    // Create the final output file
    FILE *output_file = fopen(COMBINED_FILE, "wb");
    if (!output_file) {
        perror("Failed to create final output file");
        fclose(input_file);
        return 1;
    }

    // Copy the PICO firmware binary file to the final output file
    uint8_t buffer[1024];
    size_t bytes_read;
    size_t total_bytes_written = 0;
    size_t current_size = 0; // Current size of the output file
    uint32_t base_offset = 0x1d; // Base offset for the ROM file = 29B (one config record)

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), input_file)) > 0) {
        fwrite(buffer, 1, bytes_read, output_file);
    }
    fclose(input_file);

    // Open the ROM file
    FILE *rom_file = fopen(argv[1], "rb");
    if (!rom_file) {
        perror("Failed to open ROM file");
        fclose(output_file);
        return 1;
    }

    // Write the ROM name to the configuration file
    if ((strstr(argv[1], ".ROM") != NULL) || (strstr(argv[1], ".rom") != NULL)) // Check if it is a .ROM file
    {
        // Get the size of the ROM file
        uint32_t rom_size = file_size(argv[1]);
        if (rom_size == 0 || rom_size > MAX_ROM_SIZE) {
            perror("Failed to get the size of the ROM file or size not supported");
            fclose(rom_file);
            fclose(output_file);
            return 1;
        }

        // Detect the ROM type
        uint8_t rom_type = detect_rom_type(argv[1]);
        if (rom_type == 0) {
            perror("Failed to detect the ROM type");
            fclose(rom_file);
            fclose(output_file);
            return 1;
        }

        // Write the ROM name to the configuration file
        char rom_name[MAX_FILE_NAME_LENGTH] = {0};
        uint32_t fl_offset = base_offset;

        // Extract the first part of the file name (up to the first '.ROM' or '.rom')
        char *dot_position = strstr(argv[1], ".ROM");
        if (dot_position == NULL) {
            dot_position = strstr(argv[1], ".rom");
        }
        if (dot_position != NULL) {
            size_t name_length = dot_position - argv[1];
            if (name_length > MAX_FILE_NAME_LENGTH) {
                name_length = MAX_FILE_NAME_LENGTH;
            }
            strncpy(rom_name, argv[1], name_length);
        } else {
            strncpy(rom_name, argv[1], MAX_FILE_NAME_LENGTH);
        }

        printf("ROM Name: %s\n", rom_name);
        // Write the file name (20 bytes)
        fwrite(rom_name, 1, MAX_FILE_NAME_LENGTH, output_file);

        printf("ROM Type: %s\n", rom_types[rom_type]);
        // Write the mapper (1 byte)
        fwrite(&rom_type, 1, 1, output_file);

        printf("ROM Size: %u bytes\n", rom_size);
        // Write the file size (4 bytes)
        fwrite(&rom_size, 4, 1, output_file);

        printf("Flash Offset: 0x%08X\n", fl_offset);
        // Write the flash offset (4 bytes)
        fwrite(&fl_offset, 4, 1, output_file);

        // Write the ROM file to the final output file
        while ((bytes_read = fread(buffer, 1, sizeof(buffer), rom_file)) > 0) {
            fwrite(buffer, 1, bytes_read, output_file);
        }
        fclose(rom_file);
        fclose(output_file);

        //create the uf2 file
        create_uf2_file(COMBINED_FILE, UF2FILENAME);

    } else
    {
        perror("Invalid ROM file");
        fclose(rom_file);
        fclose(output_file);
        return 1;
    }

    

}