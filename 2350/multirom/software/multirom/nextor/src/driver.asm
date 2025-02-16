; MSX PICOVERSE PROJECT
; (c) 2025 Cristiano Goncalves
; The Retro Hacker
; 
; Nextor 2.1 Device-Based Driver for PicoVerse
;

org 4100h

; I/O ports used:
;   21  = Command port
;   20  = Data port
PORT_CMD  EQU 21
PORT_DATA EQU 20

; Nextor Command codes:
CMD_NEXTOR_READ_SECTOR  EQU 01     	; Command to read one sector
CMD_NEXTOR_WRITE_SECTOR EQU 02   	; Command to write one sector

; Bios Command codes:
CHPUT 			EQU 00A2h
CHGET 			EQU 009Fh
BIOS_EXPTBL 	EQU 0xfcc1
BIOS_INITXT  	EQU 0x006c
BIOS_CALSLT   	EQU 0x001c

ds 100h ;256 dummy bytes

DRV_START:

;-----------------------------------------------------------------------------
;
; Miscellaneous constants
;

;This is a 2 byte buffer to store the address of code to be executed.
;It is used by some of the kernel page 0 routines.

CODE_ADD:	equ	0F1D0h

;-----------------------------------------------------------------------------
;
; Driver configuration constants
;

;Driver type:
;   0 for drive-based
;   1 for device-based

DRV_TYPE	equ	1

;Driver version

VER_MAIN	equ	1
VER_SEC		equ	0
VER_REV		equ	0

;-----------------------------------------------------------------------------
;
; Error codes for DEV_RW
;

if DRV_TYPE eq 1

.NCOMP	equ	0FFh
.WRERR	equ	0FEh
.DISK	equ	0FDh
.NRDY	equ	0FCh
.DATA	equ	0FAh
.RNF	equ	0F9h
.WPROT	equ	0F8h
.UFORM	equ	0F7h
.SEEK	equ	0F3h
.IFORM	equ	0F0h
.IDEVL	equ	0B5h
.IPARM	equ	08Bh

endif


;-----------------------------------------------------------------------------
;
; Routines and information available on kernel page 0
;

;* Get in A the current slot for page 1. Corrupts F.
;  Must be called by using CALBNK to bank 0:
;    xor a
;    ld ix,GSLOT1
;    call CALBNK

GSLOT1	equ	402Dh


;* This routine reads a byte from another bank.
;  Must be called by using CALBNK to the desired bank,
;  passing the address to be read in HL:
;    ld a,<bank number>
;    ld hl,<byte address>
;    ld ix,RDBANK
;    call CALBNK

RDBANK	equ	403Ch


;* This routine temporarily switches kernel main bank
;  (usually bank 0, but will be 3 when running in MSX-DOS 1 mode),
;  then invokes the routine whose address is at (CODE_ADD).
;  It is necessary to use this routine to invoke CALBAS
;  (so that kernel bank is correct in case of BASIC error)
;  and to invoke DOS functions via F37Dh hook.
;
;  Input:  Address of code to invoke in (CODE_ADD).
;          AF, BC, DE, HL, IX, IY passed to the called routine.
;  Output: AF, BC, DE, HL, IX, IY returned from the called routine.

CALLB0	equ	403Fh


;* Call a routine in another bank.
;  Must be used if the driver spawns across more than one bank.
;
;  Input:  A = bank number
;          IX = routine address
;          AF' = AF for the routine
;          HL' = Ix for the routine
;          BC, DE, HL, IY = input for the routine
;  Output: AF, BC, DE, HL, IX, IY returned from the called routine.

CALBNK	equ	4042h


;* Get in IX the address of the SLTWRK entry for the slot passed in A,
;  which will in turn contain a pointer to the allocated page 3
;  work area for that slot (0 if no work area was allocated).
;  If A=0, then it uses the slot currently switched in page 1.
;  Returns A=current slot for page 1, if A=0 was passed.
;  Corrupts F.
;  Must be called by using CALBNK to bank 0:
;    ld a,<slot number> (xor a for current page 1 slot)
;    ex af,af'
;    xor a
;    ld ix,GWORK
;    call CALBNK

GWORK	equ	4045h


;* This address contains one byte that tells how many banks
;  form the Nextor kernel (or alternatively, the first bank
;  number of the driver).

K_SIZE	equ	40FEh


;* This address contains one byte with the current bank number.

CUR_BANK	equ	40FFh


;-----------------------------------------------------------------------------
;
; Built-in format choice strings
;

NULL_MSG  equ     781Fh	;Null string (disk can't be formatted)
SING_DBL  equ     7820h ;"1-Single side / 2-Double side"


;-----------------------------------------------------------------------------
;
; Driver signature
;
	db	"NEXTOR_DRIVER",0


;-----------------------------------------------------------------------------
;
; Driver flags:
;    bit 0: 0 for drive-based, 1 for device-based
;    bit 2: 1 if the driver implements the DRV_CONFIG routine
;             (used by Nextor from v2.0.5)

if DRV_TYPE eq 0
	db	0+4
endif

if DRV_TYPE eq 1
	db	1+4
endif


;-----------------------------------------------------------------------------
;
; Reserved byte
;

	db	0


;-----------------------------------------------------------------------------
;
; Driver name
;

DRV_NAME:
	db	"PicoVerse Nextor Driver"
	ds	32-($-DRV_NAME)," "


;-----------------------------------------------------------------------------
;
; Jump table for the driver public routines
;

	; These routines are mandatory for all drivers
        ; (but probably you need to implement only DRV_INIT)

	jp	DRV_TIMI
	jp	DRV_VERSION
	jp	DRV_INIT
	jp	DRV_BASSTAT
	jp	DRV_BASDEV
    jp  DRV_EXTBIO
    jp  DRV_DIRECT0
    jp  DRV_DIRECT1
    jp  DRV_DIRECT2
    jp  DRV_DIRECT3
    jp  DRV_DIRECT4
	jp	DRV_CONFIG

	ds	12

if DRV_TYPE eq 1

	; These routines are mandatory for device-based drivers

	jp	DEV_RW
	jp	DEV_INFO
	jp	DEV_STATUS
	jp	LUN_INFO
endif


;=====
;=====  END of data that must be at fixed addresses
;=====


;-----------------------------------------------------------------------------
;
; Timer interrupt routine, it will be called on each timer interrupt
; (at 50 or 60Hz), but only if DRV_INIT returns Cy=1 on its first execution.

DRV_TIMI:
	ret


;-----------------------------------------------------------------------------
;
; Driver initialization routine, it is called twice:
;
; 1) First execution, for information gathering.
;    Input:
;      A = 0
;      B = number of available drives
;      HL = maximum size of allocatable work area in page 3
;      C: bit 5 set if user is requesting reduced drive count
;         (by pressing the 5 key)
;    Output:
;      A = number of required drives (for drive-based driver only)
;      HL = size of required work area in page 3
;      Cy = 1 if DRV_TIMI must be hooked to the timer interrupt, 0 otherwise
;
; 2) Second execution, for work area and hardware initialization.
;    Input:
;      A = 1
;      B = number of allocated drives for this controller
;      C: bit 5 set if user is requesting reduced drive count
;         (by pressing the 5 key)
;
;    The work area address can be obtained by using GWORK.
;
;    If first execution requests more work area than available,
;    second execution will not be done and DRV_TIMI will not be hooked
;    to the timer interrupt.
;
;    If first execution requests more drives than available,
;    as many drives as possible will be allocated, and the initialization
;    procedure will continue the normal way
;    (for drive-based drivers only. Device-based drivers always
;     get two allocated drives.)

DRV_INIT:

	ld	de,strCopyRight
	jp	printString

    ; Send read command to test if device is available (TEST)
    ld   a, 1
    out  (21), a

	xor	a
	ld	hl,0
	ret


;-----------------------------------------------------------------------------
;
; Obtain driver version
;
; Input:  -
; Output: A = Main version number
;         B = Secondary version number
;         C = Revision number

DRV_VERSION:
	ld	a,VER_MAIN
	ld	b,VER_SEC
	ld	c,VER_REV
	ret


;-----------------------------------------------------------------------------
;
; BASIC expanded statement ("CALL") handler.
; Works the expected way, except that if invoking CALBAS is needed,
; it must be done via the CALLB0 routine in kernel page 0.

DRV_BASSTAT:
	scf 	; Not handling BASIC extended statements
	ret


;-----------------------------------------------------------------------------
;
; BASIC expanded device handler.
; Works the expected way, except that if invoking CALBAS is needed,
; it must be done via the CALLB0 routine in kernel page 0.

DRV_BASDEV:
	scf	; Not handling BASIC extended devices
	ret


;-----------------------------------------------------------------------------
;
; Extended BIOS hook.
; Works the expected way, except that it must return
; D'=1 if the old hook must be called, D'=0 otherwise.
; It is entered with D'=1.

DRV_EXTBIO:
	ret


;-----------------------------------------------------------------------------
;
; Direct calls entry points.
; Calls to addresses 7850h, 7853h, 7856h, 7859h and 785Ch
; in kernel banks 0 and 3 will be redirected
; to DIRECT0/1/2/3/4 respectively.
; Receives all register data from the caller except IX and AF'.

DRV_DIRECT0:
DRV_DIRECT1:
DRV_DIRECT2:
DRV_DIRECT3:
DRV_DIRECT4:
	ret


;-----------------------------------------------------------------------------
;
; Get driver configuration 
; (bit 2 of driver flags must be set if this routine is implemented)
;
; Input:
;   A = Configuration index
;   BC, DE, HL = Depends on the configuration
;
; Output:
;   A = 0: Ok
;       1: Configuration not available for the supplied index
;   BC, DE, HL = Depends on the configuration
;
; * Get number of drives at boot time (for device-based drivers only):
;   Input:
;     A = 1
;     B = 0 for DOS 2 mode, 1 for DOS 1 mode
;     C: bit 5 set if user is requesting reduced drive count
;        (by pressing the 5 key)
;   Output:
;     B = number of drives
;
; * Get default configuration for drive
;   Input:
;     A = 2
;     B = 0 for DOS 2 mode, 1 for DOS 1 mode
;     C = Relative drive number at boot time
;   Output:
;     B = Device index
;     C = LUN index

DRV_CONFIG:
	ld a,1
	ret
	
;=====
;=====  BEGIN of DEVICE-BASED specific routines
;=====

if DRV_TYPE eq 1

;-----------------------------------------------------------------------------
;
; Read or write logical sectors from/to a logical unit
;
;Input:    Cy=0 to read, 1 to write
;          A = Device number, 1 to 7
;          B = Number of sectors to read or write
;          C = Logical unit number, 1 to 7
;          HL = Source or destination memory address for the transfer
;          DE = Address where the 4 byte sector number is stored.
;Output:   A = Error code (the same codes of MSX-DOS are used):
;              0: Ok
;              .IDEVL: Invalid device or LUN
;              .NRDY: Not ready
;              .DISK: General unknown disk error
;              .DATA: CRC error when reading
;              .RNF: Sector not found
;              .UFORM: Unformatted disk
;              .WPROT: Write protected media, or read-only logical unit
;              .WRERR: Write error
;              .NCOMP: Incompatible disk.
;              .SEEK: Seek error.
;          B = Number of sectors actually read (in case of error only)

DEV_RW:
	
	; Send read command to test if device is available
    ld   a, CMD_NEXTOR_READ_SECTOR
    out  (PORT_CMD), a
	ret

DEV_RW2:

	; Save flag state in A: 
	; Using "ld a,0" then "adc a,a" yields 0 if CF clear, 1 if CF set.
	ld   a,0
	adc  a,a        ; A = 0 if read, 1 if write
	cp   0
	jp   z, DEV_RW_READ_ENTRY
	jp   DEV_RW_WRITE_ENTRY

;----------------------------------------
; Read Branch (CF=0)
;----------------------------------------
DEV_RW_READ_ENTRY:

    push af         ; save our flag/state (also device index flag result is in A)
    push bc
    push de
    push hl

    ; Send read command
    ld   a, CMD_NEXTOR_READ_SECTOR
    out  (PORT_CMD), a

    ; Send 4-byte sector address from DE
    ld   a, (de)
    out  (PORT_DATA), a
    inc  de
    ld   a, (de)
    out  (PORT_DATA), a
    inc  de
    ld   a, (de)
    out  (PORT_DATA), a
    inc  de
    ld   a, (de)
    out  (PORT_DATA), a
    inc  de     ; now DE is advanced past the 4-byte address

    ; Send sector count (from B)
    ld   a, b
    out  (PORT_DATA), a

    ; Copy sector count from B into D (we use D as outer loop counter)
    ld   d, b

DEV_RW_READ_SECTOR_LOOP:
        
	; If no more sectors to read, branch to DONE
    ld   a, d
    or   a
    jr   z, DEV_RW_READ_DONE

    ; For each sector, read 512 bytes
    ; Use IX as a 16-bit inner loop counter.
    ld   ix, 0x0200   ; 512 iterations

DEV_RW_READ_INNER_LOOP:
        
	in   a, (PORT_DATA)   ; read one byte from Pico via PORT_DATA
    ld   (hl), a        ; store it in buffer
    inc  hl             ; advance buffer pointer
    dec  ix             ; decrement inner counter (assumes DEC IX is supported)
    jr   nz, DEV_RW_READ_INNER_LOOP

    ; One sector done; decrement outer counter in D
    dec  d
    jr   DEV_RW_READ_SECTOR_LOOP

DEV_RW_READ_DONE:
        
	; Return success: A = 0
    ld   a, 0

    pop  hl
    pop  de
    pop  bc
    pop  af
	ret

;----------------------------------------
; Write Branch (CF=1)
;----------------------------------------
DEV_RW_WRITE_ENTRY:
        
	push af
    push bc
    push de
    push hl

    ; Send write command
    ld   a, CMD_NEXTOR_WRITE_SECTOR
    out  (PORT_CMD), a

    ; Send 4-byte sector address from DE
    ld   a, (de)
    out  (PORT_DATA), a
    inc  de
    ld   a, (de)
    out  (PORT_DATA), a
    inc  de
    ld   a, (de)
    out  (PORT_DATA), a
    inc  de
    ld   a, (de)
    out  (PORT_DATA), a
    inc  de

    ; Send sector count (from B)
    ld   a, b
    out  (PORT_DATA), a

    ; Copy sector count from B into D (outer loop counter)
    ld   d, b

DEV_RW_WRITE_LOOP:
        
	; If no more sectors to write, finish loop
    ld   a, d
    or   a
    jr   z, DEV_RW_WRITE_DONE

    ; For each sector, write 512 bytes from buffer pointed to by HL.
    ld   ix, 0x0200   ; 512 iterations

DEV_RW_WRITE_INNER_LOOP:

    ld   a, (hl)     ; get one byte from buffer
    out  (PORT_DATA), a  ; send it to Pico
    inc  hl         ; advance buffer pointer
    dec  ix         ; decrement inner loop counter
    jr   nz, DEV_RW_WRITE_INNER_LOOP

    ; One sector done; decrement outer counter D
    dec  d
    jr   DEV_RW_WRITE_LOOP

DEV_RW_WRITE_DONE:
        
	; After writing, send CTRL_SYNC command to ensure data integrity.
    ;ld   a, CTRL_SYNC
    ;out  (PORT_CMD), a

    ; Return success.
    ld   a, 0

    pop  hl
    pop  de
    pop  bc
    pop  af
    ret

;-----------------------------------------------------------------------------
;
; Device information gathering
;
;Input:   A = Device index, 1 to 7
;         B = Information to return:
;             0: Basic information
;             1: Manufacturer name string
;             2: Device name string
;             3: Serial number string
;         HL = Pointer to a buffer in RAM
;Output:  A = Error code:
;             0: Ok
;             1: Device not available or invalid device index
;             2: Information not available, or invalid information index
;         When basic information is requested,
;         buffer filled with the following information:
;
;+0 (1): Numer of logical units, from 1 to 7. 1 if the device has no logical
;        units (which is functionally equivalent to having only one).
;+1 (1): Device flags, always zero in Beta 2.
;
; The strings must be printable ASCII string (ASCII codes 32 to 126),
; left justified and padded with spaces. All the strings are optional,
; if not available, an error must be returned.
; If a string is provided by the device in binary format, it must be reported
; as an hexadecimal, upper-cased string, preceded by the prefix "0x".
; The maximum length for a string is 64 characters;
; if the string is actually longer, the leftmost 64 characters
; should be provided.
;
; In the case of the serial number string, the same rules for the strings
; apply, except that it must be provided right-justified,
; and if it is too long, the rightmost characters must be
; provided, not the leftmost.

DEV_INFO:
;ld	a,1
	ld a, 0 
	ret


;-----------------------------------------------------------------------------
;
; Obtain device status
;
;Input:   A = Device index, 1 to 7
;         B = Logical unit number, 1 to 7
;             0 to return the status of the device itself.
;Output:  A = Status for the specified logical unit,
;             or for the whole device if 0 was specified:
;                0: The device or logical unit is not available, or the
;                   device or logical unit number supplied is invalid.
;                1: The device or logical unit is available and has not
;                   changed since the last status request.
;                2: The device or logical unit is available and has changed
;                   since the last status request
;                   (for devices, the device has been unplugged and a
;                    different device has been plugged which has been
;                    assigned the same device index; for logical units,
;                    the media has been changed).
;                3: The device or logical unit is available, but it is not
;                   possible to determine whether it has been changed
;                   or not since the last status request.
;
; Devices not supporting hot-plugging must always return status value 1.
; Non removable logical units may return values 0 and 1.
;
; The returned status is always relative to the previous invokation of
; DEV_STATUS itself. Please read the Driver Developer Guide for more info.

DEV_STATUS:
	;xor	a
	ld a, 1
	ret


;-----------------------------------------------------------------------------
;
; Obtain logical unit information
;
;Input:   A  = Device index, 1 to 7
;         B  = Logical unit number, 1 to 7
;         HL = Pointer to buffer in RAM.
;Output:  A = 0: Ok, buffer filled with information.
;             1: Error, device or logical unit not available,
;                or device index or logical unit number invalid.
;         On success, buffer filled with the following information:
;
;+0 (1): Medium type:
;        0: Block device
;        1: CD or DVD reader or recorder
;        2-254: Unused. Additional codes may be defined in the future.
;        255: Other
;+1 (2): Sector size, 0 if this information does not apply or is
;        not available.
;+3 (4): Total number of available sectors.
;        0 if this information does not apply or is not available.
;+7 (1): Flags:
;        bit 0: 1 if the medium is removable.
;        bit 1: 1 if the medium is read only. A medium that can dinamically
;               be write protected or write enabled is not considered
;               to be read-only.
;        bit 2: 1 if the LUN is a floppy disk drive.
;        bit 3: 1 if this LUN shouldn't be used for automapping.
;        bits 4-7: must be zero.
;+8 (2): Number of cylinders
;+10 (1): Number of heads
;+11 (1): Number of sectors per track
;
; Number of cylinders, heads and sectors apply to hard disks only.
; For other types of device, these fields must be zero.

LUN_INFO:
	ld	a,1
	ret

endif


;=====
;=====  END of DEVICE-BASED specific routines
;=====

; ------------------------------------------------
printString:
; Prints an ASCII string that has the last bit7 set
; Input   : DE = Pointer to the string
; Modifies: AF, DE, EI 
; ------------------------------------------------
	ld	a,(de)
	bit	7,a
	res	7,a
	call	CHPUT
	ret	nz
	inc	de
	jr	printString


strCopyright:
	db	"PicoVerse Nextor 2.1 Driver",13,10
	db	"(c) 2025 The Retro Hacker",13,10
.end:

;-----------------------------------------------------------------------------
;
; End of the driver code

DRV_END:

	ds 3DD0h-(DRV_END-DRV_START)
	;ds 3FD0h-(DRV_END-DRV_START)


	end
