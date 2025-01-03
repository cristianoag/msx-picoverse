// MSX PICOVERSE PROJECT
// (c) 2024 Cristiano Goncalves
// The Retro Hacker
//
// multirom.c - Windows console application to create a multirom binary file for the MSX PICOVERSE 2040
//
// This is a program to create a UF2 file to program the MSX PICOVERSE 2040 with multiple ROM files. It reads all the .ROM files in the current directory 
// and creates a binary file with a maximum of 256 records of 29 bytes. Each record represents one ROM processed by the tool. The resultant binary file 
// has 7424 bytes and is appended right after the main firmware that runs on the Raspberry Pi Pico.
// Each record has the following structure:
//  game - Game name                            - 20 bytes (padded by 0x00)
//  mapp - Mapper code                          - 01 byte  (0x01: 16KB, 0x02: 32KB, 0x03: Konami, 0x04: Linear0)
//  size - Size of the game in bits             - 4 bytes 
//  offset - Offset of the game in the flash    - 4 bytes 
//
// Offset is calculated by the size of the firmware + 7424 bytes.
// The program will pad the binary file with 0x00 bytes if the file size is less than 7424 bytes.
// The program will print the information of each ROM file processed.
// 
// The program is intended to be used on Windows, but it can be easily ported to other platforms.
// 
// This work is licensed  under a "Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
// License". https://creativecommons.org/licenses/by-nc-sa/4.0/
#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include "uf2format.h"

#define CONFIG_FILE     "multirom.cfg"          // this is the 7424 (256 * 29) bytes file with the list of ROMs and their information
#define COMBINED_FILE   "multirom.cmb"          // this is the final binary file with the firmware, menu and ROMs
#define MENU_FILE       "multirom.msx"          // this is the 32KB MSX MENU ROM file
#define PICOFIRMWARE    "multirom.bin"          // this is the Raspberry PI Pico firmware binary file
#define UF2FILENAME     "multirom.uf2"          // this is the UF2 file to program the Raspberry Pi Pico

#define MAX_FILE_NAME_LENGTH    20          // Maximum length of a ROM name
#define TARGET_FILE_SIZE        32768       // Size of the combined MSX MENU ROM and the configuration file
#define FLASH_START             0x10000000  // Start of the flash memory on the Raspberry Pi Pico


// Structure to store file information
// This struct will be used to store the information of each ROM file processed by the tool
// The information will be used to create the configuration file and to append the ROM files to the final binary file in the exact
// same order as they were processed by the tool.
typedef struct {
    char file_name[256];    // File name
    uint32_t file_size;     // File size
} FileInfo;

void create_uf2_file(const char *combined_filename, const char *uf2_filename);
void write_padding(FILE *file, size_t current_size, size_t target_size, uint8_t padding_byte);
uint32_t file_size(const char *filename);
uint8_t discover_mapper(const char *filename);

// Function to write padding to the file
// This function will write padding bytes to the file to reach the target size
// Parameters:
// file - File pointer
// current_size - Current size of the file
// target_size - Desired target size of the file
// padding_byte - Byte to use as padding
void write_padding(FILE *file, size_t current_size, size_t target_size, uint8_t padding_byte) {
    size_t padding_size = target_size - current_size;

    for (size_t i = 0; i < padding_size; i++) {
        fwrite(&padding_byte, 1, 1, file);
    }
}

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
    fclose(file);
    return size;
}

// Function to discover the mapper of the ROM file
// At this moment, the function only checks if the file size is 16KB, 32KB or 48KB
// Need to modify to detect the mapper type
uint8_t discover_mapper(const char *filename) {
    uint8_t one_byte = 0;
    uint32_t size = file_size(filename);

    switch (size) {
        case 16384:
            one_byte = 1; // 16KB mapper
            break;
        case 32768:
            one_byte = 2; // 32KB mapper
            break;
        case 49152:
            one_byte = 4; // 32KB mapper
            break;
        default:
            one_byte = 9; // unknown mapper
            break;
    }

    return one_byte;
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
    printf("Wrote %d blocks to %s\n", numbl, uf2_filename);
    printf("UF2 file created successfully.\n");

}

// Main function
int main()
{
    printf("MSX PICOVERSE 2040 MultiROM UF2 Creator v1.0\n");
    printf("(c) 2024 The Retro Hacker\n\n");

    DIR *dir;  // Directory pointer    
    struct dirent *entry; // Directory entry
    FILE *output_file, *final_output_file, *input_file; // File pointers
    size_t current_size = 0; // Current size of the output file
    int file_index = 1; // Index of the ROM file
    uint32_t FIRMWARE_BINARY_SIZE = file_size(PICOFIRMWARE); // Size of the PICO firmware binary file
    uint32_t base_offset = FLASH_START + FIRMWARE_BINARY_SIZE + TARGET_FILE_SIZE; // Base offset for the ROM files = Firmware + 32KB MSX MENU
    FileInfo files[256]; // Array to store file information
    int file_count = 0; // Number of ROM files processed

    // Create CONFIG_FILE file 
    output_file = fopen(CONFIG_FILE, "wb");
    if (!output_file) {
        printf("Failed to create the %s file!", CONFIG_FILE);
        return 1;
    }
    
    dir = opendir("."); // Open the current directory
    if (!dir) {
        printf("Failed to open directory!");
        fclose(output_file);
        return 1;
    }

    // Process all rom files on the folder
    while ((entry = readdir(dir)) != NULL) 
    {
        if ((strstr(entry->d_name, ".ROM") != NULL) || (strstr(entry->d_name, ".rom") != NULL)) // Check for .ROM files
        { 
            char rom_name[MAX_FILE_NAME_LENGTH] = {0};
            uint32_t rom_size = 0;
            uint32_t fl_offset = base_offset;

            // Extract the first part of the file name (up to the first '.ROM' or '.rom')
            char *dot_position = strstr(entry->d_name, ".ROM");
            if (dot_position == NULL) {
                dot_position = strstr(entry->d_name, ".rom");
            }
            if (dot_position != NULL) {
                size_t name_length = dot_position - entry->d_name;
                if (name_length > MAX_FILE_NAME_LENGTH) {
                    name_length = MAX_FILE_NAME_LENGTH;
                }
                strncpy(rom_name, entry->d_name, name_length);
            } else {
                strncpy(rom_name, entry->d_name, MAX_FILE_NAME_LENGTH);
            }

            // Write the file name (20 bytes)
            fwrite(rom_name, 1, MAX_FILE_NAME_LENGTH, output_file);
            current_size += MAX_FILE_NAME_LENGTH;

            rom_size = file_size(entry->d_name);

            // Write the mapper (1 byte)
             uint8_t mapper_byte = discover_mapper(entry->d_name);
             fwrite(&mapper_byte, 1, 1, output_file);
             current_size += 1;

            // Write the file size (4 bytes)
            fwrite(&rom_size, 4, 1, output_file);
            current_size += 4;

            // Write the flash offset (4 bytes)
            fwrite(&fl_offset, 4, 1, output_file);
            current_size += 4;

            // Print file information
            printf("File %00d: Name = %-20s, Size = %u bytes, Flash Offset = 0x%08X, Mapper = %d\n", file_index, rom_name, rom_size, fl_offset, mapper_byte);

            // Update base offset for the next file
            base_offset += rom_size;

            // Store rom information
            strncpy(files[file_count].file_name, entry->d_name, sizeof(files[file_count].file_name));
            files[file_count].file_size = rom_size;
            file_count++;

            file_index++;
        }
    }

    closedir(dir);
    fclose(output_file);

    // Create the final output file
    final_output_file = fopen(COMBINED_FILE, "wb");
    if (!final_output_file) {
        perror("Failed to create final output file");
        fclose(output_file);
        return 1;
    }

    // Open the PICO firmware binary file to copy to the final output file
    output_file = fopen(PICOFIRMWARE, "rb");
    if (!output_file) {
        perror("Failed to open PICO firmware binary file");
        return 1;
    }

    // Copy the PICO firmware binary file to the final output file
    uint8_t buffer[1024];
    size_t bytes_read;
    size_t total_bytes_written = 0;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), output_file)) > 0) {
        fwrite(buffer, 1, bytes_read, final_output_file);
    }
    fclose(output_file);

    // Open the MENU_FILE to copy to the final output file - 1st file
    output_file = fopen(MENU_FILE, "rb");
    if (!output_file) {
        perror("Failed to open MENU_FILE");
        return 1;
    }

    // Copy only 16KB (16 * 1024 bytes) of the MENU_FILE
    size_t bytes_to_copy = 16 * 1024;
    while (bytes_to_copy > 0 && (bytes_read = fread(buffer, 1, sizeof(buffer), output_file)) > 0) {
        if (bytes_read > bytes_to_copy) {
            bytes_read = bytes_to_copy;
        }
        fwrite(buffer, 1, bytes_read, final_output_file);
        total_bytes_written += bytes_read;
        bytes_to_copy -= bytes_read;
    }
    fclose(output_file);

    // Open the CONFIG_FILE to copy to the final output file
    output_file = fopen(CONFIG_FILE, "rb");
    if (!output_file) {
        perror("Failed to open CONFIG_FILE");
        return 1;
    }

    // Copy the CONFIG_FILE to the final output file
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), output_file)) > 0) {
        fwrite(buffer, 1, bytes_read, final_output_file);
        total_bytes_written += bytes_read;
    }
    fclose(output_file);

    // Pad the remaining space to reach 32KB
    write_padding(final_output_file, total_bytes_written, TARGET_FILE_SIZE, 0xFF);

    // Close the final output file
    fclose(final_output_file);

    // Append the content of each ROM file to the final output file in the same order
    for (int i = 1; i < file_count; i++) {
        input_file = fopen(files[i].file_name, "rb");
        if (!input_file) {
            perror("Failed to open ROM file");
            continue;
        }

        while ((bytes_read = fread(buffer, 1, sizeof(buffer), input_file)) > 0) {
            fwrite(buffer, 1, bytes_read, final_output_file);
        }

        fclose(input_file);
    }

    fclose(final_output_file);
    printf("\nCombined ROM binary file '%s' created successfully.\n", COMBINED_FILE);
    create_uf2_file(COMBINED_FILE, UF2FILENAME);
    return 0;
}