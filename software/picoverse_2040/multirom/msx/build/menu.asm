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
	.globl _printf
	.globl _WaitKey
	.globl _Screen
	.globl _Cls
	.globl _Locate
	.globl _PrintChar
	.globl _totalFiles
	.globl _currentIndex
	.globl _totalPages
	.globl _currentPage
	.globl _size
	.globl _offset
	.globl _mapp
	.globl _game
	.globl _putchar
	.globl _mapper_description
	.globl _displayMenu
	.globl _helpMenu
	.globl _navigateMenu
;--------------------------------------------------------
; special function registers
;--------------------------------------------------------
;--------------------------------------------------------
; ram data
;--------------------------------------------------------
	.area _DATA
_game::
	.ds 512
_mapp::
	.ds 512
_offset::
	.ds 1024
_size::
	.ds 1024
_currentPage::
	.ds 2
_totalPages::
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
;src/menu.c:34: int putchar (int character)
;	---------------------------------
; Function putchar
; ---------------------------------
_putchar::
;src/menu.c:46: __endasm;
	ld	hl, #2
	add	hl, sp ;Bypass the return address of the function
	ld	a, (hl)
	ld	iy,(#0xfcc1 -1) ;BIOS slot in iyh
	push	ix
	ld	ix,#0x00a2 ;address of BIOS routine
	call	0x001c ;interslot call
	pop	ix
;src/menu.c:48: return character;
	pop	bc
	pop	hl
	push	hl
	push	bc
;src/menu.c:49: }
	ret
_Done_Version_tag:
	.ascii "Made with FUSION-C 1.3 R21010 (c)EBSOFT:2021"
	.db 0x00
;src/menu.c:53: char* mapper_description(int number) {
;	---------------------------------
; Function mapper_description
; ---------------------------------
_mapper_description::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl, #-8
	add	hl, sp
	ld	sp, hl
;src/menu.c:55: const char *descriptions[] = {"16KB", "32KB", "KONAMI", "LINEAR0"};
	ld	hl, #0
	add	hl, sp
	ex	de, hl
	ld	l, e
	ld	h, d
	ld	(hl), #<(___str_1)
	inc	hl
	ld	(hl), #>(___str_1)
	ld	l, e
	ld	h, d
	inc	hl
	inc	hl
	ld	bc, #___str_2+0
	ld	(hl), c
	inc	hl
	ld	(hl), b
	ld	hl, #0x0004
	add	hl, de
	ld	bc, #___str_3+0
	ld	(hl), c
	inc	hl
	ld	(hl), b
	ld	hl, #0x0006
	add	hl, de
	ld	bc, #___str_4+0
	ld	(hl), c
	inc	hl
	ld	(hl), b
;src/menu.c:56: return descriptions[number - 1];
	ld	a, 4 (ix)
	dec	a
	ld	l, a
	rla
	sbc	a, a
	ld	h, a
	add	hl, hl
	add	hl, de
	ld	c, (hl)
	inc	hl
	ld	h, (hl)
	ld	l, c
;src/menu.c:57: }
	ld	sp, ix
	pop	ix
	ret
___str_1:
	.ascii "16KB"
	.db 0x00
___str_2:
	.ascii "32KB"
	.db 0x00
___str_3:
	.ascii "KONAMI"
	.db 0x00
___str_4:
	.ascii "LINEAR0"
	.db 0x00
;src/menu.c:60: void displayMenu() {
;	---------------------------------
; Function displayMenu
; ---------------------------------
_displayMenu::
	push	ix
	ld	ix,#0
	add	ix,sp
	ld	hl, #-10
	add	hl, sp
	ld	sp, hl
;src/menu.c:62: Screen(0); // Set the screen mode
	xor	a, a
	push	af
	inc	sp
	call	_Screen
	inc	sp
;src/menu.c:63: Cls(); // Clear the screen
	call	_Cls
;src/menu.c:66: printf("MSX PICOVERSE 2040     [MultiROM v1.0]");
	ld	hl, #___str_5
	push	hl
	call	_printf
;src/menu.c:67: Locate(0, 1);
	ld	h,#0x01
	ex	(sp),hl
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src/menu.c:68: printf("--------------------------------------");
	ld	hl, #___str_6
	ex	(sp),hl
	call	_printf
;src/menu.c:69: int xi = currentIndex%(FILES_PER_PAGE-1); // Calculate the index of the file to start displaying
	ld	hl, #0x0012
	ex	(sp),hl
	ld	hl, (_currentIndex)
	push	hl
	call	__modsint
	pop	af
	pop	af
	inc	sp
	inc	sp
	push	hl
;src/menu.c:70: for (int i = 0; (i < FILES_PER_PAGE) && (xi<totalFiles-1) ; i++)  // Loop through the files
	ld	-2 (ix), #0x00
	ld	-1 (ix), #0x00
00104$:
	ld	a, -2 (ix)
	sub	a, #0x13
	ld	a, -1 (ix)
	rla
	ccf
	rra
	sbc	a, #0x80
	jp	NC, 00101$
	ld	bc, (_totalFiles)
	dec	bc
	ld	a, -10 (ix)
	sub	a, c
	ld	a, -9 (ix)
	sbc	a, b
	jp	PO, 00124$
	xor	a, #0x80
00124$:
	jp	P, 00101$
;src/menu.c:72: Locate(1, 2 + i); // Position on the screen, starting at line 2
	ld	a, -2 (ix)
	add	a, #0x02
	ld	d,a
	ld	e,#0x01
	push	de
	call	_Locate
	pop	af
;src/menu.c:73: xi = i+((currentPage-1)*FILES_PER_PAGE); // Calculate the index of the file to display
	ld	bc, (_currentPage)
	dec	bc
	ld	l, c
	ld	h, b
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, bc
	add	hl, hl
	add	hl, bc
	ld	e, -2 (ix)
	ld	d, -1 (ix)
	add	hl, de
	inc	sp
	inc	sp
;src/menu.c:74: printf("%-22s %4luKB %-8s",game[xi], size[xi]/1024, mapper_description(mapp[xi]));  // Print each file name, size and mapper
	ex	de,hl
	push	de
	sla	e
	rl	d
	ld	hl, #_mapp
	add	hl, de
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	push	de
	push	bc
	call	_mapper_description
	pop	af
	ld	-8 (ix), l
	ld	-7 (ix), h
	pop	de
	pop	hl
	push	hl
	add	hl, hl
	add	hl, hl
	ld	bc, #_size
	add	hl, bc
	inc	hl
	ld	b, (hl)
	inc	hl
	inc	hl
	ld	a, (hl)
	dec	hl
	ld	l, (hl)
	ld	h, a
	ld	-6 (ix), b
	ld	-5 (ix), l
	ld	-4 (ix), h
	ld	-3 (ix), #0x00
	ld	b, #0x02
00127$:
	srl	-4 (ix)
	rr	-5 (ix)
	rr	-6 (ix)
	djnz	00127$
	ld	hl, #_game
	add	hl, de
	ld	c, (hl)
	inc	hl
	ld	b, (hl)
	pop	de
	pop	hl
	push	hl
	push	de
	push	hl
	ld	l, -4 (ix)
	ld	h, -3 (ix)
	push	hl
	ld	l, -6 (ix)
	ld	h, -5 (ix)
	push	hl
	push	bc
	ld	hl, #___str_7
	push	hl
	call	_printf
	ld	hl, #10
	add	hl, sp
	ld	sp, hl
;src/menu.c:70: for (int i = 0; (i < FILES_PER_PAGE) && (xi<totalFiles-1) ; i++)  // Loop through the files
	inc	-2 (ix)
	jp	NZ,00104$
	inc	-1 (ix)
	jp	00104$
00101$:
;src/menu.c:77: Locate(0, 21);
	ld	a, #0x15
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src/menu.c:78: printf("--------------------------------------");
	ld	hl, #___str_6
	ex	(sp),hl
	call	_printf
;src/menu.c:79: Locate(0, 22);
	ld	h,#0x16
	ex	(sp),hl
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
	pop	af
;src/menu.c:80: printf("Page %02d/%02d     [H - Help] [C - Config]",currentPage, totalPages); // Print the page number and the help and config options
	ld	hl, (_totalPages)
	push	hl
	ld	hl, (_currentPage)
	push	hl
	ld	hl, #___str_8
	push	hl
	call	_printf
	ld	hl, #6
	add	hl, sp
	ld	sp, hl
;src/menu.c:81: Locate(0, (currentIndex%FILES_PER_PAGE) + 2); // Position the cursor on the selected file
	ld	hl, #0x0013
	push	hl
	ld	hl, (_currentIndex)
	push	hl
	call	__modsint
	pop	af
	pop	af
	ld	a, l
	add	a, #0x02
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src/menu.c:82: printf(">"); // Print the cursor
	ld	hl, #___str_9
	ex	(sp),hl
	call	_printf
;src/menu.c:83: }
	ld	sp,ix
	pop	ix
	ret
___str_5:
	.ascii "MSX PICOVERSE 2040     [MultiROM v1.0]"
	.db 0x00
___str_6:
	.ascii "--------------------------------------"
	.db 0x00
___str_7:
	.ascii "%-22s %4luKB %-8s"
	.db 0x00
___str_8:
	.ascii "Page %02d/%02d     [H - Help] [C - Config]"
	.db 0x00
___str_9:
	.ascii ">"
	.db 0x00
;src/menu.c:85: void helpMenu()
;	---------------------------------
; Function helpMenu
; ---------------------------------
_helpMenu::
	push	ix
	ld	ix,#0
	add	ix,sp
	push	af
;src/menu.c:88: Cls(); // Clear the screen
	call	_Cls
;src/menu.c:89: printf("MSX PICOVERSE 2040     [MultiROM v1.0]");
	ld	hl, #___str_10
	push	hl
	call	_printf
;src/menu.c:90: Locate(0, 1);
	ld	h,#0x01
	ex	(sp),hl
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src/menu.c:91: printf("--------------------------------------");
	ld	hl, #___str_11
	ex	(sp),hl
	call	_printf
;src/menu.c:92: Locate(0, 2);
	ld	h,#0x02
	ex	(sp),hl
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src/menu.c:93: printf("Use [UP]  [DOWN] to navigate the menu.");
	ld	hl, #___str_12
	ex	(sp),hl
	call	_printf
;src/menu.c:94: Locate(0, 3);
	ld	h,#0x03
	ex	(sp),hl
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src/menu.c:95: printf("Use [LEFT] [RIGHT] to navigate  pages.");
	ld	hl, #___str_13
	ex	(sp),hl
	call	_printf
;src/menu.c:96: Locate(0, 4);
	ld	h,#0x04
	ex	(sp),hl
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src/menu.c:97: printf("Press [H] to display the help screen.");
	ld	hl, #___str_14
	ex	(sp),hl
	call	_printf
;src/menu.c:98: Locate(0, 5);
	ld	h,#0x05
	ex	(sp),hl
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src/menu.c:99: printf("Press [C] to display the config page.");
	ld	hl, #___str_15
	ex	(sp),hl
	call	_printf
;src/menu.c:100: Locate(0, 7);
	ld	h,#0x07
	ex	(sp),hl
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
	pop	af
;src/menu.c:107: col = 0;
	ld	bc, #0x0000
;src/menu.c:109: for (int i = 33; i < 256; i++) 
	ld	hl, #0x0008
	ex	(sp), hl
	ld	de, #0x0021
00105$:
	ld	a, d
	xor	a, #0x80
	sub	a, #0x81
	jr	NC,00103$
;src/menu.c:112: Locate(col, row);
	ld	h, -2 (ix)
	ld	a, c
	push	bc
	push	de
	push	hl
	inc	sp
	push	af
	inc	sp
	call	_Locate
	pop	af
	pop	de
	pop	bc
;src/menu.c:114: PrintChar(i);
	ld	a, e
	push	bc
	push	de
	push	af
	inc	sp
	call	_PrintChar
	inc	sp
	pop	de
	pop	bc
;src/menu.c:116: col++;
	inc	bc
;src/menu.c:118: if (col >= 40) {
	ld	a, c
	sub	a, #0x28
	ld	a, b
	rla
	ccf
	rra
	sbc	a, #0x80
	jr	C,00106$
;src/menu.c:119: col = 0;
	ld	bc, #0x0000
;src/menu.c:120: row++;
	inc	-2 (ix)
	jr	NZ,00125$
	inc	-1 (ix)
00125$:
00106$:
;src/menu.c:109: for (int i = 33; i < 256; i++) 
	inc	de
	jr	00105$
00103$:
;src/menu.c:124: Locate(0, 21);
	ld	a, #0x15
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src/menu.c:125: printf("--------------------------------------");
	ld	hl, #___str_11
	ex	(sp),hl
	call	_printf
;src/menu.c:126: Locate(0, 22);
	ld	h,#0x16
	ex	(sp),hl
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src/menu.c:127: printf("Press any key to return to the menu...");
	ld	hl, #___str_16
	ex	(sp),hl
	call	_printf
	pop	af
;src/menu.c:128: WaitKey();
	call	_WaitKey
;src/menu.c:129: displayMenu();
	call	_displayMenu
;src/menu.c:130: navigateMenu();
	call	_navigateMenu
;src/menu.c:131: }
	ld	sp, ix
	pop	ix
	ret
___str_10:
	.ascii "MSX PICOVERSE 2040     [MultiROM v1.0]"
	.db 0x00
___str_11:
	.ascii "--------------------------------------"
	.db 0x00
___str_12:
	.ascii "Use [UP]  [DOWN] to navigate the menu."
	.db 0x00
___str_13:
	.ascii "Use [LEFT] [RIGHT] to navigate  pages."
	.db 0x00
___str_14:
	.ascii "Press [H] to display the help screen."
	.db 0x00
___str_15:
	.ascii "Press [C] to display the config page."
	.db 0x00
___str_16:
	.ascii "Press any key to return to the menu..."
	.db 0x00
;src/menu.c:133: void navigateMenu() 
;	---------------------------------
; Function navigateMenu
; ---------------------------------
_navigateMenu::
	push	ix
	ld	ix,#0
	add	ix,sp
	push	af
	dec	sp
;src/menu.c:137: while (1) 
00124$:
;src/menu.c:143: Locate(0, (currentIndex%FILES_PER_PAGE) + 2);
	ld	hl, #0x0013
	push	hl
	ld	hl, (_currentIndex)
	push	hl
	call	__modsint
	pop	af
	pop	af
	ld	a, l
	add	a, #0x02
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
	pop	af
;src/menu.c:144: key = WaitKey();
	call	_WaitKey
	ld	-3 (ix), l
;src/menu.c:150: Locate(0, (currentIndex%FILES_PER_PAGE) + 2); // Position the cursor on the selected file
	ld	hl, #0x0013
	push	hl
	ld	hl, (_currentIndex)
	push	hl
	call	__modsint
	pop	af
	pop	af
	ld	a, l
	add	a, #0x02
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src/menu.c:151: printf(" "); // Clear the cursor
	ld	hl, #___str_17
	ex	(sp),hl
	call	_printf
	pop	af
;src/menu.c:169: currentPage++; // Move to the next page
	ld	de, (_currentPage)
	inc	de
;src/menu.c:152: switch (key) 
	ld	a, -3 (ix)
	sub	a, #0x1c
	jp	Z,00114$
;src/menu.c:158: if (currentIndex < ((currentPage-1) * FILES_PER_PAGE))  // Check if we need to move to the previous page
	ld	bc, (_currentPage)
	dec	bc
;src/menu.c:152: switch (key) 
	ld	a, -3 (ix)
	sub	a, #0x1d
	jp	Z,00117$
;src/menu.c:143: Locate(0, (currentIndex%FILES_PER_PAGE) + 2);
	push	bc
	push	de
	ld	hl, #0x0013
	push	hl
	ld	hl, (_currentIndex)
	push	hl
;src/menu.c:152: switch (key) 
	call	__modsint
	pop	af
	pop	af
	ld	-2 (ix), l
	ld	-1 (ix), h
	pop	de
	pop	bc
	ld	a, -3 (ix)
	sub	a, #0x1e
	jr	Z,00101$
	ld	a, -3 (ix)
	sub	a, #0x1f
	jr	Z,00108$
	ld	a, -3 (ix)
	sub	a, #0x48
	jp	Z,00121$
	ld	a, -3 (ix)
	sub	a, #0x68
	jp	Z,00121$
	jp	00122$
;src/menu.c:154: case 30: // Up arrow
00101$:
;src/menu.c:155: if (currentIndex > 0) // Check if we are not at the first file
	xor	a, a
	ld	iy, #_currentIndex
	cp	a, 0 (iy)
	sbc	a, 1 (iy)
	jp	PO, 00209$
	xor	a, #0x80
00209$:
	jp	P, 00122$
;src/menu.c:157: if (currentIndex%FILES_PER_PAGE >= 0) currentIndex--; // Move to the previous file
	bit	7, -1 (ix)
	jr	NZ,00103$
	ld	hl, (_currentIndex)
	dec	hl
	ld	(_currentIndex), hl
00103$:
;src/menu.c:158: if (currentIndex < ((currentPage-1) * FILES_PER_PAGE))  // Check if we need to move to the previous page
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
	jp	PO, 00210$
	xor	a, #0x80
00210$:
	jp	P, 00122$
;src/menu.c:160: currentPage--; // Move to the previous page
	ld	(_currentPage), bc
;src/menu.c:161: displayMenu(); // Display the menu
	call	_displayMenu
;src/menu.c:164: break;
	jp	00122$
;src/menu.c:165: case 31: // Down arrow
00108$:
;src/menu.c:166: if ((currentIndex%FILES_PER_PAGE < FILES_PER_PAGE) && currentIndex < totalFiles-1) currentIndex++; // Move to the next file
	ld	a, -2 (ix)
	sub	a, #0x13
	ld	a, -1 (ix)
	rla
	ccf
	rra
	sbc	a, #0x80
	jr	NC,00110$
	ld	bc, (_totalFiles)
	dec	bc
	ld	iy, #_currentIndex
	ld	a, 0 (iy)
	sub	a, c
	ld	a, 1 (iy)
	sbc	a, b
	jp	PO, 00211$
	xor	a, #0x80
00211$:
	jp	P, 00110$
	ld	hl, (_currentIndex)
	inc	hl
	ld	(_currentIndex), hl
00110$:
;src/menu.c:167: if (currentIndex >= (currentPage * FILES_PER_PAGE)) // Check if we need to move to the next page
	ld	bc, (_currentPage)
	ld	l, c
	ld	h, b
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, bc
	add	hl, hl
	add	hl, bc
	ld	c, l
	ld	b, h
	ld	iy, #_currentIndex
	ld	a, 0 (iy)
	sub	a, c
	ld	a, 1 (iy)
	sbc	a, b
	jp	PO, 00212$
	xor	a, #0x80
00212$:
	jp	M, 00122$
;src/menu.c:169: currentPage++; // Move to the next page
	ld	(_currentPage), de
;src/menu.c:170: displayMenu(); // Display the menu
	call	_displayMenu
;src/menu.c:172: break;
	jr	00122$
;src/menu.c:173: case 28: // Right arrow
00114$:
;src/menu.c:174: if (currentPage < totalPages) // Check if we are not on the last page
	ld	hl, #_totalPages
	ld	iy, #_currentPage
	ld	a, 0 (iy)
	sub	a, (hl)
	ld	a, 1 (iy)
	inc	hl
	sbc	a, (hl)
	jp	PO, 00213$
	xor	a, #0x80
00213$:
	jp	P, 00122$
;src/menu.c:176: currentPage++; // Move to the next page
	ld	(_currentPage), de
;src/menu.c:177: currentIndex = (currentPage-1) * FILES_PER_PAGE; // Move to the first file of the page
	ld	bc, (_currentPage)
	dec	bc
	ld	l, c
	ld	h, b
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, bc
	add	hl, hl
	add	hl, bc
	ld	(_currentIndex), hl
;src/menu.c:178: displayMenu(); // Display the menu
	call	_displayMenu
;src/menu.c:180: break;
	jr	00122$
;src/menu.c:181: case 29: // Left arrow
00117$:
;src/menu.c:182: if (currentPage > 1) // Check if we are not on the first page
	ld	a, #0x01
	ld	iy, #_currentPage
	cp	a, 0 (iy)
	ld	a, #0x00
	sbc	a, 1 (iy)
	jp	PO, 00214$
	xor	a, #0x80
00214$:
	jp	P, 00122$
;src/menu.c:184: currentPage--; // Move to the previous page
	ld	(_currentPage), bc
;src/menu.c:185: currentIndex = (currentPage-1) * FILES_PER_PAGE; // Move to the first file of the page
	ld	bc, (_currentPage)
	dec	bc
	ld	l, c
	ld	h, b
	add	hl, hl
	add	hl, hl
	add	hl, hl
	add	hl, bc
	add	hl, hl
	add	hl, bc
	ld	(_currentIndex), hl
;src/menu.c:186: displayMenu(); // Display the menu
	call	_displayMenu
;src/menu.c:188: break;
	jr	00122$
;src/menu.c:190: case 104: // h - Help (lowercase h)
00121$:
;src/menu.c:192: helpMenu(); // Display the help menu
	call	_helpMenu
;src/menu.c:194: }
00122$:
;src/menu.c:195: Locate(0, (currentIndex%FILES_PER_PAGE) + 2); // Position the cursor on the selected file
	ld	hl, #0x0013
	push	hl
	ld	hl, (_currentIndex)
	push	hl
	call	__modsint
	pop	af
	pop	af
	ld	a, l
	add	a, #0x02
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
;src/menu.c:196: printf(">"); // Print the cursor
	ld	hl, #___str_18
	ex	(sp),hl
	call	_printf
;src/menu.c:197: Locate(0, (currentIndex%FILES_PER_PAGE) + 2); // Position the cursor on the selected file
	ld	hl, #0x0013
	ex	(sp),hl
	ld	hl, (_currentIndex)
	push	hl
	call	__modsint
	pop	af
	pop	af
	ld	a, l
	add	a, #0x02
	push	af
	inc	sp
	xor	a, a
	push	af
	inc	sp
	call	_Locate
	pop	af
;src/menu.c:199: }
	jp	00124$
___str_17:
	.ascii " "
	.db 0x00
___str_18:
	.ascii ">"
	.db 0x00
;src/menu.c:203: void main() {
;	---------------------------------
; Function main
; ---------------------------------
_main::
;src/menu.c:205: currentPage = 1; // Start on page 1
	ld	hl, #0x0001
	ld	(_currentPage), hl
;src/menu.c:206: currentIndex = 0; // Start at the first file - index 0
	ld	l, #0x00
	ld	(_currentIndex), hl
;src/menu.c:208: totalFiles = 22; // Total of files stored on the flash
	ld	l, #0x16
	ld	(_totalFiles), hl
;src/menu.c:209: totalPages = (int)((totalFiles/FILES_PER_PAGE)+1); // Calculate the total pages based on the total files and files per page
	ld	l, #0x02
	ld	(_totalPages), hl
;src/menu.c:219: game[0] =  "Metal Gear          ";    mapp[0] = 3; size[0] = 131072;
	ld	hl, #___str_19
	ld	(_game), hl
	ld	hl, #0x0003
	ld	(_mapp), hl
	ld	l, #0x00
	ld	(_size), hl
	ld	hl, #0x0002
	ld	(_size+2), hl
;src/menu.c:220: game[1] =  "Nemesis             ";    mapp[1] = 3; size[1] = 131072;
	ld	hl, #___str_20
	ld	((_game + 0x0002)), hl
	ld	hl, #0x0003
	ld	((_mapp + 0x0002)), hl
	ld	l, #0x00
	ld	((_size + 0x0004)), hl
	ld	hl, #0x0002
	ld	((_size + 0x0004)+2), hl
;src/menu.c:221: game[2] =  "Contra              ";    mapp[2] = 3; size[2] = 131072;
	ld	hl, #___str_21
	ld	((_game + 0x0004)), hl
	ld	hl, #0x0003
	ld	((_mapp + 0x0004)), hl
	ld	l, #0x00
	ld	((_size + 0x0008)), hl
	ld	hl, #0x0002
	ld	((_size + 0x0008)+2), hl
;src/menu.c:222: game[3] =  "Castlevania         ";    mapp[3] = 3; size[3] = 131072;
	ld	hl, #___str_22
	ld	((_game + 0x0006)), hl
	ld	hl, #0x0003
	ld	((_mapp + 0x0006)), hl
	ld	l, #0x00
	ld	((_size + 0x000c)), hl
	ld	hl, #0x0002
	ld	((_size + 0x000c)+2), hl
;src/menu.c:223: game[4] =  "Kings Valley II     ";    mapp[4] = 3; size[4] = 131072;
	ld	hl, #___str_23
	ld	((_game + 0x0008)), hl
	ld	hl, #0x0003
	ld	((_mapp + 0x0008)), hl
	ld	l, #0x00
	ld	((_size + 0x0010)), hl
	ld	hl, #0x0002
	ld	((_size + 0x0010)+2), hl
;src/menu.c:224: game[5] =  "Vampire Killer      ";    mapp[5] = 3; size[5] = 131072;
	ld	hl, #___str_24
	ld	((_game + 0x000a)), hl
	ld	hl, #0x0003
	ld	((_mapp + 0x000a)), hl
	ld	l, #0x00
	ld	((_size + 0x0014)), hl
	ld	hl, #0x0002
	ld	((_size + 0x0014)+2), hl
;src/menu.c:225: game[6] =  "Snatcher            ";    mapp[6] = 3; size[6] = 131072;
	ld	hl, #___str_25
	ld	((_game + 0x000c)), hl
	ld	hl, #0x0003
	ld	((_mapp + 0x000c)), hl
	ld	l, #0x00
	ld	((_size + 0x0018)), hl
	ld	hl, #0x0002
	ld	((_size + 0x0018)+2), hl
;src/menu.c:226: game[7] =  "Galaga              ";    mapp[7] = 2; size[7] = 32768;
	ld	hl, #___str_26
	ld	((_game + 0x000e)), hl
	ld	hl, #0x0002
	ld	((_mapp + 0x000e)), hl
	ld	hl, #0x8000
	ld	((_size + 0x001c)), hl
	ld	hl, #0x0000
	ld	((_size + 0x001c)+2), hl
;src/menu.c:227: game[8] =  "Zaxxon              ";    mapp[8] = 2; size[8] = 32768;
	ld	hl, #___str_27
	ld	((_game + 0x0010)), hl
	ld	hl, #0x0002
	ld	((_mapp + 0x0010)), hl
	ld	hl, #0x8000
	ld	((_size + 0x0020)), hl
	ld	hl, #0x0000
	ld	((_size + 0x0020)+2), hl
;src/menu.c:228: game[9] =  "Salamander          ";    mapp[9] = 3; size[9] = 131072;
	ld	hl, #___str_28
	ld	((_game + 0x0012)), hl
	ld	hl, #0x0003
	ld	((_mapp + 0x0012)), hl
	ld	l, #0x00
	ld	((_size + 0x0024)), hl
	ld	hl, #0x0002
	ld	((_size + 0x0024)+2), hl
;src/menu.c:229: game[10] = "Parodius            ";    mapp[10] = 3; size[10] = 131072;
	ld	hl, #___str_29
	ld	((_game + 0x0014)), hl
	ld	hl, #0x0003
	ld	((_mapp + 0x0014)), hl
	ld	l, #0x00
	ld	((_size + 0x0028)), hl
	ld	hl, #0x0002
	ld	((_size + 0x0028)+2), hl
;src/menu.c:230: game[11] = "Knightmare          ";    mapp[11] = 2; size[11] = 32768;
	ld	hl, #___str_30
	ld	((_game + 0x0016)), hl
	ld	hl, #0x0002
	ld	((_mapp + 0x0016)), hl
	ld	hl, #0x8000
	ld	((_size + 0x002c)), hl
	ld	hl, #0x0000
	ld	((_size + 0x002c)+2), hl
;src/menu.c:231: game[12] = "Pippols             ";    mapp[12] = 1; size[12] = 16384;
	ld	hl, #___str_31
	ld	((_game + 0x0018)), hl
	ld	hl, #0x0001
	ld	((_mapp + 0x0018)), hl
	ld	hl, #0x4000
	ld	((_size + 0x0030)), hl
	ld	hl, #0x0000
	ld	((_size + 0x0030)+2), hl
;src/menu.c:232: game[13] = "The Maze of Galious ";    mapp[13] = 3; size[13] = 131072;
	ld	hl, #___str_32
	ld	((_game + 0x001a)), hl
	ld	hl, #0x0003
	ld	((_mapp + 0x001a)), hl
	ld	l, #0x00
	ld	((_size + 0x0034)), hl
	ld	hl, #0x0002
	ld	((_size + 0x0034)+2), hl
;src/menu.c:233: game[14] = "Penguin Adventure   ";    mapp[14] = 2; size[14] = 32768;
	ld	hl, #___str_33
	ld	((_game + 0x001c)), hl
	ld	hl, #0x0002
	ld	((_mapp + 0x001c)), hl
	ld	hl, #0x8000
	ld	((_size + 0x0038)), hl
	ld	hl, #0x0000
	ld	((_size + 0x0038)+2), hl
;src/menu.c:234: game[15] = "Space Manbow        ";    mapp[15] = 3; size[15] = 131072;
	ld	hl, #___str_34
	ld	((_game + 0x001e)), hl
	ld	hl, #0x0003
	ld	((_mapp + 0x001e)), hl
	ld	l, #0x00
	ld	((_size + 0x003c)), hl
	ld	hl, #0x0002
	ld	((_size + 0x003c)+2), hl
;src/menu.c:235: game[16] = "Gradius 2           ";    mapp[16] = 3; size[16] = 131072;
	ld	hl, #___str_35
	ld	((_game + 0x0020)), hl
	ld	hl, #0x0003
	ld	((_mapp + 0x0020)), hl
	ld	l, #0x00
	ld	((_size + 0x0040)), hl
	ld	hl, #0x0002
	ld	((_size + 0x0040)+2), hl
;src/menu.c:236: game[17] = "TwinBee             ";    mapp[17] = 1; size[17] = 16384;
	ld	hl, #___str_36
	ld	((_game + 0x0022)), hl
	ld	hl, #0x0001
	ld	((_mapp + 0x0022)), hl
	ld	hl, #0x4000
	ld	((_size + 0x0044)), hl
	ld	hl, #0x0000
	ld	((_size + 0x0044)+2), hl
;src/menu.c:237: game[18] = "Zanac               ";    mapp[18] = 2; size[18] = 32768;
	ld	hl, #___str_37
	ld	((_game + 0x0024)), hl
	ld	hl, #0x0002
	ld	((_mapp + 0x0024)), hl
	ld	hl, #0x8000
	ld	((_size + 0x0048)), hl
	ld	hl, #0x0000
	ld	((_size + 0x0048)+2), hl
;src/menu.c:238: game[19] = "H.E.R.O.            ";    mapp[19] = 1; size[19] = 16384;
	ld	hl, #___str_38
	ld	((_game + 0x0026)), hl
	ld	hl, #0x0001
	ld	((_mapp + 0x0026)), hl
	ld	hl, #0x4000
	ld	((_size + 0x004c)), hl
	ld	hl, #0x0000
	ld	((_size + 0x004c)+2), hl
;src/menu.c:239: game[20] = "Yie Ar Kung-Fu      ";    mapp[20] = 2; size[20] = 32768;
	ld	hl, #___str_39
	ld	((_game + 0x0028)), hl
	ld	hl, #0x0002
	ld	((_mapp + 0x0028)), hl
	ld	hl, #0x8000
	ld	((_size + 0x0050)), hl
	ld	hl, #0x0000
	ld	((_size + 0x0050)+2), hl
;src/menu.c:240: game[21] = "XRacing             ";    mapp[21] = 4; size[21] = 49152;
	ld	hl, #___str_40
	ld	((_game + 0x002a)), hl
	ld	hl, #0x0004
	ld	((_mapp + 0x002a)), hl
	ld	hl, #0xc000
	ld	((_size + 0x0054)), hl
	ld	hl, #0x0000
	ld	((_size + 0x0054)+2), hl
;src/menu.c:243: displayMenu();
	call	_displayMenu
;src/menu.c:245: navigateMenu();
;src/menu.c:246: }
	jp  _navigateMenu
___str_19:
	.ascii "Metal Gear          "
	.db 0x00
___str_20:
	.ascii "Nemesis             "
	.db 0x00
___str_21:
	.ascii "Contra              "
	.db 0x00
___str_22:
	.ascii "Castlevania         "
	.db 0x00
___str_23:
	.ascii "Kings Valley II     "
	.db 0x00
___str_24:
	.ascii "Vampire Killer      "
	.db 0x00
___str_25:
	.ascii "Snatcher            "
	.db 0x00
___str_26:
	.ascii "Galaga              "
	.db 0x00
___str_27:
	.ascii "Zaxxon              "
	.db 0x00
___str_28:
	.ascii "Salamander          "
	.db 0x00
___str_29:
	.ascii "Parodius            "
	.db 0x00
___str_30:
	.ascii "Knightmare          "
	.db 0x00
___str_31:
	.ascii "Pippols             "
	.db 0x00
___str_32:
	.ascii "The Maze of Galious "
	.db 0x00
___str_33:
	.ascii "Penguin Adventure   "
	.db 0x00
___str_34:
	.ascii "Space Manbow        "
	.db 0x00
___str_35:
	.ascii "Gradius 2           "
	.db 0x00
___str_36:
	.ascii "TwinBee             "
	.db 0x00
___str_37:
	.ascii "Zanac               "
	.db 0x00
___str_38:
	.ascii "H.E.R.O.            "
	.db 0x00
___str_39:
	.ascii "Yie Ar Kung-Fu      "
	.db 0x00
___str_40:
	.ascii "XRacing             "
	.db 0x00
	.area _CODE
	.area _INITIALIZER
	.area _CABS (ABS)
