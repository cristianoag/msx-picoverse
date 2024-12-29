;--------------------------------------------------------
; File Created by SDCC : free open source ANSI-C Compiler
; Version 3.9.0 #11195 (MINGW64)
;--------------------------------------------------------
	.module menu
	.optsdcc -mz80
	
;--------------------------------------------------------
; Public variables in this module
;--------------------------------------------------------
	.globl _main
	.globl _navigateMenu
	.globl _displayMenu
	.globl _printf
	.globl _WaitKey
	.globl _Screen
	.globl _Cls
	.globl _Locate
	.globl _totalFiles
	.globl _currentIndex
	.globl _currentPage
	.globl _mapp
	.globl _game
	.globl _putchar
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_game::
	.ds 40
_mapp::
	.ds 40
_currentPage::
	.ds 2
_currentIndex::
	.ds 2
_totalFiles::
	.ds 2
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _INITIALIZED
;--------------------------------------------------------
; absolute external ram data
;--------------------------------------------------------
	.area _DABS (ABS)
;--------------------------------------------------------
; global & static initialisations
;--------------------------------------------------------
	.area _HOME
	.area _GSINIT
	.area _GSFINAL
	.area _GSINIT
;--------------------------------------------------------
; Home
;--------------------------------------------------------
	.area _HOME
	.area _HOME
;--------------------------------------------------------
; code
;--------------------------------------------------------
	.area _CODE
;src\menu.c:20: int putchar (int character)
;	---------------------------------
; Function putchar
; ---------------------------------
_putchar::
;src\menu.c:32: __endasm;
	ld	hl, #2
	add	hl, sp ;Bypass the return address of the function
	ld	a, (hl)
	ld	iy,(#0xfcc1 -1) ;BIOS slot in iyh
	push	ix
	ld	ix,#0x00a2 ;address of BIOS routine
	call	0x001c ;interslot call
	pop	ix
;src\menu.c:34: return character;
	pop	bc
	pop	hl
	push	hl
	push	bc
;src\menu.c:35: }
	ret
_Done_Version_tag:
	.ascii "Made with FUSION-C 1.3 R21010 (c)EBSOFT:2021"
	.db 0x00
;src\menu.c:37: void displayMenu() {
;	---------------------------------
; Function displayMenu
; ---------------------------------
_displayMenu::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl, #-6
	add	hl, sp
	ld	sp, hl
;src\menu.c:39: Screen(0);
	xor	a, a
	push	af
	inc	sp
	call	_Screen
	inc	sp
;src\menu.c:40: Cls();
	call	_Cls
;src\menu.c:42: int startFile = currentPage * FILES_PER_PAGE;
	ld	bc, (_currentPage)
	ld	l, c
	ld	h, b
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, bc
	add	hl, hl
	add	hl, bc
	ex	(sp), hl
;src\menu.c:46: printf("MSX PICOVERSE 2040     [MultiROM v1.0]");
	ld	hl, #___str_1
	push	hl
	call	_printf
;src\menu.c:47: Locate(0, 1);
	ld	h,#0x01
	ex	(sp),hl
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src\menu.c:48: printf("--------------------------------------");
	ld	hl, #___str_2
	ex	(sp),hl
	call	_printf
	pop	af
;src\menu.c:49: Locate(0, currentIndex + 2);
	ld	a,(#_currentIndex + 0)
	add	a, #0x02
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src\menu.c:50: printf(">");
	ld	hl, #___str_3
	ex	(sp),hl
	call	_printf
	pop	af
;src\menu.c:51: int xi = 0;
	ld	-4 (ix), #0x00
	ld	-3 (ix), #0x00
;src\menu.c:52: for (int i = 0; (i < FILES_PER_PAGE) && (xi<totalFiles) ; i++) 
	ld	de, #0x0000
00104$:
	ld	a, e
	sub	a, #0x13
	ld	a, d
	rla
	ccf
	rra
	sbc	a, #0x80
	jr	NC,00101$
	ld	hl, #_totalFiles
	ld	a, -4 (ix)
	sub	a, (hl)
	ld	a, -3 (ix)
	inc	hl
	sbc	a, (hl)
	jp	PO, 00124$
	xor	a, #0x80
00124$:
	jp	P, 00101$
;src\menu.c:54: Locate(1, 2 + i); // Position on the screen, starting at line 2
	ld	a, e
	add	a, #0x02
	push	de
	ld	d,a
	ld	e,#0x01
	push	de
	call	_Locate
	pop	af
	pop	de
;src\menu.c:55: xi = i+startFile;
	pop	hl
	push	hl
	add	hl, de
;src\menu.c:56: printf("%-30s %-8s",game[xi], mapp[xi]);  // Print each file name
	ld	-4 (ix), l
	ld	-3 (ix), h
	ld	c, l
	ld	b, h
	sla	c
	rl	b
	ld	hl, #_mapp
	add	hl, bc
	ld	a, (hl)
	ld	-2 (ix), a
	inc	hl
	ld	a, (hl)
	ld	-1 (ix), a
	ld	hl, #_game
	add	hl, bc
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	push	de
	ld	l, -2 (ix)
	ld	h, -1 (ix)
	push	hl
	push	bc
	ld	hl, #___str_4
	push	hl
	call	_printf
	ld	hl, #6
	add	hl, sp
	ld	sp, hl
	pop	de
;src\menu.c:52: for (int i = 0; (i < FILES_PER_PAGE) && (xi<totalFiles) ; i++) 
	inc	de
	jr	00104$
00101$:
;src\menu.c:59: Locate(0, 21);
	ld	a, #0x15
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src\menu.c:60: printf("--------------------------------------");
	ld	hl, #___str_2
	ex	(sp),hl
	call	_printf
;src\menu.c:61: Locate(0, 22);
	ld	h,#0x16
	ex	(sp),hl
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
	pop	af
;src\menu.c:62: printf("Page %02d/02       [F1 Help] [F2 Config]",currentPage + 1);
	ld	hl, (_currentPage)
	inc	hl
	ld	bc, #___str_5+0
	push	hl
	push	bc
	call	_printf
	pop	af
	pop	af
;src\menu.c:63: Locate(0, currentIndex + 2);
	ld	a,(#_currentIndex + 0)
	add	a, #0x02
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src\menu.c:64: }
	ld	sp,ix
	pop	ix
	ret
___str_1:
	.ascii "MSX PICOVERSE 2040     [MultiROM v1.0]"
	.db 0x00
___str_2:
	.ascii "--------------------------------------"
	.db 0x00
___str_3:
	.ascii ">"
	.db 0x00
___str_4:
	.ascii "%-30s %-8s"
	.db 0x00
___str_5:
	.ascii "Page %02d/02       [F1 Help] [F2 Config]"
	.db 0x00
;src\menu.c:66: void navigateMenu() 
;	---------------------------------
; Function navigateMenu
; ---------------------------------
_navigateMenu::
	push	ix
	push	af
;src\menu.c:70: while (1) 
00119$:
;src\menu.c:72: key = WaitKey();
	call	_WaitKey
	ld	e, l
;src\menu.c:73: Locate(0, 23);
	push	de
	ld	a, #0x17
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
	pop	af
	pop	de
;src\menu.c:74: printf("Key: %3d", key);
	ld	c, e
	ld	b, #0x00
	push	de
	push	bc
	ld	hl, #___str_6
	push	hl
	call	_printf
	pop	af
	pop	af
	pop	de
;src\menu.c:75: Locate(0, currentIndex + 2);
	ld	a,(#_currentIndex + 0)
	add	a, #0x02
	push	de
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
	ld	hl, #___str_7
	ex	(sp),hl
	call	_printf
	pop	af
	pop	de
;src\menu.c:82: currentPage--;
	ld	bc, (_currentPage)
	dec	bc
;src\menu.c:77: switch (key) 
	ld	a, e
	sub	a, #0x1e
	jr	Z,00101$
;src\menu.c:89: if (currentIndex >= (currentPage + 1) * FILES_PER_PAGE) {
	ld	hl, (_currentPage)
	inc	hl
	ex	(sp), hl
;src\menu.c:77: switch (key) 
	ld	a,e
	cp	a,#0x1f
	jr	Z,00106$
	cp	a,#0x4b
	jp	Z,00114$
	sub	a, #0x4d
	jp	Z,00111$
	jp	00117$
;src\menu.c:79: case 30: // Up arrow
00101$:
;src\menu.c:80: if (currentIndex > 0) currentIndex--;
	xor	a, a
	ld	iy, #_currentIndex
	cp	a, 0 (iy)
	sbc	a, 1 (iy)
	jp	PO, 00182$
	xor	a, #0x80
00182$:
	jp	P, 00103$
	ld	hl, (_currentIndex)
	dec	hl
	ld	(_currentIndex), hl
00103$:
;src\menu.c:81: if (currentIndex < currentPage * FILES_PER_PAGE) {
	ld	de, (_currentPage)
	ld	l, e
	ld	h, d
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, de
	add	hl, hl
	add	hl, de
	ex	de, hl
	ld	iy, #_currentIndex
	ld	a, 0 (iy)
	sub	a, e
	ld	a, 1 (iy)
	sbc	a, d
	jp	PO, 00183$
	xor	a, #0x80
00183$:
	jp	P, 00105$
;src\menu.c:82: currentPage--;
	ld	(_currentPage), bc
00105$:
;src\menu.c:84: Locate(28, 23);
	ld	de, #0x171c
	push	de
	call	_Locate
	pop	af
;src\menu.c:85: printf("Index: %2d", currentIndex);
	ld	hl, (_currentIndex)
	push	hl
	ld	hl, #___str_8
	push	hl
	call	_printf
	pop	af
	pop	af
;src\menu.c:86: break;
	jp	00117$
;src\menu.c:87: case 31: // Down arrow
00106$:
;src\menu.c:88: if (currentIndex < FILES_PER_PAGE - 1) currentIndex++;
	ld	iy, #_currentIndex
	ld	a, 0 (iy)
	sub	a, #0x12
	ld	a, 1 (iy)
	rla
	ccf
	rra
	sbc	a, #0x80
	jr	NC,00108$
	ld	hl, (_currentIndex)
	inc	hl
	ld	(_currentIndex), hl
00108$:
;src\menu.c:89: if (currentIndex >= (currentPage + 1) * FILES_PER_PAGE) {
	pop	bc
	push	bc
	ld	l, c
	ld	h, b
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, bc
	add	hl, hl
	add	hl, bc
	ex	de, hl
	ld	iy, #_currentIndex
	ld	a, 0 (iy)
	sub	a, e
	ld	a, 1 (iy)
	sbc	a, d
	jp	PO, 00184$
	xor	a, #0x80
00184$:
	jp	M, 00110$
;src\menu.c:90: currentPage++;
	pop	hl
	push	hl
	ld	(_currentPage), hl
00110$:
;src\menu.c:92: Locate(28, 23);
	ld	de, #0x171c
	push	de
	call	_Locate
	pop	af
;src\menu.c:93: printf("Index: %2d", currentIndex);
	ld	hl, (_currentIndex)
	push	hl
	ld	hl, #___str_8
	push	hl
	call	_printf
	pop	af
	pop	af
;src\menu.c:94: break;
	jr	00117$
;src\menu.c:95: case 0x4D: // Right arrow
00111$:
;src\menu.c:96: if (currentPage < (totalFiles - 1) / FILES_PER_PAGE) {
	ld	bc, (_totalFiles)
	dec	bc
	ld	hl, #0x0013
	push	hl
	push	bc
	call	__divsint
	pop	af
	pop	af
	ld	c, l
	ld	b, h
	ld	iy, #_currentPage
	ld	a, 0 (iy)
	sub	a, c
	ld	a, 1 (iy)
	sbc	a, b
	jp	PO, 00185$
	xor	a, #0x80
00185$:
	jp	P, 00117$
;src\menu.c:97: currentPage++;
	pop	hl
	push	hl
	ld	(_currentPage), hl
;src\menu.c:98: currentIndex = currentPage * FILES_PER_PAGE;
	ld	bc, (_currentPage)
	ld	l, c
	ld	h, b
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, bc
	add	hl, hl
	add	hl, bc
	ld	(_currentIndex), hl
;src\menu.c:100: break;
	jr	00117$
;src\menu.c:101: case 0x4B: // Left arrow
00114$:
;src\menu.c:102: if (currentPage > 0) {
	xor	a, a
	ld	iy, #_currentPage
	cp	a, 0 (iy)
	sbc	a, 1 (iy)
	jp	PO, 00186$
	xor	a, #0x80
00186$:
	jp	P, 00117$
;src\menu.c:103: currentPage--;
	ld	(_currentPage), bc
;src\menu.c:104: currentIndex = currentPage * FILES_PER_PAGE;
	ld	bc, (_currentPage)
	ld	l, c
	ld	h, b
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, bc
	add	hl, hl
	add	hl, bc
	ld	(_currentIndex), hl
;src\menu.c:107: }
00117$:
;src\menu.c:108: Locate(0, currentIndex + 2);
	ld	a,(#_currentIndex + 0)
	add	a, #0x02
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src\menu.c:109: printf(">");
	ld	hl, #___str_9
	ex	(sp),hl
	call	_printf
	pop	af
;src\menu.c:110: Locate(0, currentIndex + 2);
	ld	a,(#_currentIndex + 0)
	add	a, #0x02
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
	pop	af
;src\menu.c:112: }
	jp	00119$
___str_6:
	.ascii "Key: %3d"
	.db 0x00
___str_7:
	.ascii " "
	.db 0x00
___str_8:
	.ascii "Index: %2d"
	.db 0x00
___str_9:
	.ascii ">"
	.db 0x00
;src\menu.c:117: void main() {
;	---------------------------------
; Function main
; ---------------------------------
_main::
;src\menu.c:118: currentPage = 0;
	ld	hl, #0x0000
	ld	(_currentPage), hl
;src\menu.c:119: currentIndex = 0;
	ld	l, #0x00
	ld	(_currentIndex), hl
;src\menu.c:120: totalFiles = 20; // Total of files stored on the flash
	ld	l, #0x14
	ld	(_totalFiles), hl
;src\menu.c:122: game[0] =  "Metal Gear          ";    mapp[0] = "Konami";
	ld	hl, #___str_10
	ld	(_game), hl
	ld	hl, #___str_11
	ld	(_mapp), hl
;src\menu.c:123: game[1] =  "Nemesis             ";    mapp[1] = "Konami";
	ld	hl, #___str_12
	ld	((_game + 0x0002)), hl
	ld	hl, #___str_11
	ld	((_mapp + 0x0002)), hl
;src\menu.c:124: game[2] =  "Contra              ";    mapp[2] = "Konami";
	ld	hl, #___str_13
	ld	((_game + 0x0004)), hl
	ld	hl, #___str_11
	ld	((_mapp + 0x0004)), hl
;src\menu.c:125: game[3] =  "Castlevania         ";    mapp[3] = "Konami";
	ld	hl, #___str_14
	ld	((_game + 0x0006)), hl
	ld	hl, #___str_11
	ld	((_mapp + 0x0006)), hl
;src\menu.c:126: game[4] =  "Kings Valley II     ";    mapp[4] = "Konami";
	ld	hl, #___str_15
	ld	((_game + 0x0008)), hl
	ld	hl, #___str_11
	ld	((_mapp + 0x0008)), hl
;src\menu.c:127: game[5] =  "Vampire Killer      ";    mapp[5] = "Konami";
	ld	hl, #___str_16
	ld	((_game + 0x000a)), hl
	ld	hl, #___str_11
	ld	((_mapp + 0x000a)), hl
;src\menu.c:128: game[6] =  "Snatcher            ";    mapp[6] = "Konami";
	ld	hl, #___str_17
	ld	((_game + 0x000c)), hl
	ld	hl, #___str_11
	ld	((_mapp + 0x000c)), hl
;src\menu.c:129: game[7] =  "Galaga              ";    mapp[7] = "32KB";
	ld	hl, #___str_18
	ld	((_game + 0x000e)), hl
	ld	hl, #___str_19
	ld	((_mapp + 0x000e)), hl
;src\menu.c:130: game[8] =  "Zaxxon              ";    mapp[8] = "32KB";
	ld	hl, #___str_20
	ld	((_game + 0x0010)), hl
	ld	hl, #___str_19
	ld	((_mapp + 0x0010)), hl
;src\menu.c:131: game[9] =  "Salamander          ";    mapp[9] = "Konami";
	ld	hl, #___str_21
	ld	((_game + 0x0012)), hl
	ld	hl, #___str_11
	ld	((_mapp + 0x0012)), hl
;src\menu.c:132: game[10] = "Parodius            ";    mapp[10] = "Konami";
	ld	hl, #___str_22
	ld	((_game + 0x0014)), hl
	ld	hl, #___str_11
	ld	((_mapp + 0x0014)), hl
;src\menu.c:133: game[11] = "Knightmare          ";    mapp[11] = "32KB";
	ld	hl, #___str_23
	ld	((_game + 0x0016)), hl
	ld	hl, #___str_19
	ld	((_mapp + 0x0016)), hl
;src\menu.c:134: game[12] = "Pippols             ";    mapp[12] = "16KB";
	ld	hl, #___str_24
	ld	((_game + 0x0018)), hl
	ld	hl, #___str_25
	ld	((_mapp + 0x0018)), hl
;src\menu.c:135: game[13] = "The Maze of Galious ";    mapp[13] = "Konami";
	ld	hl, #___str_26
	ld	((_game + 0x001a)), hl
	ld	hl, #___str_11
	ld	((_mapp + 0x001a)), hl
;src\menu.c:136: game[14] = "Penguin Adventure   ";    mapp[14] = "32KB";
	ld	hl, #___str_27
	ld	((_game + 0x001c)), hl
	ld	hl, #___str_19
	ld	((_mapp + 0x001c)), hl
;src\menu.c:137: game[15] = "Space Manbow        ";    mapp[15] = "Konami";
	ld	hl, #___str_28
	ld	((_game + 0x001e)), hl
	ld	hl, #___str_11
	ld	((_mapp + 0x001e)), hl
;src\menu.c:138: game[16] = "Gradius 2           ";    mapp[16] = "Konami";
	ld	hl, #___str_29
	ld	((_game + 0x0020)), hl
	ld	hl, #___str_11
	ld	((_mapp + 0x0020)), hl
;src\menu.c:139: game[17] = "TwinBee             ";    mapp[17] = "16KB";
	ld	hl, #___str_30
	ld	((_game + 0x0022)), hl
	ld	hl, #___str_25
	ld	((_mapp + 0x0022)), hl
;src\menu.c:140: game[18] = "Zanac               ";    mapp[18] = "32KB";
	ld	hl, #___str_31
	ld	((_game + 0x0024)), hl
	ld	hl, #___str_19
	ld	((_mapp + 0x0024)), hl
;src\menu.c:141: game[19] = "H.E.R.O.            ";    mapp[19] = "16KB";
	ld	hl, #___str_32
	ld	((_game + 0x0026)), hl
	ld	hl, #___str_25
	ld	((_mapp + 0x0026)), hl
;src\menu.c:144: displayMenu();
	call	_displayMenu
;src\menu.c:145: navigateMenu();
;src\menu.c:146: }
	jp  _navigateMenu
___str_10:
	.ascii "Metal Gear          "
	.db 0x00
___str_11:
	.ascii "Konami"
	.db 0x00
___str_12:
	.ascii "Nemesis             "
	.db 0x00
___str_13:
	.ascii "Contra              "
	.db 0x00
___str_14:
	.ascii "Castlevania         "
	.db 0x00
___str_15:
	.ascii "Kings Valley II     "
	.db 0x00
___str_16:
	.ascii "Vampire Killer      "
	.db 0x00
___str_17:
	.ascii "Snatcher            "
	.db 0x00
___str_18:
	.ascii "Galaga              "
	.db 0x00
___str_19:
	.ascii "32KB"
	.db 0x00
___str_20:
	.ascii "Zaxxon              "
	.db 0x00
___str_21:
	.ascii "Salamander          "
	.db 0x00
___str_22:
	.ascii "Parodius            "
	.db 0x00
___str_23:
	.ascii "Knightmare          "
	.db 0x00
___str_24:
	.ascii "Pippols             "
	.db 0x00
___str_25:
	.ascii "16KB"
	.db 0x00
___str_26:
	.ascii "The Maze of Galious "
	.db 0x00
___str_27:
	.ascii "Penguin Adventure   "
	.db 0x00
___str_28:
	.ascii "Space Manbow        "
	.db 0x00
___str_29:
	.ascii "Gradius 2           "
	.db 0x00
___str_30:
	.ascii "TwinBee             "
	.db 0x00
___str_31:
	.ascii "Zanac               "
	.db 0x00
___str_32:
	.ascii "H.E.R.O.            "
	.db 0x00
	.area _CODE
	.area _INITIALIZER
	.area _CABS (ABS)
