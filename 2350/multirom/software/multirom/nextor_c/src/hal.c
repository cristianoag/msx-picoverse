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

uint8_t getManufacturerID() 
{
    write_command(0x03);
    //delay_ms(50);
    return read_status();
}

uint32_t getSDCapacity() 
{
    uint32_t sd_capacity;
    sd_capacity = 0;
    write_command(0x05); 
    for (uint8_t i=0;i<4;i++)
    {
       delay_ms(60);
       uint8_t byte = read_status();
       sd_capacity |= (uint32_t)byte<<(8 * i);
    }
    return sd_capacity;
}

uint32_t getSDSerial() 
{
    uint32_t sd_serial;
    sd_serial = 0;
    write_command(0x04); 
    for (uint8_t i=0;i<4;i++)
    {
        delay_ms(60);
        uint8_t byte = read_status();
        sd_serial |= (uint32_t)byte<<(8 * i);
    }
    return sd_serial;
}
