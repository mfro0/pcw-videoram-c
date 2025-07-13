;--------------------------------------------------------------------------
;  cpm0.s - Generic cpm0.s for a Z80 CP/M-80 v2.2 Application
;  Copyright (C) 2011, Douglas Goodall All Rights Reserved.
;--------------------------------------------------------------------------

	.module	crt0
       	.globl	_main
	.globl	_cpm_fcb
	.globl	_cpm_fcb2
	.globl	_cpm_default_dma

	.area 	_CODE
	;.area	_HEADER(ABS)


	_cpm_fcb = 0x5c
	_cpm_fcb2 = 0x6c

	_cpm_default_dma = 0x80

init:
	;; Define an adequate stack (at end of TPA)
	ld	hl,#0
	add	hl,sp
	ld	(oldstack),hl
	ld	hl,(6)
	ld	sp,hl

        ;; Initialise global variables
        call    gsinit

	;; Call the C main routine
	call	_main

	ld	c,#0
	call	5
	ld	hl,(oldstack)
	ret

	halt

	;; Ordering of segments for the linker.

	.area	_HOME
	.area	_CODE
	.area	_INITIALIZER
        .area   _GSINIT
        .area   _GSFINAL
	.area	_DATA
	.area	_INITIALIZED
	.area	_BSEG
	.area	_BSS
	.area	_HEAP

	.area	_END
oldstack:
	.dw	1

        .area   _GSINIT
gsinit::
	; Default-initialized global variables
	ld	bc,#l__DATA
	ld	a,b
	or	a,c
	jr	z,zeroed_data
	ld	hl,#s__DATA
	ld	(hl),#0x00
	dec	bc
	ld	a,b
	or	a,c
	jr	z,zeroed_data
	ld	e,l
	ld	d,h
	inc	de
	ldir

zeroed_data:
	; explicitely initialized global variables
	ld	bc,#l__INITIALIZER
	ld	a,b
	or	a,c
	jr	z,gsinit_next
	ld	de,#s__INITIALIZED
	ld	hl,#s__INITIALIZER
	ldir
gsinit_next:

        .area   _GSFINAL
        ret

	.area	_END
_cpm_ram:
	.db	0xe5

;;;;;;;;;;;;;;;;
; eof - cpm0.s ;
;;;;;;;;;;;;;;;;
