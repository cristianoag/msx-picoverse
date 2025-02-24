; -----------------------------------------------------------------------------
; Nextor SD Driver for MSX PicoVerse
; Version: 1.0
; Author: The Retro Hacker
; Copyright (C) 2025 The Retro Hacker
;
; This driver is an integral part of the MSX PicoVerse project, providing
; reliable SD card support for MSX systems.
;
; For documentation, updates, and support, please visit:
;   https://github.com/cristianoag/msx-picoverse
; -----------------------------------------------------------------------------

; SJASMPLUS assembler instruction to create the binary file
	output	"build/driver.bin"

; This will be removed soon... 
; Uses HW (1) or SW (0) disk-change:
HWDS = 1

;-----------------------------------------------------------------------------
;
; Miscellaneous constants
;

;This is a 2 byte buffer to store the address of code to be executed.
;It is used by some of the kernel page 0 routines.

CODE_ADD:	equ	0F84Ch

;-----------------------------------------------------------------------------
;
; Driver configuration constants
;

;Driver type:
;   0 for drive-based
;   1 for device-based

DRV_TYPE	equ	1

;Hot-plug devices support (device-based drivers only):
;   0 for no hot-plug support
;   1 for hot-plug support

DRV_HOTPLUG	equ	0 ; This driver does not support hot-plugging at this moment

;Driver version
VER_MAIN	equ	1
VER_SEC		equ	0
VER_REV		equ	0

;-----------------------------------------------------------------------------
;
; Error codes for DEV_RW
;

ENCOMP	equ	0FFh
EWRERR	equ	0FEh
EDISK	equ	0FDh
ENRDY	equ	0FCh
EDATA	equ	0FAh
ERNF	equ	0F9h
EWPROT	equ	0F8h
EUFORM	equ	0F7h
ESEEK	equ	0F3h
EIFORM	equ	0F0h
EIDEVL	equ	0B5h
EIPARM	equ	08Bh

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

NULL_MSG  equ     781Fh		;Null string (disk can't be formatted)
SING_DBL  equ     7820h 	;"1-Single side / 2-Double side"

; -----------------------------------------------------------------------------
BIOS_INITXT	= $6C		; Screen0 initialization
BIOS_CHPUT	= $A2		; A=char
BIOS_CLS	= $C3		; Call with A=0
LINL40		= $F3AE		; Width
LINLEN		= $F3B0

; IO Ports used by the driver
; Details on the protocol available at: https://github.com/cristianoag/msx-picoverse/wiki/PicoVerse-2350-Nextor-Driver-Protocol

PORTCFG		= $9E
PORTSTATUS	= $9E
PORTDATA	= $9F

; PicoVerse Nextor Protocol Commands
; Send commands on the PORTCFG and get the response on the PORTSTATUS
CMD_01	= 0x01	; SD card initialization, return 1 byte (0x00 = success, 0xFF = failure)
CMD_02	= 0x02	; Get SD card presence, return 1 byte (0x00 = present, 0xFF = not present)
CMD_03	= 0x03  ; Get SD card manufacturer ID, return 1 byte (byte of the manufacturer, 0xFF = failure)
CMD_04	= 0x04  ; Get SD card serial number, return 1 byte (serial number, 0xFF = failure)
CMD_05	= 0x05  ; Get SD card number of sectors, return 4 bytes (little-endian format, 1 byte 0xFF = failure)
CMD_06  = 0x06   ; Read a block (or multiple blocks), return multiples of 512 bytes (data from the SD card, 1 byte 0xFF = failure)
CMD_07  = 0x07   ; Continue reading blocks, return multiples of 512 bytes (data from the SD card, 1 byte 0xFF = failure)

	org	$4000

	ds	256, $FF	; 256 dummy bytes

DRV_START:

;-----------------------------------------------------------------------------
;
; Driver signature
;
	db	"NEXTOR_DRIVER",0


;-----------------------------------------------------------------------------
;
; Driver flags:
;    bit 0: 0 for drive-based, 1 for device-based
;    bit 1: 1 for hot-plug devices supported (device-based drivers only)
;    bit 2: 1 if the driver provides configuration
;             (implements the DRV_CONFIG routine)


	db	1


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
	db	"PicoVerse microSD Driver"
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
	jp	DRV_EXTBIO
	jp	DRV_DIRECT0
	jp	DRV_DIRECT1
	jp	DRV_DIRECT2
	jp	DRV_DIRECT3
	jp	DRV_DIRECT4
	jp      DRV_CONFIG

    ds      12


	; These routines are mandatory for device-based drivers

	jp	DEV_RW
	jp	DEV_INFO
	jp	DEV_STATUS
	jp	LUN_INFO


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
	; first nextor call
	or		a			; first call
	;ld	hl,WRKAREA
	ld		hl, 41		; no work area needed
	ret 	z			

	; second nextor call
	call	BIOS_INITXT			; initialize screen
	xor		a				
	call	BIOS_CLS			; clear screen
	ld		de, STRTITLE		; load title text
	call	PRINTSTRING 		; print title
 
	ld 		de, STRCARD			; load card text
	call 	PRINTSTRING			; print card text

	ld		a, CMD_01			; set A with the command to initialize the SD card	
	out		(PORTCFG), a		; send the command to the config port
	ld 		bc, 0xFF			; this one needs to be long enough to wait for the card to initialize
	ld 		e, 2				; set E to 2 for the delay loop
	call	DELAY				; wait to initialize the card and then read the status
	in		a, (PORTSTATUS)		; read which will have the info if the card is detected or not
	cp		0					; check if a is 0
	jr		z, DETECTED			; if a is 0, jump to DETECTED
	ld		de, STRNOTDETECTED	; if a is not 0, then we need to load the no card text
	call	PRINTSTRING			; print the no card text
	
	ret

DETECTED:
	ld		de, STRDETECTED	; load the card detected text
	call	PRINTSTRING		; print the card detected text

	ld		a, CMD_03		; set A with the command to return the manufacturer ID
	out 	(PORTCFG), a	; send the command to the port

	ld		a, '['
	call	BIOS_CHPUT		; print the opening bracket

	in		a, (PORTSTATUS)	; read the byte with the manufacturer ID
	call	GETMANUFACTURER	; get the name of the manufacturer on the table
	ex		de, hl			; exchange DE and HL
	call	PRINTSTRING	

	ld		a, ']'
	call	BIOS_CHPUT		; print the closing bracket
	ld		de, STRCRLF
	call	PRINTSTRING		; print CRLF
	;ld 		bc, 0xFF		
	;ld 		e, 2
	;call	DELAY			; wait to have the user reading the screen

	ret

; -----------------------------------------------------------------------------
; DELAY LOOP
; set the bc and e registers with appropriate values before entering the loop. 
; The values you choose will determine the duration of the delay.
; The loop will continue until both bc and e reach zero.
;-----------------------------------------------------------------------------
DELAY:
    nop             
    dec 	bc
    ld 		a, c
    or 		b
    jr 		nz, DELAY
    dec 	e
    jr 		nz, DELAY
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
	scf
	ret

;-----------------------------------------------------------------------------
;
; BASIC expanded device handler.
; Works the expected way, except that if invoking CALBAS is needed,
; it must be done via the CALLB0 routine in kernel page 0.

DRV_BASDEV:
	scf
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

    ld 	a,1
	ld  b,1
    ret


;=====
;=====  BEGIN of DEVICE-BASED specific routines
;=====

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

	cp 		1							; Check if device index	is 1
	jr 		nz, DEV_RW_ERROR_EIDEVL     ; if not jumps to error and set it to EIDEVL
	ld 		a, b                    	; loads the number of blocks to read/write in A
	ld		(WRKAREA.NUMBLOCKS), a		; store the number of blocks to read/write in WRKAREA.NUMBLOCKS

	ld		a, c			;Cy=0 to read, 1 to write
	or		a				; check if it is a read or write operation			
	jp		z, DEV_RW_READ  ; If zero, select read operation
	cp		1				; check if it is a write operation
	jp		z, DEV_RW_WRITE ; If zero, select write operation

	ld		a, EIPARM	; Invalid parameters
	ret

DEV_RW_READ:

	;-- Load the 32-bit block number from memory (non–page 1) pointed by DE --

	ld		a, (de)		; first byte of the block number, this goes to E
	push	af
	inc		de
	ld		a, (de)		; second byte of the block number, this goes to D
	push	af
	inc		de
	ld		a, (de)		; third byte of the block number, this goes to C
	ld		c, a
	inc		de
	ld		a, (de)		; fourth byte of the block number, this goes to B
	inc		de
	ld		b, a
	pop		af
	ld		d, a
	pop		af			
	ld		e, a		; Now BC:DE holds the 32-bit block number.

	call DEV_RW_READ_BLOCK
	jr   nc, DEV_RW_READ_OK

	ld   a, EDISK   ; General unknown disk error
	ld   b, 0      ; Indicate that 0 blocks were read
	ret

DEV_RW_READ_OK:
	xor  a         ; Return 0 (ok)
	ret

DEV_RW_WRITE:
	ret

DEV_RW_ERROR_EIDEVL:
	ld 	a, EIDEVL	; Invalid device or LUN
	ret

; ------------------------------------------------
; Read a 512-byte block from the card
; HL points to the start of the data
; BC and DE contain the block number (BCDE = 32 bits)
; Destroys AF, BC, DE, HL
; ------------------------------------------------
DEV_RW_READ_BLOCK:

	ld 		a, CMD_06		;	set A with the command to read a block
	out		(PORTCFG), a	;	send the command to the port

	;; now we need to send the address of the block to be read
	;; Send block address (4 bytes)

	ld 		a, b			; first byte of the block number
	out     (PORTCFG), a	
	ld 		a, c			; second byte
	out     (PORTCFG), a	
	ld 		a, d			; third byte
	out     (PORTCFG), a	
	ld 		a, e			; fourth byte
	out     (PORTCFG), a	

	ld      a, CMD_06		; Begin the first block read
	out     (PORTCFG), a	 

.loop_read:
	; Read 512 bytes from PORTDATA into memory at HL
	ld 		c, PORTDATA 	; set C to the port to read the data from
	ld 		b, 255			; Read 256 bytes...
	inir					; ...and auto-increment HL
	ld 		b, 255			; Repeat for another 256 bytes
	inir					

	; Test if there are more blocks to read
	ld    	a, (WRKAREA.NUMBLOCKS)
	cp 		1
	jr   	z, .exit_read_loop  ; If equal to 1, we have read the last block; exit loop

	; More blocks remain: decrement the counter and request next block
	dec    	a 						; Decrement block counter by 1
	ld    	(WRKAREA.NUMBLOCKS), a  ; Store the new block count
	ld   	a, CMD_07         ; Set A with the continuation command (CMD_07)
	out 	(PORTCFG), a      ; Send CMD_07 to the configuration port    
	jr   	.loop_read        ; Then loop to read the next 512-byte block

.exit_read_loop:
	ret

DEV_RW_WRITE_BLOCK:
	ret

;-----------------------------------------------------------------------------
; Device information gathering - Extended DEV_INFO implementation
;
; Input:   A = Device index, only device 1 is supported
;          B = Information index:
;              0: Basic information
;              1: Manufacturer name string
;              2: Device name string
;              3: Serial number string
;          HL = Pointer to a buffer in RAM
; Output:  A = Error code:
;              0: Ok
;              1: Device not available or invalid device index
;              2: Information not available, or invalid information index
;-----------------------------------------------------------------------------

DEV_INFO:

	cp 	1
	jr 	nz, DEV_INFO_ERROR      ; Only device index 1 supported

	ld 	a, b                   	; Check the requested information index
	cp 	0
	jr 	z, DEV_INFO_BASIC      	; If 0, return basic information
	cp 	1
	jr 	z, DEV_INFO_MANUFACTURER
	cp 	2
	jr 	z, DEV_INFO_DEVICENAME
	cp 	3
	jr 	z, DEV_INFO_SERIAL

	;ld 	a, 2                   	; Otherwise, error code 2: Information not available
	;ret

DEV_INFO_BASIC:
	; Fill buffer with basic information:
	; +0: Number of logical units (1)
	; +1: Device flags (0)
	ld      a, $1
	ld 		(hl), a       	; One logical unit
	inc 	hl     
	xor 	a       
	ld 		(hl), a		; Set device flags = 0
	ret

DEV_INFO_MANUFACTURER:
	push	hl				; save pointer to the buffer
	ld		b, 64			; Set B to 64 (number of bytes to fill)
	ld		a, ' ' 			; Load A with the ASCII code for space
.loop1:
	ld		(hl), a			; Store space at the address pointed to by HL
	inc		hl				; Increment HL to point to the next byte
	djnz	.loop1			; Decrement B and repeat until B is zero
	pop		de				; recover pointer in DE
	ld		a, CMD_03		; set A with the command to return the manufacturer ID
	out 	(PORTCFG), a	; send the command to the config port
	in		a, (PORTSTATUS)	; read the byte with the manufacturer ID
	call	GETMANUFACTURER	; get the name of the manufacturer on the table
	ldir					; Copy the manufacturer name string to the buffer at HL
	xor		a				; Clear A (set to 0)
	ret						; Return from the routine

DEV_INFO_DEVICENAME:

	push	hl					; save pointer to the buffer
	ld		b, 64				; fill 64 bytes with spaces
	ld		a, ' '				; load A with the ASCII code for space
.loop2:
	ld		(hl), a				; store space at the address pointed to by HL
	inc		hl					; increment HL to point to the next byte
	djnz	.loop2
	pop		de					; recover pointer in DE
	ld 		de, STR_DEVICENAME  ; DE points to null-terminated device name string
	ex		de, hl				; HL points to the buffer
	xor 	a					; Clear A (set to 0)
	ret

DEV_INFO_SERIAL:
	push	hl				; save pointer to the buffer
	ld		b, 64			; fill 64 bytes with spaces
	ld		a, ' '			; load A with the ASCII code for space
.loop3:
	ld		(hl), a			; store space at the address pointed to by HL
	inc		hl				; increment HL to point to the next byte
	djnz	.loop3
	pop		de				; recover pointer in DE
					
	; Prepare the destination buffer starting with "0x"
	ld		hl, DEV_INFO_SERIAL_BUFFER ; HL will point to the start of the string buffer
	ld		(hl), '0'	; Store '0' at the start of the buffer
	inc		hl			; Increment HL to point to the next byte
	ld		(hl), 'x'	; Store 'x' at the next position in the buffer
	inc		hl			; Increment HL to point to the next byte

	ld 		a, CMD_04       ; set A with the command to return the serial number (0x04)
	out 	(PORTCFG), a    ; send the command to the config port
	; next four readings from PORTSTATUS will provide the serial number in little-endian format (32 bits)
.serial_read_loop:
    in      a, (PORTSTATUS)   	; Read sector byte from the status port
	call 	HEXTOASCII      	; convert the serial number byte in A to ASCII, now DE points to the converted ASCII characters (2)
	; Copy the converted ASCII characters from DE to the buffer at HL
	ld		b, 2    ; copy 2 bytes (hexadecimal representation for one byte)
.copy_loop:
	ld		a, (de)	; Load the next byte from DE into A
	ld		(hl), a	; Store the byte at the address pointed to by HL
	inc		hl	; Increment HL to point to the next byte
	inc		de		; Increment DE to point to the next byte
	djnz	.copy_loop
	djnz    .serial_read_loop ; Loop until all 4 bytes are read

	ld a, 0	; Clear A (set to 0)
	ret

; Define a buffer for the final microSD card serial string
DEV_INFO_SERIAL_BUFFER:
	ds		4

DEV_INFO_ERROR:
	ld a, 1       ; Error: device index not valid
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
	ld a, 1	; Device is available and has not changed since the last status request
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

	cp      2           ; only one device supported
	jr      nc, LUN_INFO_ERROR

	xor     a
	ld      (hl), a     ; +0: Medium type = 0 (block device)
	inc     hl
	ld      (hl), a     ; +1: Low byte of sector size (0x00)
	inc     hl
	ld      a, 2
	ld      (hl), a     ; +2: High byte of sector size (0x02 -> 512 bytes)
	inc     hl

	; next four commands bring readings from PORTSTATUS with four bytes with the number of sectors 
	; in little-endian format
    ld      b, 4             ; Set loop counter for 4 sector bytes

    ld      a, CMD_05        ; Set A with command 0x05 to read the number of sectors
    out     (PORTCFG), a     ; Send the command
							 ; next four readings from PORTSTATUS will provide the number of sectors in little-endian format (32 bits)

	; still dont know if this is required
	;ld 		bc, 0xFF		
	;ld 		e, 2
	;call	DELAY			; wait to have the user reading the screen
.sector_read_loop:
	nop						  ; NOP instruction to allow time for the command to be processed
    in      a, (PORTSTATUS)   ; Read sector byte from the status port
    ld      (hl), a           ; Store it
    inc     hl				  ; Increment HL to point to the next byte
    djnz    .sector_read_loop ; Loop until all 4 bytes are read

	ld      (hl), 0     ; +7: Flags = device is not removable
	inc     hl
	ld      (hl), 0     ; +8: Number of cylinders = 0 (not applicable)
	inc     hl
	ld      (hl), 0     ; +9: Number of heads = 0
	inc     hl
	ld      (hl), 0     ; +10: Sectors per track = 0

	ld	  a, 0      ; return 0 (ok)
	ret

LUN_INFO_ERROR:
	ld      a, 1        ; report error
	ret


;=====
;=====  END of DEVICE-BASED specific routines
;=====

; ------------------------------------------------
; Prints the string on the screen pointed by DE
; Destroys all registers
; ------------------------------------------------
PRINTSTRING:
	ld	a, (de)
	or	a
	ret	z
	call	BIOS_CHPUT
	inc	de
	jr	PRINTSTRING

; ------------------------------------------------
; Convert the byte in A to a decimal string in
; the buffer pointed to by DE
; Destroys AF, BC, HL, DE
; ------------------------------------------------
DECTOASCII:
	ld	iy, WRKAREA.TEMP
	ld	h, 0
	ld	l, a		; copiar A para HL
	ld	(iy+0), 1	; flag para indicar que devemos cortar os zeros a esquerda
	ld	bc, -100	; centenas
	call	.num1
	ld	c, -10		; dezenas
	call	.num1
	ld	(iy+0), 2	; unidade deve exibir 0 se for zero e nao corta-lo
	ld	c, -1		; unidades
.num1:
	ld	a, '0'-1
.num2:
	inc	a		; contar o valor em ascii de '0' a '9'
	add	hl, bc		; somar com negativo
	jr	c, .num2	; ainda nao zeramos
	sbc	hl, bc		; retoma valor original
	dec	(iy+0)		; se flag do corte do zero indicar para nao cortar, pula
	jr	nz, .naozero
	cp	'0'		; devemos cortar os zeros a esquerda. Eh zero?
	jr	nz, .naozero
	inc	(iy+0)		; se for zero, nao salvamos e voltamos a flag
	ret
.naozero:
	ld	(de), a		; eh zero ou eh outro numero, salvar
	inc	de		; incrementa ponteiro de destino
	ret

; ------------------------------------------------
; Converte o byte em A para string em hexa no
; buffer apontado por DE
; Destroi AF, C, DE
; ------------------------------------------------
HEXTOASCII:
	ld	c, a
	rra
	rra
	rra
	rra
	call	.conv
	ld  	a, c
.conv:
	and	$0F
	add	a, $90
	daa
	adc	a, $40
	daa
	ld	(de), a
	inc	de
	ret

; ------------------------------------------------
; Converts the byte in A to a decimal string and
; prints it on the screen
; Destroys registers: AF, BC, HL, DE
; ------------------------------------------------
PRINTDECTOASCII:
	ld		h, 0
	ld		l, a		; copy A into HL
	ld		b, 1		; flag to indicate that leading zeros should be trimmed
	ld		de, -100	; hundreds
	call	.num1
	ld		e, -10		; tens
	call	.num1
	ld		b, 2		; units: should display 0 if zero and do not trim it
	ld		e, -1		; units
.num1:
	ld		a, '0'-1
.num2:
	inc		a			; count the ASCII value from '0' to '9'
	add		hl, de		; add the negative value
	jr		c, .num2	; not zeroed yet
	sbc		hl, de		; restore the original value
	djnz	.notzero	; if the trim flag indicates not to trim, jump
	cp		'0'			; we should trim leading zeros. Is it zero?
	jr		nz, .notzero
	inc		b			; if it is zero, do not print and restore the flag
	ret
.notzero:
	push	hl		; not zero or some other number, so print
	push	bc
	call	BIOS_CHPUT
	pop		bc
	pop		hl
	ret

; ------------------------------------------------
; Search for the manufacturer's name in a table.
; A contains the manufacturer's byte.
; Returns HL pointing to the manufacturer's buffer
; and BC with the length of the text.
; Destroys AF, BC, HL
; ------------------------------------------------
GETMANUFACTURER:
	ld		c, a
	ld		hl, TBLMANUFACTURERS

.loop:
	ld		a, (hl)
	inc		hl
	cp		c
	jr		z, .found
	or		a
	jr		z, .found
	push	bc
	call	.found
	add		hl, bc
	inc		hl
	pop		bc
	jr		.loop

.found:
	ld		c, 0
	push	hl
	xor		a
.loop2:
	inc		c
	inc		hl
	cp		(hl)
	jr		nz, .loop2
	pop		hl
	ld		b, 0
	ret

; ---------------------------------------------------------------------------

TBLMANUFACTURERS:
	db	1
	db	"Panasonic",0
	db	2
	db	"Toshiba",0
	db	3
	db	"SanDisk",0
	db	4
	db	"SMI-S",0
	db	6
	db	"Renesas",0
	db	17
	db	"Dane-Elec",0
	db	19
	db	"KingMax",0
	db	21
	db	"Samsung",0
	db	24
	db	"Infineon",0
	db	26
	db	"PQI",0
	db	27
	db	"Sony",0
	db	28
	db	"Transcend",0
	db	29
	db	"A-DATA",0
	db	31
	db	"SiliconPower",0
	db	39
	db	"Verbatim",0
	db  62
	db "RetroWiki VHD MiSTer",0
	db	65
	db	"OKI",0
	db	115
	db	"SilverHT",0
	db  116
	db  "RetroWiki VHD MiST",0
	db	137
	db	"L.Data",0
	db	0
	db	"Generic",0

; ------------------------------------------------
STRTITLE:
	db	"MSX PICOVERSE 2350",13,10
	db	"SD Nextor Driver Version "
	db	VER_MAIN + $30, '.', VER_SEC + $30, '.', VER_REV + $30
	db	13, 10
	db	"Copyright (c) 2025 - The Retro Hacker",13,10
STRCRLF:
	db	13,10,0
STRCARD:
	db	"Card: ",0
STRNOTDETECTED:
	db	"Not detected!",13,10,0
STRDETECTED:
	db	"Detected ",0
strEntrei:
	db	"Entrei",13,10,0
STR_DEVICENAME:
	db "microSD card",0

; RAM area
	org		$7000

;-----------------------------------------------------------------------------
;
; End of the driver code
; Work area variables
WRKAREA:
WRKAREA.BCSD 		ds 16	; Card Specific Data
WRKAREA.BCID1		ds 16	; Card-ID of card1
WRKAREA.NUMSD		ds 1	; Currently selected card: 1 or 2
WRKAREA.NUMBLOCKS	ds 1	; Number of blocks in multi-block operations
WRKAREA.BLOCKS1	    ds 3	; 3 bytes. Size of card1, in blocks.
WRKAREA.BLOCKS2	    ds 3	; 3 bytes. Size of card2, in blocks.
WRKAREA.TEMP		ds 1	; Temporary data
; ENDS

DRV_END:

	ds	3ED0h-(DRV_END-DRV_START)
	end

