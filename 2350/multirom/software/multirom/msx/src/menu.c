// MSX PICOVERSE PROJECT
// (c) 2025 Cristiano Goncalves
// The Retro Hacker
//
// menu.c - MSX ROM with the menu program for the MSX PICOVERSE 2350 project
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

#include "menu.h"

// read_ulong - Read a 4-byte value from the memory area
// This function will read a 4-byte value from the memory area pointed by ptr and return the value as an unsigned long
// Parameters:
//   ptr - Pointer to the memory area to read the value from
// Returns:
//   The 4-byte value as an unsigned long 
unsigned long read_ulong(const unsigned char *ptr) {
    return (unsigned long)ptr[0] |
           ((unsigned long)ptr[1] << 8) |
           ((unsigned long)ptr[2] << 16) |
           ((unsigned long)ptr[3] << 24);
}

// isEndOfData - Check if the memory area is the end of the data
// This function will check if the memory area pointed by memory is the end of the data. The end of the data is defined by all bytes being 0xFF.
// Parameters:
//   memory - Pointer to the memory area to check
// Returns:
//   1 if the memory area is the end of the data, 0 otherwise
int isEndOfData(const unsigned char *memory) {
    for (int i = 0; i < ROM_RECORD_SIZE; i++) {
        if (memory[i] != 0xFF) {
            return 0;
        }
    }
    return 1;
}

// readROMData - Read the ROM records from the memory area
// This function will read the ROM records from the memory area pointed by memory and store them in the records array. The function will stop reading
// when it reaches the end of the data or the maximum number of records is reached.
// Parameters:
//   records - Pointer to the array of ROM records to store the data
//   recordCount - Pointer to the variable to store the number of records read
void readROMData(ROMRecord *records, unsigned char *recordCount, unsigned long *sizeTotal) {
    unsigned char *memory = (unsigned char *)MEMORY_START;
    unsigned char count;
    unsigned long total;

    count = 0;
    total = 0;
    while (count < MAX_ROM_RECORDS && !isEndOfData(memory)) {
        // Copy Name
        MemCopy(records[count].Name, memory, 19);
        records[count].Name[19] = '\0'; // Ensure null termination
        records[count].Mapper = memory[20]; // Read Mapper code
        records[count].Size = read_ulong(&memory[21]); // Read Size (4 bytes)
        records[count].Offset = read_ulong(&memory[25]); // Read Offset (4 bytes)
        memory += ROM_RECORD_SIZE; // Move to the next record
        total += records[count].Size;
        count++;
    }

    *recordCount = count;
    *sizeTotal = total;
}

// putchar - Print a character on the screen
// This function will override the putchar function to use the MSX BIOS routine to print characters on the screen
// This is to deal with the mess we have between the Fusion-C putchar and the SDCC Z80 library putchar
// Parameters:
//  character - The character to print
// Returns:
//  The character printed
int putchar (int character)
{
    __asm
    ld      hl, #2              ;Get the return address of the function
    add     hl, sp              ;Bypass the return address of the function 
    ld      a, (hl)              ;Get the character to print

    ld      iy,(#BIOS_EXPTBL-1)  ;BIOS slot in iyh
    push    ix                     ;save ix
    ld      ix,#BIOS_CHPUT       ;address of BIOS routine
    call    BIOS_CALSLT          ;interslot call
    pop ix                      ;restore ix
    __endasm;

    return character;
}

void execute_rst00() {
    __asm
        rst 0x00
    __endasm;
}

void clear_screen_0() {
    __asm
    ld      a, #0            ; Set SCREEN 0 mode
    call    BIOS_CHGMOD    ; Call BIOS CHGMOD to change screen mode to SCREEN 0
    call    BIOS_CLS       ; Call BIOS CLS function to clear the screen
    __endasm;
}

void clear_fkeys()
{
    __asm
    ld hl, #BIOS_FNKSTR    ; Load the starting address of function key strings into HL
    ld de, #0xF880    ; Load the next address into DE for block fill
    ld bc, #160       ; Set BC to 160, the number of bytes to clear
    ld (hl), #0       ; Initialize the first byte to 0

clear_loop:
    ldi              ; Load (HL) with (DE), increment HL and DE, decrement BC
    dec hl           ; Adjust HL back to the correct position
    ld (hl), #0      ; Set the current byte to 0
    inc hl           ; Move to the next byte
    dec bc           ; Decrement the byte counter
    ld a, b          ; Check if BC has reached zero
    or c
    jp nz, clear_loop ; Repeat until all bytes are cleared
    __endasm;
}

// invert_chars - Invert the characters in the character table
// This function will invert the characters from startChar to endChar in the character table. We use it to copy and invert the characters from the
// normal character table area to the inverted character table area. This is to display the game names in the inverted character table.
// Parameters:
//   startChar - The first character to invert
//   endChar - The last character to invert
void invert_chars(unsigned char startChar, unsigned char endChar)
{
    unsigned int srcAddress, dstAddress;
    unsigned char patternByte;
    unsigned char i, c;

    for (c = startChar; c <= endChar; c++)
    {
        // Each character has 8 bytes in the pattern table.
        srcAddress  = 0x0800 + ((unsigned int)c * 8);
        // Calculate destination address (shift by +95 bytes)
        dstAddress = srcAddress + (96*8);

        // Flip all 8 bytes that define this character.
        for (i = 0; i < 8; i++)
        {
            patternByte = Vpeek(srcAddress + i);
            patternByte = ~patternByte;           // CPL (bitwise NOT)
            Vpoke(dstAddress  + i, patternByte);
        }
    }
}

// print_str_normal - Print a string using the normal character table
// This function will print a string using the normal character table. It will apply an offset to the characters to display them correctly.
// Used to display the game names in the normal characters.
// Parameters:
//   str - The string to print
void print_str_normal(const char *str) 
{
    while (*str) { // Loop through each character in the string
        char modifiedChar = *str - 96; // Apply the offset to the character
        PrintChar(modifiedChar); // Print the modified character
        str++; // Move to the next character in the string
    }
}

// print_str_inverted - Print a string using the inverted character table
// This function will print a string using the inverted character table. It will apply an offset to the characters to display them correctly.
// Used to display the game names in the inverted characters.
// Parameters:
//   str - The string to print
void print_str_inverted(const char *str) 
{
    while (*str) {// Loop through each character in the string
        int modifiedChar = *str + 96; // Apply the offset to the character
        PrintChar(modifiedChar); // Print the modified character
        str++; // Move to the next character in the string
    }
}

char* mapper_description(int number) {
    // Array of strings for the descriptions
    const char *descriptions[] = {"PL-16", "PL-32", "KonSCC", "Linear", "ASC-08", "ASC-16", "Konami","NEO-8","NEO-16"};	
    return descriptions[number - 1];
}

// displayMenu - Display the menu on the screen
// This function will display the menu on the screen. It will print the header, the files on the current page and the footer with the page number and options.
void displayMenu() {
    //Cls(); // for some reason is not working here
    //clear_screen_0(); // works but reset the char table
    Screen(0);
    invert_chars(32, 126); // Invert the characters from 32 to 126
    //FunctionKeys(1); // Disable the function keys


    Locate(0, 0);
    printf("MSX PICOVERSE 2350    [MultiROM v1.0]");
    Locate(0, 1);
    printf("-------------------------------------");
    unsigned char xi = currentIndex%(FILES_PER_PAGE); // Calculate the index of the file to start displaying
    for (int i = 0; (i < FILES_PER_PAGE) && (xi<totalFiles-1) && (i<totalFiles); i++)  // Loop through the files
    {   
        Locate(0, 2 + i); // Position on the screen, starting at line 2
        xi = i+((currentPage-1)*FILES_PER_PAGE); // Calculate the index of the file to display
        printf(" %-24s %04lu %-7s",records[xi].Name, records[xi].Size/1024, mapper_description(records[xi].Mapper));  // Print each file name, size and mapper
    }
    // footer
    Locate(0, 21);
    printf("-------------------------------------");
    Locate(0, 22);
    printf("Page: %02d/%02d   [H - Help] [C - Config]",currentPage, totalPages); // Print the page number and the help and config options
    Locate(0, (currentIndex%FILES_PER_PAGE) + 2); // Position the cursor on the selected file
    printf(">"); // Print the cursor
    print_str_inverted(records[currentIndex%FILES_PER_PAGE].Name); // Print the selected file name inverted
}

// configMenu - Display the configuration menu on the screen
// This function will display the configuration menu on the screen. 
void configMenu()
{
    Cls(); // Clear the screen
    Locate(0,0);
    printf("MSX PICOVERSE 2350    [MultiROM v1.0]");
    Locate(0, 1);
    printf("-------------------------------------");
    Locate(0, 2);

    

    Locate(0, 21);
    printf("-------------------------------------");
    Locate(0, 22);
    printf("Press any key to return to the menu!");
    InputChar();
    displayMenu();
    navigateMenu();
}

// helpMenu - Display the help menu on the screen
// This function will display the help menu on the screen. It will print the help information and the keys to navigate the menu.
void helpMenu()
{
    
    Cls(); // Clear the screen
    Locate(0,0);
    printf("MSX PICOVERSE 2350    [MultiROM v1.0]");
    Locate(0, 1);
    printf("-------------------------------------");
    Locate(0, 2);
    printf("Use [UP]  [DOWN] to navigate the menu.");
    Locate(0, 3);
    printf("Use [LEFT] [RIGHT] to navigate  pages.");
    Locate(0, 4);
    printf("Press [ENTER] or [SPACE] to load the ");
    Locate(0, 5);
    printf("  selected game.");
    Locate(0, 6);
    printf("Press [H] to display the help screen.");
    Locate(0, 7);
    printf("Press [C] to display the config page.");
    Locate(0, 21);
    printf("-------------------------------------");
    Locate(0, 22);
    printf("Press any key to return to the menu!");
    InputChar();
    displayMenu();
    navigateMenu();
}

// loadGame - Load the game from the flash memory
// This function will load the game from the flash memory based on the index. 
void loadGame(int index) 
{
    if (records[index].Mapper != 0)
    {
        Poke(0x9D01, index); // Set the game index
        execute_rst00();
        execute_rst00();
    }
}

// navigateMenu - Navigate the menu
// This function will navigate the menu. It will wait for the user to press a key and then act based on the key pressed. The user can navigate the menu using the arrow keys
// to move up and down the files, left and right to move between pages, enter to load the game, H to display the help screen and C to display the config screen.
// The function will update the current page and current index based on the key pressed and display the menu again.
void navigateMenu() 
{
    char key;

    while (1) 
    {
        //debug
        Locate(0, 23);
        //printf("Key: %3d", key);
        printf("Size: %05lu/15872", totalSize/1024);
        //debug
        //Locate(20, 23);
        //printf("Memory Mapper: Off");
        //printf("CPage: %2d Index: %2d", currentPage, currentIndex);
        Locate(0, (currentIndex%FILES_PER_PAGE) + 2);
        key = WaitKey();
        //key = KeyboardRead();
        //key = InputChar();
        char fkey = Fkeys();

        Locate(0, (currentIndex%FILES_PER_PAGE) + 2); // Position the cursor on the selected file
        printf(" "); // Clear the cursor
        printf(records[currentIndex].Name); // Print the file name
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
            case 99: // C - Config (uppercase C)
            case 67: // c - Config (lowercase c)
                // Config
                configMenu(); // Display the config menu
                break;
            case 13: // Enter
            case 32: // Space
                // Load the game
                loadGame(currentIndex); // Load the selected game
                break;
        }
        Locate(0, (currentIndex%FILES_PER_PAGE) + 2); // Position the cursor on the selected file
        printf(">"); // Print the cursor
        print_str_inverted(records[currentIndex].Name); // Print the selected file name
        Locate(0, (currentIndex%FILES_PER_PAGE) + 2); // Position the cursor on the selected file
    }
}

void main() {
    // Initialize the variables
    currentPage = 1; // Start on page 1
    currentIndex = 0; // Start at the first file - index 0
    
    readROMData(records, &totalFiles, &totalSize);
    totalPages = (int)((totalFiles/FILES_PER_PAGE)+1); // Calculate the total pages based on the total files and files per page

    //Screen(0); // Set the screen mode
    //invert_chars(32, 126); // Invert the characters from 32 to 126
    clear_fkeys(); // Clear the function keys
    //KillKeyBuffer(); // Clear the key buffer

    // Display the menu
    displayMenu();
    // Activate navigation
    navigateMenu();
}