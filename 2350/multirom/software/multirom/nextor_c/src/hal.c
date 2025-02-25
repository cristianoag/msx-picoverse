#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "hal.h"
#include "bios.h"

__at (BIOS_HWVER) uint8_t msx_version;
__at (BIOS_LINL40) uint8_t text_columns;

bool supports_80_column_mode()
{
    return msx_version>=1;
}

void hal_init ()
{
    text_columns = 40;
    // check MSX version
    if (supports_80_column_mode())
        text_columns = 80;

    __asm
    ld     iy,(#BIOS_EXPTBL-1)       ;BIOS slot in iyh
    push ix
    ld     ix,#BIOS_INITXT           ;address of BIOS routine
    call   BIOS_CALSLT               ;interslot call
    pop ix
    __endasm;    
}

void hal_deinit ()
{
    // nothing to do
}

void write_command (uint8_t command)  __z88dk_fastcall __naked
{
    __asm
        ld a,l
        out (#CMD_PORT),a
        ret
    __endasm;
}

void write_data (uint8_t data)  __z88dk_fastcall __naked
{
    __asm
        ld a,l
        out (#DATA_PORT),a 
        ret
    __endasm;
}

uint8_t read_data ()  __z88dk_fastcall __naked
{
    __asm
        in a,(#DATA_PORT) 
        ld l,a
        ret
    __endasm;
}

uint8_t read_status ()  __z88dk_fastcall __naked
{
    __asm
        in a,(#CMD_PORT) 
        ld l,a
        ret
    __endasm;
}

#pragma disable_warning 85	// because the var msg is not used in C context
void msx_wait (uint16_t times_jiffy)  __z88dk_fastcall __naked
{
    __asm
    ei
    ; Wait a determined number of interrupts
    ; Input: BC = number of 1/framerate interrupts to wait
    ; Output: (none)
    WAIT:
        halt        ; waits 1/50th or 1/60th of a second till next interrupt
        dec hl
        ld a,h
        or l
        jr nz, WAIT
        ret

    __endasm; 
}

void delay_ms (uint16_t milliseconds)
{
    msx_wait (milliseconds/20);
}