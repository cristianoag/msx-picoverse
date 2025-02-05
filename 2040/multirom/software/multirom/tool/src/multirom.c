// MSX PICOVERSE PROJECT
// (c) 2024 Cristiano Goncalves
// The Retro Hacker
//
// multirom.c - Windows console application to create a multirom binary file for the MSX PICOVERSE 2040
//
// This program creates a UF2 file to program the Raspberry Pi Pico with the MSX PICOVERSE 2040 MultiROM firmware. The UF2 file is
// created with the combined PICO firmware binary file, the MSX MENU ROM file, the configuration file and the ROM files. The 
// configuration file contains the information of each ROM file processed by the tool and it is incorporated into the MENU ROM file 
// so the MSX can read it.
// 
// Each record has the following structure:
//  game - Game name                            - 20 bytes (padded by 0x00)
//  mapp - Mapper code                          - 01 byte  (0x01: 16KB, 0x02: 32KB, 0x03: Konami, 0x04: Linear0)
//  size - Size of the game in bits             - 4 bytes 
//  offset - Offset of the game in the flash    - 4 bytes 
//
// 
// This work is licensed  under a "Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
// License". https://creativecommons.org/licenses/by-nc-sa/4.0/
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <dirent.h>
#include "uf2format.h"

#define CONFIG_FILE     "multirom.cfg"          // this is the 7424 (256 * 29) bytes file with the list of ROMs and their information
#define COMBINED_FILE   "multirom.cmb"          // this is the final binary file with the firmware, menu and ROMs
#define MENU_FILE       "multirom.msx"          // this is the 32KB MSX MENU ROM file
#define PICOFIRMWARE    "multirom.bin"          // this is the Raspberry PI Pico firmware binary file
#define UF2FILENAME     "multirom.uf2"          // this is the UF2 file to program the Raspberry Pi Pico

#define MAX_FILE_NAME_LENGTH    20              // Maximum length of a ROM name
#define TARGET_FILE_SIZE        32768           // Size of the combined MSX MENU ROM and the configuration file
#define FLASH_START             0x10000000      // Start of the flash memory on the Raspberry Pi Pico
#define MAX_ROM_FILES           256             // Maximum number of ROM files
#define MAX_ROM_SIZE            10*1024*1024    // Maximum size of a ROM file
#define MIN_ROM_SIZE            8192            // Minimum size of a ROM file
#define MAX_ANALYSIS_SIZE       131072          // 128KB for the mapper analysis

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
uint8_t detect_rom_type(const char *filename, uint32_t size);

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

// detect_rom_type - Detect the ROM type using a heuristic approach
// Parameters:
// filename - Name of the ROM file
// size - Size of the ROM file
// Returns:
// ROM type: 0 - Unknown, 1 - 16KB ROM, 2 - 32KB ROM, 3 - Konami SCC ROM, 4 - 48KB Linear0 ROM, 5 - ASCII8 ROM, 6 - ASCII16 ROM, 7 - Konami (without SCC) ROM
uint8_t detect_rom_type(const char *filename, uint32_t size) {
    
    // Define the NEO8 signature
    const char neo8_signature[] = "ROM_NEO8";
    const char neo16_signature[] = "ROM_NE16";

    // Initialize weighted scores for different mapper types
    int konami_score = 0;
    int konami_scc_score = 0;
    int ascii8_score = 0;
    int ascii16_score = 0;

    // Define weights for specific addresses
    const int KONAMI_WEIGHT = 2;
    const int KONAMI_SCC_WEIGHT = 2;
    const int ASCII8_WEIGHT_HIGH = 3;
    const int ASCII8_WEIGHT_LOW = 1;
    const int ASCII16_WEIGHT = 2;

    //size_t size = file_size(filename);
    if (size > MAX_ROM_SIZE || size < MIN_ROM_SIZE) {
        printf("Invalid ROM size\n");
        return 0; // unknown mapper
    }

    FILE *file = fopen(filename, "rb");
    if (!file) {
        printf("Failed to open ROM file\n");
        return 0; // unknown mapper
    }

    // Determine the size to read (max 128KB or the actual size if smaller)
    size_t read_size = (size > MAX_ANALYSIS_SIZE) ? MAX_ANALYSIS_SIZE : size;
    //size_t read_size = size; 
    // Dynamically allocate memory for the ROM
    uint8_t *rom = (uint8_t *)malloc(read_size);
    if (!rom) {
        printf("Failed to allocate memory for ROM\n");
        fclose(file);
        return 0; // unknown mapper
    }

    fread(rom, 1, read_size, file);
    fclose(file);
    
    // Check if the ROM has the signature "AB" at 0x0000 and 0x0001
    // Those are the cases for 16KB and 32KB ROMs
    if (rom[0] == 'A' && rom[1] == 'B' && size == 16384) {
        free(rom);
        return 1;     // Plain 16KB 
    }

    if (rom[0] == 'A' && rom[1] == 'B' && size <= 32768) {

        //check if it is a normal 32KB ROM or linear0 32KB ROM
        if (rom[0x4000] == 'A' && rom[0x4001] == 'B') {
            free(rom);
            return 4; // Linear0 32KB
        }
        
        free(rom);
        return 2;     // Plain 32KB 
    }

    // Check for the "AB" header at the start
    if (rom[0] == 'A' && rom[1] == 'B') {
        // Check for the NEO8 signature at offset 16
        if (memcmp(&rom[16], neo8_signature, sizeof(neo8_signature) - 1) == 0) {
            return 8; // NEO8 mapper detected
        } else if (memcmp(&rom[16], neo16_signature, sizeof(neo16_signature) - 1) == 0) {
            return 9; // NEO16 mapper detected
        }
    }

    // Check if the ROM has the signature "AB" at 0x4000 and 0x4001
    // That is the case for 48KB ROMs with Linear page 0 config
    if (rom[0x4000] == 'A' && rom[0x4001] == 'B' && size == 49152) {
        free(rom);
        return 4; // Linear0 48KB
    }

    // Heuristic analysis for larger ROMs
    if (size > 32768) {
        // Scan through the ROM data to detect patterns
        for (size_t i = 0; i < read_size - 3; i++) {
            if (rom[i] == 0x32) { // Check for 'ld (nnnn),a' instruction
                uint16_t addr = rom[i + 1] | (rom[i + 2] << 8);
                switch (addr) {
                    case 0x4000:
                    case 0x8000:
                    case 0xA000:
                        konami_score += KONAMI_WEIGHT;
                        break;
                    case 0x5000:
                    case 0x9000:
                    case 0xB000:
                        konami_scc_score += KONAMI_SCC_WEIGHT;
                        break;
                    case 0x6800:
                    case 0x7800:
                        ascii8_score += ASCII8_WEIGHT_HIGH;
                        break;
                    case 0x77FF:
                        ascii16_score += ASCII16_WEIGHT;
                        break;
                    case 0x6000:
                        konami_score += KONAMI_WEIGHT;
                        konami_scc_score += KONAMI_SCC_WEIGHT;
                        ascii8_score += ASCII8_WEIGHT_LOW;
                        ascii16_score += ASCII16_WEIGHT;
                        break;
                    case 0x7000:
                        konami_scc_score += KONAMI_SCC_WEIGHT;
                        ascii8_score += ASCII8_WEIGHT_LOW;
                        ascii16_score += ASCII16_WEIGHT;
                        break;
                    // Add more cases as needed
                }
            }
        }
         
        
        /*
        printf ("DEBUG: ascii8_score = %d\n", ascii8_score);
        printf ("DEBUG: ascii16_score = %d\n", ascii16_score);
        printf ("DEBUG: konami_score = %d\n", konami_score);
        printf ("DEBUG: konami_scc_score = %d\n\n", konami_scc_score);
        */
        
        if (ascii8_score==1) ascii8_score--;

        // Determine the ROM type based on the highest weighted score
        if (konami_scc_score > konami_score && konami_scc_score > ascii8_score && konami_scc_score > ascii16_score) {
            free(rom);
            return 3; // Konami SCC
        }
        if (konami_score > konami_scc_score && konami_score > ascii8_score && konami_score > ascii16_score) {
            free(rom);
            return 7; // Konami
        }
        if (ascii8_score > konami_score && ascii8_score > konami_scc_score && ascii8_score > ascii16_score) {
            free(rom);
            return 5; // ASCII8
        }
        if (ascii16_score > konami_score && ascii16_score > konami_scc_score && ascii16_score > ascii8_score) {
            free(rom);
            return 6; // ASCII16
        }

        if (ascii16_score == konami_scc_score)
        {
            free(rom);
            return 6; // Konami SCC
        }

        free(rom);
        return 0; // unknown mapper
    }
   
}

// create_uf2_file - Create the UF2 file
// This function will create the UF2 file with the firmware, menu and ROM files
// Parameters:
// combined_filename - Name of the combined file
// uf2_filename - Name of the UF2 file
void create_uf2_file(const char *combined_filename, const char *uf2_filename) {
   
    // Get the size of the combined file
    uint32_t sz = file_size(combined_filename);

    // Open the combined file
    FILE *combined_file = fopen(combined_filename, "rb");
    if (!combined_file) {
        printf("Failed to open the combined file");
        return;
    }

    // Create/Open for writing the UF2 file
    FILE *uf2_file = fopen(uf2_filename, "wb");
    if (!uf2_file) {
        printf("Failed to create UF2 file");
        fclose(combined_file);
        return;
    }

    UF2_Block bl; // UF2 block
    memset(&bl, 0, sizeof(bl)); // Clear the block

    bl.magicStart0 = UF2_MAGIC_START0; // 0x0A324655 "UF2\n"
    bl.magicStart1 = UF2_MAGIC_START1; // 0x9E5D5157
    bl.flags = 0x00002000; // UF2_FLAG_FAMILYID_PRESENT
    bl.magicEnd = UF2_MAGIC_END; // 0x0AB16F30
    bl.targetAddr = FLASH_START; // Start of the flash memory on the Raspberry Pi Pico
    bl.numBlocks = (sz + 255) / 256; // Number of blocks
    bl.payloadSize = 256; // Payload size
    bl.fileSize = 0xe48bff56; // Size of the file
    int numbl = 0; // Number of blocks

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
    uint32_t base_offset = TARGET_FILE_SIZE; // Base offset for the ROM files = 32KB MSX MENU
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

            // Only process the ROM file if it is a valid ROM/supported mapper
            rom_size = file_size(entry->d_name);
            uint8_t mapper_byte = detect_rom_type(entry->d_name, rom_size);
            if (mapper_byte != 0)
            {
                // Write the file name (20 bytes)
                fwrite(rom_name, 1, MAX_FILE_NAME_LENGTH, output_file);
                current_size += MAX_FILE_NAME_LENGTH;

                // Write the mapper (1 byte)
                fwrite(&mapper_byte, 1, 1, output_file);
                current_size += 1;

                // Write the file size (4 bytes)
                fwrite(&rom_size, 4, 1, output_file);
                current_size += 4;

                // Write the flash offset (4 bytes)
                fwrite(&fl_offset, 4, 1, output_file);
                current_size += 4;

                // Print file information
                printf("File %02d: Name = %-20s, Size = %07u bytes, Flash Offset = 0x%08X, Mapper = %02d\n", file_index, rom_name, rom_size, fl_offset, mapper_byte);

                // Update base offset for the next file
                base_offset += rom_size;

                // Store rom information
                strncpy(files[file_count].file_name, entry->d_name, sizeof(files[file_count].file_name));
                files[file_count].file_size = rom_size;
                file_count++;
                file_index++;
            }
        }
    }

    closedir(dir);
    fclose(output_file);

    // Create the final output file
    final_output_file = fopen(COMBINED_FILE, "wb");
    if (!final_output_file) {
        printf("Failed to create final output file");
        fclose(output_file);
        return 1;
    }

    // Open the PICO firmware binary file to copy to the final output file
    output_file = fopen(PICOFIRMWARE, "rb");
    if (!output_file) {
        printf("Failed to open PICO firmware binary file");
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
        printf("Failed to open MENU_FILE");
        return 1;
    }

#ifdef DEBUG
    // create a MSX ROM file to debug on OpenMSX
    FILE *msx_rom = fopen("multirom.rom", "wb");
    if (!msx_rom) {
        printf("Failed to create MSX ROM file");
        return 1;
    }
#endif
    // Copy only 16KB (16 * 1024 bytes) of the MENU_FILE
    size_t bytes_to_copy = 16 * 1024;
    while (bytes_to_copy > 0 && (bytes_read = fread(buffer, 1, sizeof(buffer), output_file)) > 0) {
        if (bytes_read > bytes_to_copy) {
            bytes_read = bytes_to_copy;
        }
        fwrite(buffer, 1, bytes_read, final_output_file);
#ifdef DEBUG
        fwrite(buffer, 1, bytes_read, msx_rom); //debug
#endif
        total_bytes_written += bytes_read;
        bytes_to_copy -= bytes_read;
    }
    fclose(output_file);

    // Open the CONFIG_FILE to copy to the final output file
    output_file = fopen(CONFIG_FILE, "rb");
    if (!output_file) {
        printf("Failed to open CONFIG_FILE");
        return 1;
    }

    // Copy the CONFIG_FILE to the final output file
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), output_file)) > 0) {
        fwrite(buffer, 1, bytes_read, final_output_file);
#ifdef DEBUG
       fwrite(buffer, 1, bytes_read, msx_rom); //debug
#endif
        total_bytes_written += bytes_read;
    }
    fclose(output_file);

    // Pad the remaining space to reach 32KB
    write_padding(final_output_file, total_bytes_written, TARGET_FILE_SIZE, 0xFF);
#ifdef DEBUG
    write_padding(msx_rom, total_bytes_written, TARGET_FILE_SIZE, 0xFF); //debug
    fclose(msx_rom); //debug
#endif

    // Append the content of each ROM file to the final output file in the same order
    for (int i = 0; i < file_count; i++) {
        input_file = fopen(files[i].file_name, "rb");
        //printf("Appending ROM file %s to the final output file...\n", files[i].file_name);
        if (!input_file) {
            printf("Failed to open ROM file");
            continue;
        }

        while ((bytes_read = fread(buffer, 1, sizeof(buffer), input_file)) > 0) {
            fwrite(buffer, 1, bytes_read, final_output_file);
        }

        fclose(input_file);
    }

    fclose(final_output_file);
    create_uf2_file(COMBINED_FILE, UF2FILENAME);
    return 0;
}