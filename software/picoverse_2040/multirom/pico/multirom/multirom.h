// MSX PICOVERSE PROJECT
// (c) 2024 Cristiano Goncalves
// The Retro Hacker


// Maximum ROM Size
// At this moment 128KB is the maximum size of a ROM cartridge supported by this tool.
#define MAX_ROM_SIZE (128*1024)

// -----------------------
// User-defined pin assignments
// -----------------------
// Address lines (A0-A15) as inputs from MSX
#define PIN_A0     0 
#define PIN_A1     1
#define PIN_A2     2
#define PIN_A3     3
#define PIN_A4     4
#define PIN_A5     5
#define PIN_A6     6
#define PIN_A7     7
#define PIN_A8     8
#define PIN_A9     9
#define PIN_A10    10
#define PIN_A11    11
#define PIN_A12    12
#define PIN_A13    13
#define PIN_A14    14
#define PIN_A15    15

// Data lines (D0-D7)
#define PIN_D0     16
#define PIN_D1     17
#define PIN_D2     18
#define PIN_D3     19
#define PIN_D4     20
#define PIN_D5     21
#define PIN_D6     22
#define PIN_D7     23

// Control signals
#define PIN_RD     24   // Read strobe from MSX
#define PIN_WR     25   // Write strobe from MSX
#define PIN_IORQ   26   // IO Request line from MSX
#define PIN_SLTSL  27   // Slot Select for this cartridge slot
#define PIN_WAIT    28  // WAIT line to MSX 
#define PIN_BUSSDIR 29  // Bus direction line to MSX

// This symbol marks the end of the main program in flash.
// Custom data starts right after it
extern unsigned char __flash_binary_end;
//pointer to the custom data
const uint8_t *rom = (const uint8_t *)&__flash_binary_end;

// RAM buffer for the ROM data
static uint8_t rom_sram[MAX_ROM_SIZE];
