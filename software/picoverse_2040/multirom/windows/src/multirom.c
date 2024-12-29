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

#define CONFIG_FILE     "config.bin"
#define COMBINED_FILE   "romcmb.bin"
#define MAX_FILE_NAME_LENGTH 20
#define TARGET_FILE_SIZE 7424

#define FLASH_START             0x10000000 // Start of the flash memory
#define FIRMWARE_BINARY_SIZE    0x10005ec0 // Size of the firmware binary file

// Structure to store file information
typedef struct {
    char file_name[256];
    uint32_t file_size;
} FileInfo;

// Function to write padding to the file
void write_padding(FILE *file, size_t current_size, size_t target_size) {
    size_t padding_size = target_size - current_size;
    uint8_t padding_byte = 0x00;

    for (size_t i = 0; i < padding_size; i++) {
        fwrite(&padding_byte, 1, 1, file);
    }
}

int main()
{
    printf("MSX PICOVERSE 2040 MultiROM UF2 Creator v1.0\n");
    printf("(c) 2024 The Retro Hacker\n\n");

    DIR *dir;
    struct dirent *entry;
    FILE *output_file, *final_output_file, *input_file;
    size_t current_size = 0;
    int file_index = 1;
    uint32_t base_offset = FLASH_START + FIRMWARE_BINARY_SIZE + TARGET_FILE_SIZE;
    FileInfo files[256];
    int file_count = 0;

    output_file = fopen(CONFIG_FILE, "wb");
    if (!output_file) {
        perror("Failed to create output file");
        return 1;
    }

    dir = opendir(".");
    if (!dir) {
        perror("Failed to open directory");
        fclose(output_file);
        return 1;
    }

    while ((entry = readdir(dir)) != NULL) 
    {
        if (strstr(entry->d_name, ".ROM") != NULL) // Check for .ROM files
        { 
            char file_name[MAX_FILE_NAME_LENGTH] = {0};
            uint32_t file_size = 0;
            uint32_t fl_offset = base_offset;

            // Extract the first part of the file name (up to the first '.')
            char *dot_position = strchr(entry->d_name, '.');
            if (dot_position != NULL) {
                size_t name_length = dot_position - entry->d_name;
                if (name_length > MAX_FILE_NAME_LENGTH) {
                    name_length = MAX_FILE_NAME_LENGTH;
                }
                strncpy(file_name, entry->d_name, name_length);
            } else {
                strncpy(file_name, entry->d_name, MAX_FILE_NAME_LENGTH);
            }

            // Write the file name (20 bytes)
            fwrite(file_name, 1, MAX_FILE_NAME_LENGTH, output_file);
            current_size += MAX_FILE_NAME_LENGTH;

             // Write 1 byte with the mapper
             // at this moment lets write 1, then we need to create a function to discover the mapper or use something on the filename to identify it
            uint8_t one_byte = 1;
            fwrite(&one_byte, 1, 1, output_file);
            current_size += 1;

            // Open the .ROM file to determine its size
            input_file = fopen(entry->d_name, "rb");
            if (input_file) {
                fseek(input_file, 0, SEEK_END);
                file_size = ftell(input_file);
                fclose(input_file);
            }

            // Write the file size (4 bytes)
            fwrite(&file_size, 4, 1, output_file);
            current_size += 4;

            // Write the flash offset (4 bytes)
            fwrite(&fl_offset, 4, 1, output_file);
            current_size += 4;

            // Print file information
            printf("File %00d: Name = %-20s, Size = %u bytes, Flash Offset = 0x%08X, Mapper = %d\n", file_index, file_name, file_size, fl_offset, one_byte);

            // Update base offset for the next file
            base_offset += file_size;

            // Store file information
            strncpy(files[file_count].file_name, entry->d_name, sizeof(files[file_count].file_name));
            files[file_count].file_size = file_size;
            file_count++;

            file_index++;
        }
    }

    closedir(dir);

    // Write padding if the file size is less than 7424 bytes
    if (current_size < TARGET_FILE_SIZE) {
        write_padding(output_file, current_size, TARGET_FILE_SIZE);
    }

    fclose(output_file);

    // Create the final output file
    output_file = fopen(CONFIG_FILE, "rb");
    if (!output_file) {
        perror("Failed to open intermediate output file");
        return 1;
    }

    final_output_file = fopen(COMBINED_FILE, "wb");
    if (!final_output_file) {
        perror("Failed to create final output file");
        fclose(output_file);
        return 1;
    }

    // Copy the intermediate output file to the final output file
    uint8_t buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), output_file)) > 0) {
        fwrite(buffer, 1, bytes_read, final_output_file);
    }
    fclose(output_file);

    // Append the content of each ROM file to the final output file in the same order
    for (int i = 0; i < file_count; i++) {
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

    printf("\nFinal binary file '%s' created successfully.\n", COMBINED_FILE);

    return 0;
}