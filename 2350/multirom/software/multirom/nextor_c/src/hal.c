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

void  read_data_multiple (uint8_t* buffer,uint8_t len)
{
    __asm
    ld iy, #2
    add iy,sp
    ld b,+2(iy)
    ld h,+1(iy)
    ld l,+0(iy)
    ld c, #DATA_PORT
    .db 0xED,0xB2 ;inir 
    __endasm;
}
void    write_data_multiple (uint8_t* buffer,uint8_t len)
{
    __asm
    ld iy, #2
    add iy,sp
    ld b,+2(iy)
    ld h,+1(iy)
    ld l,+0(iy)
    ld c, #DATA_PORT
    .db 0xED,0xB3 ;otir 
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

bool read_write_disk_sectors (bool writing,uint8_t nr_sectors,uint32_t* sector,uint8_t* sector_buffer)
{
    if (!writing)
    {
        if (!sd_disk_read (nr_sectors,(uint8_t*)sector,sector_buffer))
            return false;
    }
    else
    {
        if (!sd_disk_write (nr_sectors,(uint8_t*)sector,sector_buffer))
            return false;
    }
    
    return true;
}


bool sd_disk_read (uint8_t nr_sectors,uint8_t* lba,uint8_t* sector_buffer)
{
    //printf("Reading %d sectors\r\n", nr_sectors);

    uint8_t nr = nr_sectors;
    uint16_t offset = 0;

    uint8_t x = 1;

    //printf("LBA: %02X %02X %02X %02X\r\n", lba[0], lba[1], lba[2], lba[3]);
    //delay_ms(50);
    write_command(0x06);
    //delay_ms(20);
    write_command(lba[3]);
    write_command(lba[2]);
    write_command(lba[1]);
    write_command(lba[0]);
    //delay_ms(20);
    write_command(0x06);
    delay_ms(50); // read from sd is expensive
    for (uint16_t i = 0; i < 512; i++) {
        sector_buffer[offset + i] = read_data();
    }
    offset += 512;
    //printf("Read sector %d\r\n", x++);

    while (nr > 1) {
        write_command(0x07);
        delay_ms(50); // read from sd is expensive
        for (uint16_t i = 0; i < 512; i++) {
            sector_buffer[offset + i] = read_data();
        }
        offset += 512;
        //printf("Read sector %d\r\n", x++);
        nr--;
    }

    return true;
}

bool sd_disk_write (uint8_t nr_sectors,uint8_t* lba,uint8_t* sector_buffer)
{
    printf("Writing %d sectors\r\n",nr_sectors);
    //printf("LBA: %02X %02X %02X %02X\r\n",lba[0],lba[1],lba[2],lba[3]);

    //delay_ms(50);
    write_command(0x08);
    //delay_ms(80);
    write_command (lba[3]);
    write_command (lba[2]);
    write_command (lba[1]);
    write_command (lba[0]);
    //delay_ms(80);
    write_command(0x08);
    delay_ms(50);
    for (uint16_t i = 0; i < 512; i++) {
        write_data(sector_buffer[i]);
    }

    return true;
}
