;----------------------------------------------------------
; msxromcrt0.s - by S0urceror - 2022
; changed by The Retro Hacker for the PicoVerse 2350 project
; Copyright (C) 2022 S0urceror
; Copyright (C) 2025 The Retro Hacker
;
; Template for Nextor drivers for MSX to be used with sdcc
;----------------------------------------------------------

	.globl _interrupt
	.globl _get_workarea_size,_init_driver
	.globl _get_nr_drives_boottime,_get_drive_config
	.globl _get_lun_info
	.globl _read_or_write_sector
	.globl _get_device_status
	.globl _get_device_info

; settings
GLOBALS_INITIALIZER = 0 	; we have global vars to initialize?
BIOS_PROCNM  .equ 0xfd89
;-----------------------------------------------------------------------------
;
; Driver configuration constants
;
;Driver version
	VER_MAIN	.equ	1
	VER_SEC		.equ	0
	VER_REV		.equ	0

.if GLOBALS_INITIALIZER
	.globl  l__INITIALIZER
    .globl  s__INITIALIZED
    .globl  s__INITIALIZER
.endif

;   ====================================
;   ========== HEADER SEGMENT ==========
;   ====================================
	.area	_HEADER (ABS)
  	.org	#0x4000
	.rept 	0x100
		.byte 0x00
	.endm
	
DRV_START:
;-----------------------------------------------------------------------------
;
; Driver signature
;
	.ascii	"NEXTOR_DRIVER"
	.db		#0

;-----------------------------------------------------------------------------
;
; Driver flags:
;    bit 0: 0 for drive-based, 1 for device-based
;    bit 2: 1 if the driver implements the DRV_CONFIG routine
;             (used by Nextor from v2.0.5)

	.db		#0b00000101

;-----------------------------------------------------------------------------
;
; Reserved byte
;
	.db		#0


;-----------------------------------------------------------------------------
;
; Driver name
;
DRV_NAME:
	.ascii	"PicoVerse Nextor Driver"
	.rept 32-(.-DRV_NAME)
		.byte " "
	.endm

;-----------------------------------------------------------------------------
;
; Jump table for the driver public routines
;

	; These routines are mandatory for all drivers
    ; (but probably you need to implement only DRV_INIT)
	jp	_interrupt
	jp	DRV_VERSION
	jp	DRV_INIT
	jp	DRV_BASSTAT		; BASIC's CALL instruction NOT expanded
	jp	DRV_BASDEV
    jp  DRV_EXTBIO
    jp  DRV_DIRECT0
    jp  DRV_DIRECT1
    jp  DRV_DIRECT2
    jp  DRV_DIRECT3
    jp  DRV_DIRECT4
	jp	DRV_CONFIG
	.ds	12

	; These routines are mandatory for device-based drivers
	jp	DEV_RW
	jp	DEV_INFO
	jp	DEV_STATUS
	jp	LUN_INFO

;=====
;=====  END of data that must be at fixed addresses
;=====

DRV_INIT:
    ;-----------------------------------------------------------------------------
    ;
    ; Driver initialization routine, it is called twice:
    ;

    ; first or second try?
    and a
	jr z, __init_1st
	jr nz, __init_2nd
__init_1st:
	push bc
    call _get_workarea_size
	pop bc
	xor a
	; HL has workarea size, return 0 drives, no interrupt
	ret
__init_2nd:	
	push bc
    call _init_driver
	pop bc
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
	ld	a,#VER_MAIN
	ld	b,#VER_SEC
	ld	c,#VER_REV
	ret

DRV_CONFIG:
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
	cp #1
    jr z, __option1
	cp #2
	jr z, __option2
__option_unknown:	
	ld a, #1
	ret
__option1:
	push bc
	call _get_nr_drives_boottime
	pop bc
	ld b, l	; return C-language return value in L via B
	xor a 	; indicate success
	ret
__option2:
	push bc
    call _get_drive_config
	pop bc
	push hl
	pop bc  ; return C-language return value in HL via BC
	xor a 	; indicate success
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

;=====
;=====  BEGIN of DEVICE-BASED specific routines
;=====


DEV_RW:
	push hl
	push de
	push bc
	push af
	call _read_or_write_sector
	pop af
	pop af
	pop af
	pop af
	ld a,l
;	and a ; clear carry when okay
;	ret z
;	scf ; set carry on error, not according to manual but expected when reading in page 1
	ret

DEV_INFO:
	push hl
	ld c, b
	ld b, a
	push bc
	call _get_device_info
	pop af
	pop af
	ld a, l
	ret

DEV_STATUS:
	ld c, b
	ld b, a
	push bc
	call _get_device_status
	pop af
	ld a, l
	ret

LUN_INFO:
	push hl
	ld c, b
	ld b, a
	push bc
	call _get_lun_info
	pop af
	pop af
	ld a, l
	ret

;=====
;=====  END of DEVICE-BASED specific routines
;=====


;----------------------------------------------------------
;	Segments order
;----------------------------------------------------------
	.area _CODE
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _INITIALIZER
	.area _ROMDATA
	.area _DATA
	.area _INITIALIZED
	.area _HEAP

;   ==================================
;   ========== HOME SEGMENT ==========
;   ==================================
	.area _HOME


;   =====================================
;   ========== GSINIT SEGMENTS ==========
;   =====================================
	.area	_GSINIT

	
;   ======================================
;   ========== ROM_DATA SEGMENT ==========
;   ======================================
	.area	_ROMDATA

	
;   ==================================
;   ========== DATA SEGMENT ==========
;   ==================================
	.area	_DATA


;   ==================================
;   ========== HEAP SEGMENT ==========
;   ==================================
	.area	_HEAP




