#include <msx_fusion.h>
#include <stdio.h>
#include "bios.h"

// Define maximum files per page and screen properties
#define FILES_PER_PAGE 19
#define SCREEN_COLUMNS 40
#define MAX_FILES 20

char *game[MAX_FILES];
char *mapp[MAX_FILES];

// Current page and file index
int currentPage;
int currentIndex;

// Total file count
int totalFiles; // Update this dynamically if needed

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

void displayMenu() {
    // Clear the screen
    Screen(0);
    Cls();

    int startFile = currentPage * FILES_PER_PAGE;
    int endFile = startFile + FILES_PER_PAGE;

    // header
    printf("MSX PICOVERSE 2040     [MultiROM v1.0]");
    Locate(0, 1);
    printf("--------------------------------------");
    Locate(0, currentIndex + 2);
    printf(">");
    int xi = 0;
    for (int i = 0; (i < FILES_PER_PAGE) && (xi<totalFiles) ; i++) 
    {
        Locate(1, 2 + i); // Position on the screen, starting at line 2
        xi = i+startFile;
        printf("%-30s %-8s",game[xi], mapp[xi]);  // Print each file name
    }
    // footer
    Locate(0, 21);
    printf("--------------------------------------");
    Locate(0, 22);
    printf("Page %02d/02       [F1 Help] [F2 Config]",currentPage + 1);
    Locate(0, currentIndex + 2);
}

void navigateMenu() 
{
    char key;

    while (1) 
    {
        key = WaitKey();
        Locate(0, 23);
        printf("Key: %3d", key);
        Locate(0, currentIndex + 2);
        printf(" ");
        switch (key) 
        {
            case 30: // Up arrow
                if (currentIndex > 0) currentIndex--;
                if (currentIndex < currentPage * FILES_PER_PAGE) {
                    currentPage--;
                }
                Locate(28, 23);
                printf("Index: %2d", currentIndex);
                break;
            case 31: // Down arrow
                if (currentIndex < FILES_PER_PAGE - 1) currentIndex++;
                if (currentIndex >= (currentPage + 1) * FILES_PER_PAGE) {
                    currentPage++;
                }
                Locate(28, 23);
                printf("Index: %2d", currentIndex);
                break;
            case 0x4D: // Right arrow
                if (currentPage < (totalFiles - 1) / FILES_PER_PAGE) {
                    currentPage++;
                    currentIndex = currentPage * FILES_PER_PAGE;
                }
                break;
            case 0x4B: // Left arrow
                if (currentPage > 0) {
                    currentPage--;
                    currentIndex = currentPage * FILES_PER_PAGE;
                }
                break;
        }
        Locate(0, currentIndex + 2);
        printf(">");
        Locate(0, currentIndex + 2);
    }
}




void main() {
    currentPage = 0;
    currentIndex = 0;
    totalFiles = 20; // Total of files stored on the flash
    
    game[0] =  "Metal Gear          ";    mapp[0] = "Konami";
    game[1] =  "Nemesis             ";    mapp[1] = "Konami";
    game[2] =  "Contra              ";    mapp[2] = "Konami";
    game[3] =  "Castlevania         ";    mapp[3] = "Konami";
    game[4] =  "Kings Valley II     ";    mapp[4] = "Konami";
    game[5] =  "Vampire Killer      ";    mapp[5] = "Konami";
    game[6] =  "Snatcher            ";    mapp[6] = "Konami";
    game[7] =  "Galaga              ";    mapp[7] = "32KB";
    game[8] =  "Zaxxon              ";    mapp[8] = "32KB";
    game[9] =  "Salamander          ";    mapp[9] = "Konami";
    game[10] = "Parodius            ";    mapp[10] = "Konami";
    game[11] = "Knightmare          ";    mapp[11] = "32KB";
    game[12] = "Pippols             ";    mapp[12] = "16KB";
    game[13] = "The Maze of Galious ";    mapp[13] = "Konami";
    game[14] = "Penguin Adventure   ";    mapp[14] = "32KB";
    game[15] = "Space Manbow        ";    mapp[15] = "Konami";
    game[16] = "Gradius 2           ";    mapp[16] = "Konami";
    game[17] = "TwinBee             ";    mapp[17] = "16KB";
    game[18] = "Zanac               ";    mapp[18] = "32KB";
    game[19] = "H.E.R.O.            ";    mapp[19] = "16KB";

    //set_mode(0);
    displayMenu();
    navigateMenu();
}