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

static inline void setup_gpio();
unsigned long read_ulong(const unsigned char *ptr);
int isEndOfData(const unsigned char *memory);
int __no_inline_not_in_flash_func(loadrom_msx_menu)(uint32_t offset, uint32_t size);
void __no_inline_not_in_flash_func(loadrom_plain32)(uint32_t offset);
void __no_inline_not_in_flash_func(loadrom_linear48)(uint32_t offset);
void __no_inline_not_in_flash_func(loadrom_konamiscc)(uint32_t offset);
void __no_inline_not_in_flash_func(loadrom_konami)(uint32_t offset);
void __no_inline_not_in_flash_func(loadrom_ascii8)(uint32_t offset);
void __no_inline_not_in_flash_func(loadrom_ascii16)(uint32_t offset);


