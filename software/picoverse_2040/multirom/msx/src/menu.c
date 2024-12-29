// MSX PICOVERSE PROJECT
// (c) 2024 Cristiano Goncalves
// The Retro Hacker
//
// menu.c - MSX ROM with the menu program for the MSX PICOVERSE 2040 project
//
// This program will display a menu with the games stored on the flash memory. The user can navigate the menu using the arrow keys and select a game to load. 
// The program will display the game name, size and mapper type. The user can also display a help screen with the available keys and a configuration screen 
// to change the settings of the program. The program will read the flash memory configuration area to populate the game list. The configuration area will 
// contain the game name, size, mapper type and offset in the flash memory.
// 
// The program needs to be compiled using the Fusion-C library and the MSX BIOS routines. 
// 
// This work is licensed  under a "Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International
// License". https://creativecommons.org/licenses/by-nc-sa/4.0/

#include <msx_fusion.h>
#include <stdio.h>
#include "bios.h"

// Define maximum files per page and screen properties
#define FILES_PER_PAGE 19 // Maximum files per page
#define MAX_FILES 256 // Maximum files supported

// Define the variables for the files
// game - Game name - 20 bytes max
// mapp - Mapper code - 1 byte max
// size - Size of the game in bits - 4 bytes max
// offset - Offset of the game in the flash - 4 bytes max
// These variables will be populated dynamically from the flash configuration area
char *game[MAX_FILES];
int mapp[MAX_FILES];
unsigned long offset[MAX_FILES];
unsigned long size[MAX_FILES];

// Function prototypes
int putchar (int character);
char* mapper_description(int number);
void navigateMenu();
void displayMenu();
void helpMenu();

// Current page and file index
int currentPage; // Current page
int totalPages; // Total pages
int currentIndex; // Current file index
int totalFiles; // Total files

// Function to override the putchar function
int putchar (int character)
{
    __asm
    ld      hl, #2 
    add     hl, sp   ;Bypass the return address of the function 
    ld     a, (hl)

    ld     iy,(#BIOS_EXPTBL-1)       ;BIOS slot in iyh
    push ix
    ld     ix,#BIOS_CHPUT       ;address of BIOS routine
    call   BIOS_CALSLT          ;interslot call
    pop ix
    __endasm;

    return character;
}

// Function to return the description of the mapper
// 1: 16KB, 2: 32KB, 3: Konami, 4: Linear0
char* mapper_description(int number) {
    // Array of strings for the descriptions
    const char *descriptions[] = {"16KB", "32KB", "KONAMI", "LINEAR0"};
    return descriptions[number - 1];
}

// Function to display the menu
void displayMenu() {
    
    Screen(0); // Set the screen mode
    Cls(); // Clear the screen

    // header
    printf("MSX PICOVERSE 2040     [MultiROM v1.0]");
    Locate(0, 1);
    printf("--------------------------------------");
    int xi = currentIndex%(FILES_PER_PAGE-1); // Calculate the index of the file to start displaying
    for (int i = 0; (i < FILES_PER_PAGE) && (xi<totalFiles-1) ; i++)  // Loop through the files
    {
        Locate(1, 2 + i); // Position on the screen, starting at line 2
        xi = i+((currentPage-1)*FILES_PER_PAGE); // Calculate the index of the file to display
        printf("%-22s %4luKB %-8s",game[xi], size[xi]/1024, mapper_description(mapp[xi]));  // Print each file name, size and mapper
    }
    // footer
    Locate(0, 21);
    printf("--------------------------------------");
    Locate(0, 22);
    printf("Page %02d/%02d     [H - Help] [C - Config]",currentPage, totalPages); // Print the page number and the help and config options
    Locate(0, (currentIndex%FILES_PER_PAGE) + 2); // Position the cursor on the selected file
    printf(">"); // Print the cursor
}

void helpMenu()
{
    
    Cls(); // Clear the screen
    printf("MSX PICOVERSE 2040     [MultiROM v1.0]");
    Locate(0, 1);
    printf("---------------------------------------");
    Locate(0, 2);
    printf("Use [UP]  [DOWN] to navigate the menu.");
    Locate(0, 3);
    printf("Use [LEFT] [RIGHT] to navigate  pages.");
    Locate(0, 4);
    printf("Press [H] to display the help screen.");
    Locate(0, 5);
    printf("Press [C] to display the config page.");
    Locate(0, 7);

    // Debug - Display the MSX character table
    // Variables for looping and positioning
    int row;
    int col;
    row = 8;
    col = 0;
    // Loop through all 256 characters in the MSX character table
    for (int i = 33; i < 256; i++) 
    {
        // Set the cursor position
        Locate(col, row);
        // Print the character
        PrintChar(i);
        // Move to the next column
        col++;
        // If we reach the end of the line, go to the next row
        if (col >= 40) {
            col = 0;
            row++;
        }
    }

    Locate(0, 21);
    printf("--------------------------------------");
    Locate(0, 22);
    printf("Press any key to return to the menu...");
    WaitKey();
    displayMenu();
    navigateMenu();
}

void navigateMenu() 
{
    char key;

    while (1) 
    {
#ifdef DEBUG
        Locate(18, 23);
        printf("CPage: %2d Index: %2d", currentPage, currentIndex);
#endif
        Locate(0, (currentIndex%FILES_PER_PAGE) + 2);
        key = WaitKey();
        //debug
#ifdef DEBUG
        Locate(0, 23);
        printf("Key: %3d", key);
#endif
        Locate(0, (currentIndex%FILES_PER_PAGE) + 2); // Position the cursor on the selected file
        printf(" "); // Clear the cursor
        switch (key) 
        {
            case 30: // Up arrow
                if (currentIndex > 0) // Check if we are not at the first file
                {
                    if (currentIndex%FILES_PER_PAGE >= 0) currentIndex--; // Move to the previous file
                    if (currentIndex < ((currentPage-1) * FILES_PER_PAGE))  // Check if we need to move to the previous page
                    {
                        currentPage--; // Move to the previous page
                        displayMenu(); // Display the menu
                    }
                }
                break;
            case 31: // Down arrow
                if ((currentIndex%FILES_PER_PAGE < FILES_PER_PAGE) && currentIndex < totalFiles-1) currentIndex++; // Move to the next file
                if (currentIndex >= (currentPage * FILES_PER_PAGE)) // Check if we need to move to the next page
                {
                    currentPage++; // Move to the next page
                    displayMenu(); // Display the menu
                }
                break;
            case 28: // Right arrow
                if (currentPage < totalPages) // Check if we are not on the last page
                {
                    currentPage++; // Move to the next page
                    currentIndex = (currentPage-1) * FILES_PER_PAGE; // Move to the first file of the page
                    displayMenu(); // Display the menu
                }
                break;
            case 29: // Left arrow
                if (currentPage > 1) // Check if we are not on the first page
                {
                    currentPage--; // Move to the previous page
                    currentIndex = (currentPage-1) * FILES_PER_PAGE; // Move to the first file of the page
                    displayMenu(); // Display the menu
                }
                break;
            case 72: // H - Help (uppercase H)
            case 104: // h - Help (lowercase h)
                // Help
                helpMenu(); // Display the help menu
                break;
        }
        Locate(0, (currentIndex%FILES_PER_PAGE) + 2); // Position the cursor on the selected file
        printf(">"); // Print the cursor
        Locate(0, (currentIndex%FILES_PER_PAGE) + 2); // Position the cursor on the selected file
    }
}



void main() {
    // Initialize the variables
    currentPage = 1; // Start on page 1
    currentIndex = 0; // Start at the first file - index 0

    totalFiles = 22; // Total of files stored on the flash
    totalPages = (int)((totalFiles/FILES_PER_PAGE)+1); // Calculate the total pages based on the total files and files per page
    
    // sample games - need to loop through the flash configuration area and populate the arrays
    // game - Game name - 20 bytes max
    // mapp - Mapper code - 1 byte max
    // size - Size of the game in bits - 4 bytes max
    // offset - Offset of the game in the flash - 4 bytes max

    // mappers - 1: 16KB, 2: 32KB, 3: Konami, 4: Linear0

    game[0] =  "Metal Gear          ";    mapp[0] = 3; size[0] = 131072;
    game[1] =  "Nemesis             ";    mapp[1] = 3; size[1] = 131072;
    game[2] =  "Contra              ";    mapp[2] = 3; size[2] = 131072;
    game[3] =  "Castlevania         ";    mapp[3] = 3; size[3] = 131072;
    game[4] =  "Kings Valley II     ";    mapp[4] = 3; size[4] = 131072;
    game[5] =  "Vampire Killer      ";    mapp[5] = 3; size[5] = 131072;
    game[6] =  "Snatcher            ";    mapp[6] = 3; size[6] = 131072;
    game[7] =  "Galaga              ";    mapp[7] = 2; size[7] = 32768;
    game[8] =  "Zaxxon              ";    mapp[8] = 2; size[8] = 32768;
    game[9] =  "Salamander          ";    mapp[9] = 3; size[9] = 131072;
    game[10] = "Parodius            ";    mapp[10] = 3; size[10] = 131072;
    game[11] = "Knightmare          ";    mapp[11] = 2; size[11] = 32768;
    game[12] = "Pippols             ";    mapp[12] = 1; size[12] = 16384;
    game[13] = "The Maze of Galious ";    mapp[13] = 3; size[13] = 131072;
    game[14] = "Penguin Adventure   ";    mapp[14] = 2; size[14] = 32768;
    game[15] = "Space Manbow        ";    mapp[15] = 3; size[15] = 131072;
    game[16] = "Gradius 2           ";    mapp[16] = 3; size[16] = 131072;
    game[17] = "TwinBee             ";    mapp[17] = 1; size[17] = 16384;
    game[18] = "Zanac               ";    mapp[18] = 2; size[18] = 32768;
    game[19] = "H.E.R.O.            ";    mapp[19] = 1; size[19] = 16384;
    game[20] = "Yie Ar Kung-Fu      ";    mapp[20] = 2; size[20] = 32768;
    game[21] = "XRacing             ";    mapp[21] = 4; size[21] = 49152;

    // Display the menu
    displayMenu();
    // Activate navigation
    navigateMenu();
}