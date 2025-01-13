#include <msx_fusion.h>
#include <stdio.h>
#include "bios.h"

// Define maximum files per page and screen properties
#define FILES_PER_PAGE 19   // Maximum files per page on the menu
#define ROM_RECORD_SIZE 29  // Size of the ROM record in the configuration area in bytes
#define MAX_ROM_RECORDS 256 // Maximum ROM files supported 2^8=256
#define MEMORY_START 0x8000 // Start of the memory area to read the ROM records
#define ROM_NAME_MAX 20     // Maximum size of the ROM name

// Structure to represent a ROM record
// The ROM record will contain the name of the ROM, the mapper code, the size of the ROM and the offset in the flash memory
// Name: ROM_NAME_MAX bytes
// Mapper: 1 byte
// Size: 4 bytes
// Offset: 4 bytes
typedef struct {
    char Name[ROM_NAME_MAX];
    unsigned char Mapper;
    unsigned long Size;
    unsigned long Offset;
} ROMRecord;

const char *rom_types[] = {
    "Unknown",          // Default for invalid indices
    "Plain16",          // Index 1
    "Plain32",          // Index 2
    "KonamiS",          // Index 3
    "Linear0",          // Index 4
    "ASCII8",           // Index 5
    "ASCII16",          // Index 6
    "Konami"            // Index 7
};

// Control Variables
int currentPage;    // Current page
int totalPages;     // Total pages
int currentIndex;   // Current file index
unsigned char totalFiles;     // Total files
unsigned long totalSize;
ROMRecord records[MAX_ROM_RECORDS]; // Array to store the ROM records

// Declare the functions
unsigned long read_ulong(const unsigned char *ptr);
int isEndOfData(const unsigned char *memory);
void readROMData(ROMRecord *records, unsigned char *recordCount, unsigned long *sizeTotal);
int putchar (int character);
void invert_chars(unsigned char startChar, unsigned char endChar);
void print_str_normal(const char *str);
void print_str_inverted(const char *str);
char* mapper_description(int number);
void charMap(); //debug
void displayMenu();
void navigateMenu();
void configMenu();
void helpMenu();
void loadGame(int index);
void main();



