; -----------------------------------------------------------------------------
; Nextor SD Driver for MSX PicoVerse
; Copyright (C) 2025 The Retro Hacker
; 
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
PORTSPI		= $9F ; This line will be removed soon

; SPI Commands (deprecated, will be removed soon)
CMD0	= 0  | $40
CMD1	= 1  | $40
CMD8	= 8  | $40
CMD9	= 9  | $40
CMD10	= 10 | $40
CMD12	= 12 | $40
CMD16	= 16 | $40
CMD17	= 17 | $40
CMD18	= 18 | $40
CMD24	= 24 | $40
CMD25	= 25 | $40
CMD55	= 55 | $40
CMD58	= 58 | $40
ACMD23	= 23 | $40
ACMD41	= 41 | $40

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


	db	1+4


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
;    Output:
;      A = number of required drives (for drive-based driver only)
;      HL = size of required work area in page 3
;      Cy = 1 if DRV_TIMI must be hooked to the timer interrupt, 0 otherwise
;
; 2) Second execution, for work area and hardware initialization.
;    Input:
;      A = 1
;      B = number of allocated drives for this controller
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

	ld		a, 0x01		
	out		(PORTCFG), a		; attempt to initialize the SD card
	ld 		bc, 0xFF
	ld 		e, 2
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

	ld		a, 3			; send the command to return the manufacturer ID
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
	ld 		bc, 0xFF
	ld 		e, 2
	call	DELAY			; wait to have the user reading the screen

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
	push	af
	cp	2		; somente 1 dispositivo
	jr	nc, .saicomerroidl
	dec	c		; somente 1 logical unit
	jr	z, .ok
.saicomerroidl:
	pop	af		; retira AF guardado no inicio
	ld	a, EIDEVL	; informar erro
	ld	b, 0
	ret
 IF HWDS = 0
.errornr:
	pop	af		; retira AF guardado no inicio
	ld	a, ENRDY	; Not ready
;	ld	a, EDISK	; General unknown disk error
	ld	b, 0
	ret
 ENDIF
.ok:
 IF HWDS = 0
	call	checkSWDS
	jr	c, .errornr
 ENDIF
 	ld	a, b
	ld	(WRKAREA.NUMBLOCKS), a	; guarda numero de blocos para ler/gravar
	push	hl
	call	calculaCIDoffset	; calculamos em IX a posicao correta do offset CID dependendo do cartao atual
	pop	hl
	pop	af		; retira AF guardado no inicio, para saber se eh leitura ou escrita
	jr	c, escrita	; se for escrita pulamos
leitura:
	ld	a, (de)		; 1. n. bloco
	push	af
	inc	de
	ld	a, (de)		; 2. n. bloco
	push	af
	inc	de
	ld	a, (de)		; 3. n. bloco
	ld	c, a
	inc	de
	ld	a, (de)		; 4. n. bloco
	inc	de
	ld	b, a
	pop	af
	ld	d, a
	pop	af		; HL = ponteiro destino
	ld	e, a		; BC DE = 32 bits numero do bloco
	call	LerBloco	; chamar rotina de leitura de dados
	jr	nc, .ok
 IF HWDS = 0
	call	marcaErroCartao	; ocorreu erro na leitura, marcar erro
 ENDIF
;	ld	a, ENRDY	; Not ready
	ld	a, EDISK	; General unknown disk error
	ld	b, 0		; informar que lemos 0 blocos
	ret
.ok:
	xor	a		; tudo OK, informar ao Nextor
	ret

escrita:
	in	a, (PORTSTATUS)	; destructive read
	and	$4		; test if the card is write protected
	jr	z, .ok
	ld	a, EWPROT	; write protect
	ld	b, 0		; 0 blocks were written
	ret
.ok:
	ld	a, (de)		; 1. n. bloco
	push	af
	inc	de
	ld	a, (de)		; 2. n. bloco
	push	af
	inc	de
	ld	a, (de)		; 3. n. bloco
	inc	de
	ld	c, a
	ld	a, (de)		; 4. n. bloco
	inc	de
	ld	b, a
	pop	af
	ld	d, a
	pop	af		; HL = ponteiro destino
	ld	e, a		; BC DE = 32 bits numero do bloco
	call	GravarBloco	; chamar rotina de gravacao de dados
	jr	nc, .ok2
 IF HWDS = 0
	call	marcaErroCartao	; ocorreu erro, marcar nas flags
 ENDIF
	ld	a, EWRERR	; Write error
	ld	b, 0
	ret
.ok2:
	xor	a		; gravacao sem erros!
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

	ld 	a, 2                   	; Otherwise, error code 2: Information not available
	ret

DEV_INFO_BASIC:
	; Fill buffer with basic information:
	; +0: Number of logical units (1)
	; +1: Device flags (0)
	ld 		(hl), 1               ; One logical unit
	inc 	hl
	xor 	a                     ; Set device flags = 0
	ld 		(hl), a
	xor 	a                     ; Return A=0 to indicate success
	ret

DEV_INFO_MANUFACTURER:
	; Return a manufacturer name string in HL
	ld		a, 3			; send the command to return the manufacturer ID
	out 	(PORTCFG), a	; send the command to the config port
	in		a, (PORTSTATUS)	; read the byte with the manufacturer ID
	call	GETMANUFACTURER	; get the name of the manufacturer on the table
							; GETMANUFACTURER returns HL pointing to the string buffer
	ret

DEV_INFO_DEVICENAME:
	; Return a device name string (not implemented yet)
	; it is returning a fixed string "microSD card"
	ld 		de, STR_DEVICENAME   ; DE points to null-terminated device name string
	ex		de, hl				; HL points to the buffer
	ret

DEV_INFO_SERIAL:
	; Return a serial number string
	ld 		a, 4            ; send the command to return the serial number (0x04)
	out 	(PORTCFG), a    ; send the command to the config port
	in		a, (PORTSTATUS)    ; read the byte with the serial number

	call 	HEXTOASCII      ; convert the serial number byte to ASCII
							; Now DE points to the converted ASCII characters

	; Prepare the destination buffer starting with "0x"
	ld		hl, SERIAL_BUFFER ; HL will point to the start of the string buffer
	ld		(hl), '0'
	inc		hl
	ld		(hl), 'x'
	inc		hl

	; Copy the converted ASCII characters from DE to the buffer at HL
	ld		b, 2              ; copy 2 bytes (hexadecimal representation for one byte)
.copy_loop:
	ld		a, (de)
	ld		(hl), a
	inc		hl
	inc		de
	djnz	.copy_loop

	; HL now points just after the concatenated string in SERIAL_BUFFER;

	; we can adjust HL to point to the beginning by reloading the address.
	ld		hl, SERIAL_BUFFER ; HL points to the start of the string buffer as required

	ret

; Define a buffer for the final microSD card serial string
SERIAL_BUFFER:
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
	xor	a
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
;+8 (2): Number of cylinders
;+10 (1): Number of heads
;+11 (1): Number of sectors per track
;
; Number of cylinders, heads and sectors apply to hard disks only.
; For other types of device, these fields must be zero.

LUN_INFO:
	cp	2		; somente 1 dispositivo
	jr	nc, .saicomerro
	dec	b		; somente 1 logical unit
	jr	z, .ok
.saicomerro:
	ld	a, 1		; informar erro
	ret
.ok:
 IF HWDS = 0
	call	checkSWDS
	jr	c, .saicomerro
 ENDIF
	push	hl
	call	calculaBLOCOSoffset	; calcular em IX e HL o offset correto do buffer que armazena total de blocos
	pop	hl		; do cartao dependendo do cartao atual solicitado
	xor	a
	ld	(hl), a		; Informar que o dispositivo eh do tipo block device
	inc	hl
	ld	(hl), a		; tamanho de um bloco = 512 bytes (coloca $00, $02 que � $200 = 512)
	inc	hl
	ld	a, 2
	ld	(hl), a
	inc	hl
	ld	a, (ix)		; copia numero de blocos total
	ld	(hl), a
	inc	hl
	ld	a, (ix+1)
	ld	(hl), a
	inc	hl
	ld	a, (ix+2)
	ld	(hl), a
	inc	hl
	xor	a		; cartoes SD tem total de blocos em 24 bits, mas o Nextor pede numero de
	ld	(hl), a 	; 32 bits, entao coloca 0 no MSB
	inc	hl
	ld	a, 1		; flags: dispositivo R/W removivel
	ld	(hl), a
	inc	hl
	xor	a		; CHS = 0
	ld	(hl), a
	inc	hl
	ld	(hl), a
	inc	hl
	ld	(hl), a
	inc	hl
	xor	a		; informar que dados foram preenchidos
	ret

;=====
;=====  END of DEVICE-BASED specific routines
;=====

;------------------------------------------------
; Rotinas auxiliares
;------------------------------------------------

;------------------------------------------------
; Testa se cartao esta inserido e/ou houve erro
; na ultima vez que foi acessado. Carry indica
; erro
; Destroi AF
;------------------------------------------------

 IF HWDS = 0
checkSWDS:
	ld	a, (WRKAREA.FLAGS)	; testar bit de erro do cartao nas flags
	and	1
	jr	z, .ok
	scf			; indica erro
	ret
.ok:
	xor	a		; zera carry indicando sem erro
	ret

;------------------------------------------------
; Marcar bit de erro nas flags
; Destroi AF, C
;------------------------------------------------
marcaErroCartao:
	ld	a, (WRKAREA.FLAGS)	; marcar erro
	or	1
	ld	(WRKAREA.FLAGS), a
	ret

 ENDIF

;------------------------------------------------
; Calcula offset do buffer na RAM em HL e IX para
; os dados do CID dependendo do cartao atual
; Destroi AF, DE, HL e IX
;------------------------------------------------
calculaCIDoffset:
	ld	hl, WRKAREA.BCID1
	ld	a, (WRKAREA.NUMSD)
	dec	a
	jr	z, .c1
.c1:
	push	hl		
	pop	ix		; vamos colocar HL em IX
	ret

;------------------------------------------------
; Calcula offset do buffer na RAM para os dados
; do total de blocos dependendo do cartao atual
; Offset fica em HL e IX
; Destroi AF, DE, HL e IX
;------------------------------------------------
calculaBLOCOSoffset:
	ld	hl, WRKAREA.BLOCKS1
	ld	a, (WRKAREA.NUMSD)
	dec	a
	jr	z, .c1
	ld	hl, WRKAREA.BLOCKS2
.c1:
	push	hl		
	pop	ix		; Vamos colocar HL em IX
	ret

; ------------------------------------------------
; Setar o tamanho do bloco para 512 se o cartao
; for SDV1
; ------------------------------------------------
mudarTamanhoBlocoPara512:
	ld	a, CMD16
	ld	bc, 0
	ld	de, 512
	jp	SD_SEND_CMD_GET_ERROR

; ------------------------------------------------
; Desabilitar (de-selecionar) todos os cartoes
; Nao destroi registradores
; ------------------------------------------------
desabilitaSDs:
	push	af
	ld	a, $FF
	out	(PORTCFG), a
	pop	af
	ret

; ------------------------------------------------
; Enviar CMD1 para cartao. Carry indica erro
; Destroi AF, BC, DE
; ------------------------------------------------
SD_SEND_CMD1:
	ld	a, CMD1
SD_SEND_CMD_NO_ARGS:
	ld	bc, 0
	ld	d, b
	ld	e, c
SD_SEND_CMD_GET_ERROR:
	call	SD_SEND_CMD
	or	a
	ret	z		; se A=0 nao houve erro, retornar
	; fall throw

; ------------------------------------------------
; Informar erro
; Nao destroi registradores
; ------------------------------------------------
setaErro:
	scf
	jr		desabilitaSDs

; ------------------------------------------------
; Enviar comando em A com 2 bytes de parametros
; em DE e testar retorno BUSY
; Retorna em A a resposta do cartao
; Destroi AF, BC
; ------------------------------------------------
SD_SEND_CMD_2_ARGS_TEST_BUSY:
	ld	bc, 0
	call	SD_SEND_CMD
	ld	b, a
	and	$FE		; testar bit 0 (flag BUSY)
	ld	a, b
	jr	nz, setaErro	; BUSY em 1, informar erro
	ret			; sem erros

; ------------------------------------------------
; Enviar comando em A com 2 bytes de parametros
; em DE e ler resposta do tipo R3 em BC DE
; Retorna em A a resposta do cartao
; Destroi AF, BC, DE, HL
; ------------------------------------------------
SD_SEND_CMD_2_ARGS_GET_R3:
	call	SD_SEND_CMD_2_ARGS_TEST_BUSY
	ret	c
	push	af
	call	WAIT_RESP_NO_FF
	ld	h, a
	call	WAIT_RESP_NO_FF
	ld	l, a
	call	WAIT_RESP_NO_FF
	ld	d, a
	call	WAIT_RESP_NO_FF
	ld	e, a
	ld	b, h
	ld	c, l
	pop	af
	ret

; ------------------------------------------------
; Enviar comando em A com 4 bytes de parametros
; em BC DE e enviar CRC correto se for CMD0 ou 
; CMD8 e aguardar processamento do cartao
; Destroi AF, BC
; ------------------------------------------------
SD_SEND_CMD:
	call	enableSD
	out	(PORTSPI), a
	push	af
	ld	a, b
	nop
	out	(PORTSPI), a
	ld	a, c
	nop
	out	(PORTSPI), a
	ld	a, d
	nop
	out	(PORTSPI), a
	ld	a, e
	nop
	out	(PORTSPI), a
	pop	af
	cp	CMD0
	ld	b, $95		; CRC para CMD0
	jr	z, enviaCRC
	cp	CMD8
	ld	b, $87		; CRC para CMD8
	jr	z, enviaCRC
	ld	b, $FF		; CRC dummy
enviaCRC:
	ld	a, b
	out	(PORTSPI), a
	jr	WAIT_RESP_NO_FF

; ------------------------------------------------
; Esperar que resposta do cartao seja $FE
; Destroi AF, B
; ------------------------------------------------
WAIT_RESP_FE:
	ld	b, 10		; 10 tentativas
.loop:
	push	bc
	call	WAIT_RESP_NO_FF	; esperar resposta diferente de $FF
	pop	bc
	cp	$FE		; resposta � $FE ?
	ret	z		; sim, retornamos com carry=0
	djnz	.loop
	scf			; erro, carry=1
	ret

; ------------------------------------------------
; Esperar que resposta do cartao seja diferente
; de $FF
; Destroi AF, BC
; ------------------------------------------------
WAIT_RESP_NO_FF:
	ld	bc, 100		; 25600 tentativas
.loop:
	in	a, (PORTSPI)
	cp	$FF		; testa $FF
	ret	nz		; sai se nao for $FF
	djnz	.loop
	dec	c
	jr	nz, .loop
	ret

; ------------------------------------------------
; Esperar que resposta do cartao seja diferente
; de $00
; Destroi A, BC
; ------------------------------------------------
WAIT_RESP_NO_00:
	ld	bc, 32768	; 32768 tentativas
.loop:
	in	a, (PORTSPI)
	or	a
	ret	nz		; se resposta for <> $00, sai
	djnz	.loop
	dec	c
	jr	nz, .loop
	scf			; erro
	ret

; ------------------------------------------------
; Ativa (seleciona) cartao atual baixando seu /CS
; Nao destroi registradores
; ------------------------------------------------
enableSD:

	push	af
	in	a, (PORTSPI)	; dummy read
	ld	a, $FE
	out	(PORTCFG), a
	pop	af
	ret


; ------------------------------------------------
; Grava um bloco de 512 bytes no cartao
; HL aponta para o inicio dos dados
; BC e DE contem o numero do bloco (BCDE = 32 bits)
; Destroi AF, BC, DE, HL
; ------------------------------------------------
GravarBloco:
	ld	a, (ix+15)	; verificar se eh SDV1 ou SDV2
	or	a
	call	z, blocoParaByte	; se for SDV1 coverter blocos para bytes
	call	enableSD
	ld	a, (WRKAREA.NUMBLOCKS)	; testar se Nextor quer gravar 1 ou mais blocos
	dec	a
	jp	z, .umBloco	; somente um bloco, gravar usando CMD24

; multiplos blocos
	push	bc
	push	de
	ld	a, CMD55	; Multiplos blocos, mandar ACMD23 com total de blocos
	call	SD_SEND_CMD_NO_ARGS
	ld	bc, 0
	ld	d, c
	ld	a, (WRKAREA.NUMBLOCKS)	; parametro = total de blocos a gravar
	ld	e, a
	ld	a, ACMD23
	call	SD_SEND_CMD_GET_ERROR
	pop	de
	pop	bc
	jp	c, .erro	; erro no ACMD23
	ld	a, CMD25	; comando CMD25 = write multiple blocks
	call	SD_SEND_CMD_GET_ERROR
	jp	c, .erro	; erro
.loop:
	ld	a, $FC		; mandar $FC para indicar que os proximos dados sao
	out	(PORTSPI), a	; dados para gravacao
	ld	bc, PORTSPI
	otir
	otir
	ld	a, $FF		; envia dummy CRC
	out	(PORTSPI), a
	nop
	out	(PORTSPI), a
	call	WAIT_RESP_NO_FF	; esperar cartao
	and	$1F		; testa bits erro
	cp	5
	jr	nz, .erro	; resposta errada, informar erro
	call	WAIT_RESP_NO_00	; esperar cartao
	jr	c, .erro
	ld	a, (WRKAREA.NUMBLOCKS)	; testar se tem mais blocos para gravar
	dec	a
	ld	(WRKAREA.NUMBLOCKS), a
	jp	nz, .loop
	in	a, (PORTSPI)	; acabou os blocos, fazer 2 dummy reads
	nop
	in	a, (PORTSPI)
	ld	a, $FD		; enviar $FD para informar ao cartao que acabou os dados
	out	(PORTSPI), a
	nop
	nop
	in	a, (PORTSPI)	; dummy reads
	nop
	in	a, (PORTSPI)
	call	WAIT_RESP_NO_00	; esperar cartao
	jp	.fim		; CMD25 concluido, sair informando nenhum erro

.umBloco:
	ld	a, CMD24	; gravar somente um bloco com comando CMD24 = Write Single Block
	call	SD_SEND_CMD_GET_ERROR
	jr	nc, .ok
.erro:
	scf			; informar erro
	jp	terminaLeituraEscritaBloco
.ok:
	ld	a, $FE		; mandar $FE para indicar que vamos mandar dados para gravacao
	out	(PORTSPI), a
	ld	bc, PORTSPI
	otir
	otir
	ld	a, $FF		; envia dummy CRC
	out	(PORTSPI), a
	nop
	out	(PORTSPI), a
	call	WAIT_RESP_NO_FF	; esperar cartao
	and	$1F		; testa bits erro
	cp	5
	jp	nz, .erro	; resposta errada, informar erro
.esp:
	call	WAIT_RESP_NO_FF	; esperar cartao
	or	a
	jr	z, .esp
.fim:
	xor	a		; zera carry e informa nenhum erro
terminaLeituraEscritaBloco:
	push	af
	call	desabilitaSDs	; desabilitar todos os cartoes
	pop	af
	ret

; ------------------------------------------------
; Ler um bloco de 512 bytes do cartao
; HL aponta para o inicio dos dados
; BC e DE contem o numero do bloco (BCDE = 32 bits)
; Destroi AF, BC, DE, HL
; ------------------------------------------------
LerBloco:
	ld	a, (ix+15)	; verificar se eh SDV1 ou SDV2
	or	a
	call	z, blocoParaByte	; se for SDV1 coverter blocos para bytes
	call	enableSD
	ld	a, (WRKAREA.NUMBLOCKS)	; testar se Nextor quer ler um ou mais blocos
	dec	a
	jp	z, .umBloco	; somente um bloco, pular

; multiplos blocos
	ld	a, CMD18	; ler multiplos blocos com CMD18 = Read Multiple Blocks
	call	SD_SEND_CMD_GET_ERROR
	jr	c, .erro
.loop:
	call	WAIT_RESP_FE
	jr	c, .erro
	ld	bc, PORTSPI
	inir
	inir
	nop
	in	a, (PORTSPI)	; descarta CRC
	nop
	in	a, (PORTSPI)
	ld	a, (WRKAREA.NUMBLOCKS)	; testar se tem mais blocos para ler
	dec	a
	ld	(WRKAREA.NUMBLOCKS), a
	jp	nz, .loop
	ld	a, CMD12	; acabou os blocos, mandar CMD12 para cancelar leitura
	call	SD_SEND_CMD_NO_ARGS
	jr	.fim

.umBloco:
	ld	a, CMD17	; ler somente um bloco com CMD17 = Read Single Block
	call	SD_SEND_CMD_GET_ERROR
	jr	nc, .ok
.erro:
	scf
	jp	terminaLeituraEscritaBloco
.ok:
	call	WAIT_RESP_FE
	jr	c, .erro
	ld	bc, PORTSPI
	inir
	inir
	nop
	in	a, (PORTSPI)	; descarta CRC
	nop
	in	a, (PORTSPI)
.fim:
	xor	a		; zera carry para informar leitura sem erros
	jp	terminaLeituraEscritaBloco

; ------------------------------------------------
; Converte blocos para bytes. Na pratica faz
; BC DE = (BC DE) * 512
; ------------------------------------------------
blocoParaByte:
	ld	b, c
	ld	c, d
	ld	d, e
	ld	e, 0
	sla	d
	rl	c
	rl	b
	ret

; ------------------------------------------------
; Funcoes utilitarias
; ------------------------------------------------

; ------------------------------------------------
; Imprime string na tela apontada por DE
; Destroi todos os registradores
; ------------------------------------------------
PRINTSTRING:
	ld	a, (de)
	or	a
	ret	z
	call	BIOS_CHPUT
	inc	de
	jr	PRINTSTRING


; ------------------------------------------------
; Converte o byte em A para string em decimal no
; buffer apontado por DE
; Destroi AF, BC, HL, DE
; ------------------------------------------------
DecToAscii:
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

 IF HWDS = 0
FLAGS		ds 1	; Flags for soft-diskchange
 ENDIF
; ENDS



DRV_END:

	ds	3ED0h-(DRV_END-DRV_START)
	end

