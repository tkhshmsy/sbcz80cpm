;
; cp/m 2.2 62K BIOS for sbcz80cpm
;
; written by tkhshmsy@gmail.com
;
                .z80
VERS            .equ    22                      ; CP/M version
BIOSVERS        .equ    01                      ; BIOS version
MSIZE           .equ    62

BIAS            .equ    (MSIZE - 20) * 1024     ; A800h
CCP             .equ    03400h + BIAS            ; DC00h
BDOS            .equ    CCP + 0806h              ; E406h
BIOS            .equ    CCP + 01600h             ; F200h
CPMBOOT         .equ    CCP
WBOOTJUMPER     .equ    0000h
IOBYTE          .equ    0003h
CURRENTDISK     .equ    0004h
BDOSJUMPER      .equ    0005h
FILECTRLBLK     .equ    005ch
DEFAULTDMAADDR  .equ    0080h
TPA_BASE        .equ    0100h

IOE_UARTST      .equ    80h
IOE_UART        .equ    81h
IOE_SELDSK      .equ    82h
IOE_SETTRK      .equ    83h
IOE_SETSEC      .equ    84h
IOE_DSKIO       .equ    85h
IOE_BOOT        .equ    8fh

TRACKOFFSET     .equ    2
MAX_DISKS       .equ    4

LF              .equ    0ah
CR              .equ    0dh

                .org   BIOS
NSECTS          .equ    ($ - CCP)/128
                JP     BOOT
WBOOTENTRY:     JP     WBOOT
                JP     CONST
                JP     CONIN
                JP     CONOUT
                JP     LIST
                JP     PUNCH
                JP     READER
                JP     HOME
                JP     SELDSK
                JP     SETTRK
                JP     SETSEC
                JP     SETDMA
                JP     READ
                JP     WRITE
                JP     LISTST
                JP     SECTRAN
                ; for CP/M 3
                ; JP     CONOST
                ; JP     AUXIST
                ; JP     AUXOST
                ; JP     DEVTBL
                ; JP     MULTIO
                ; JP     FLUSH
                ; JP     MOVE
                ; JP     TIME
                ; JP     SELMEM
                ; JP     SETBNK
                ; JP     XMOVE
                ; JP     USERF
                ; JP     RESERVE1
                ; JP     RESERVE2

; Disk Parameter Header
; for ibm-3740 format
; 4 disks
DPBASE:         .dw     DPTRANS, 0x0000
                .dw     0x0000, 0x0000
                .dw     DIRBUF,DPBLK
                .dw     CSV00, ALV00
                .dw     DPTRANS, 0x0000
                .dw     0x0000, 0x0000
                .dw     DIRBUF, DPBLK
                .dw     CSV01, ALV01
                .dw     DPTRANS, 0x0000
                .dw     0x0000, 0x0000
                .dw     DIRBUF, DPBLK
                .dw     CSV02, ALV02
                .dw     DPTRANS, 0x0000
                .dw     0x0000, 0x0000
                .dw     DIRBUF, DPBLK
                .dw     CSV03, ALV03
; skew table
DPTRANS:	.db	 1,  7, 13, 19	;sectors  1,  2,  3,  4
                .db	25,  5, 11, 17	;sectors  5,  6,  7,  6
                .db	23,  3,  9, 15	;sectors  9, 10, 11, 12
                .db	21,  2,  8, 14	;sectors 13, 14, 15, 16
                .db	20, 26,  6, 12	;sectors 17, 18, 19, 20
                .db	18, 24,  4, 10	;sectors 21, 22, 23, 24
                .db	16, 22		;sectors 25, 26
; disk parameters
DPBLK:  	                        ;disk parameter block, common to all disks
                .dw	26		;sectors per track
                .db	3		;block shift factor
                .db	7		;block mask
                .db	0		;null mask
                .dw	242		;disk size-1
                .dw	63		;directory max
                .db	192		;alloc 0
                .db	0		;alloc 1
                .dw	16		;check size
                .dw	2		;track offset

BOOTMSG:        .db     CR, LF, LF
                .db     MSIZE/10 + "0", MSIZE % 10 + "0"
                .db     "K CP/M ver. "
                .db     VERS/10 + "0", ".", VERS % 10 + "0"
                .db     CR, LF
                .db     "BIOS ver. "
                .db     BIOSVERS/10 + "0", ".", BIOSVERS % 10 + "0"
                .db     " for sbcz80cpm"
                .db     CR, LF, 0

BOOT:
                ; DI
                LD      SP, DEFAULTDMAADDR + 080h
                LD      HL, DMAADDR
                LD      (HL), DEFAULTDMAADDR                
                LD      HL, BOOTMSG
                CALL    CONSTROUT
                XOR     A
                LD      (CURRENTDISK), A
                LD      (PREVDISK), A
                LD      (CUREENTSECTOR), A
                LD      A, 11h
                LD      (IOBYTE), A
WBOOT:
                LD      SP, DEFAULTDMAADDR + 080h
                LD      C, 0
                CALL    SELDSK
                CALL    HOME

                ; DI
                CALL    LOADSYS
GOCPM:
                LD      BC, DEFAULTDMAADDR
                CALL    SETDMA
                LD      A, 0c3h         ; self write
                LD      (WBOOTJUMPER), A
                LD      HL, WBOOTENTRY
                LD      (WBOOTJUMPER + 1), HL
                LD      (BDOSJUMPER), A
                LD      HL, BDOS
                LD      (BDOSJUMPER + 1), HL
                ; EI
                LD      A, (CURRENTDISK)
                LD      C, A
                JP      CPMBOOT

CONSTROUT:                              ; console out string with HL, Break:A,C
                LD      A, (HL)
                CP      00h
                RET     Z
                LD      C, A
                CALL    CONOUT
                INC     HL
                JR      CONSTROUT
CONHEXOUT:                              ; console out hex with C, Break: A,B,C
                LD      A, C
                LD      B, A
                SRA     A
                SRA     A
                SRA     A
                SRA     A
                AND     0fh
                ADD     A, 090H
                DAA
                ADC     A, 040H
                DAA
                LD      C, A
                CALL    CONOUT
                LD      A, B
                AND     0fh
                ADD     A, 090H
                DAA
                ADC     A, 040H
                DAA
                LD      C, A
                CALL    CONOUT
                RET

CONST:                                  ; console in status, Break:A
                IN      A, (IOE_UARTST)
                AND     01h
                JR      Z, CONSTNOCHAR
                LD      A, 0FFh
                RET                     ; return A=0(no char), 0xffh(one more)
CONSTNOCHAR:    LD      A, 0
                RET
CONIN:                                  ; console in, Break:A
                IN      A, (IOE_UART)
                AND     07fh
                RET                     ; return A
CONOUT:                                 ; console out with C, Break:A
                LD      A, C
                OUT     (IOE_UART), A
                RET
LIST:                                   ; printer out with C, Break:A
                LD      A, C
                OUT     (IOE_UART), A
                RET
PUNCH:                                  ; tape out with C, Break:A
                LD      A, C
                OUT     (IOE_UART), A
                RET
READER:                                 ; tape read
                LD      A, 26
                RET                     ; return A, if error A=26
HOME:                                   ; home (set track to 0), Break:A,B,C
                LD      B, 0
                LD      C, 0
                CALL    SETTRK
                RET
SELDSK:                                 ; select disk with C,E, Break:A
                LD      HL, 0000h
                LD      A, C
                CP      MAX_DISKS
                JR      NC, SELDSKERR
                LD      A, (CURRENTDISK)
                LD      (PREVDISK), A
                LD      A, C
                LD      (CURRENTDISK), A
                OUT     (IOE_SELDSK), A
                LD      H, 0
                LD      L, A
                ADD     HL, HL
                ADD     HL, HL
                ADD     HL, HL
                ADD     HL, HL
                LD      DE, DPBASE
                ADD     HL, DE
                RET                     ; return HL=DPTABLE, if error HL=0
SELDSKERR:
                LD      HL, 0000h
                RET
SETTRK:                                 ; set track with BC, Break:A
                LD      A, C
                LD      (CURRENTTRACK), A
                OUT     (IOE_SETTRK), A
                RET
SETSEC:                                 ; set sector with BC, Break:A
                LD      A, C
                LD      (CUREENTSECTOR), A
                OUT     (IOE_SETSEC), A
                RET
SETDMA:                                 ; set DMA address with BC
                LD      H, B
                LD      L, C
                LD      (DMAADDR), HL
                RET
READ:                                   ; read into DMAADDR
                LD      C, IOE_DSKIO
                LD      B, 128
                LD      HL, (DMAADDR)
                INIR
                LD      A, 0
                RET                     ; return A=0(Ok), 1(unrecoverable), 0xffh(media changed)
WRITE:                                  ; write from DMAADDR  with C=0,1,2
                LD      C, IOE_DSKIO
                LD      B, 128
                LD      HL, (DMAADDR)
                OTIR
                LD      A, 0
                RET                     ; return A=0(Ok), if error, A=1(unrecover), 2(r/o), 0xff(media changed)

LISTST:                                 ; status of printer device
                LD      A, 0ffh
                RET                     ; return A=0(not ready), 0xffh(ready)
SECTRAN:                                ; sector translation with BC=logical sector, DE=base address
                LD      B, 0
                EX      DE, HL
                ADD     HL, BC
                LD      A, (HL)
                DEC     A               ; start sector number is 0
                LD      L, A
                LD      H, 0
                RET                     ; return HL=physical sector

; CONOST:
; AUXIST:
; AUXOST:
; DEVTBL:
; MULTIO:
; FLUSH:
; MOVE:
; TIME:
; SELMEM:
; SETBNK:
; XMOVE:
; USERF:
; RESERVE1:
; RESERVE2:
;                 RET

LOADSYS:
                XOR     A
                LD      (BOOTTRK), A
                LD      (BOOTSEC), A
                OUT     (IOE_SELDSK), A
                LD      HL, CCP
                LD      (BOOTADDR), HL
                LD      D, NSECTS
LOADSYSREAD128:
                LD      A, (BOOTTRK)
                OUT     (IOE_SETTRK), A
                LD      A, (BOOTSEC)            ; boot track is not skewed
                OUT     (IOE_SETSEC), A
                LD      HL, (BOOTADDR)
                LD      C, IOE_DSKIO
                LD      B, 128
                INIR
                LD      (BOOTADDR), HL
                DEC     D
                JR      Z, LOADSYSEND           ; reached to the top of the BIOS
                LD      A, (BOOTSEC)
                INC     A
                CP      26                      ; sectors per track
                JR      Z, LOADSYSNEXTTRK
                LD      (BOOTSEC), A
                JR      LOADSYSREAD128
LOADSYSNEXTTRK:
                LD      A, (BOOTTRK)
                INC     A
                CP      2
                JR      Z, LOADSYSEND
                LD      (BOOTTRK), A
                XOR     A
                LD      (BOOTSEC), A
                JR      LOADSYSREAD128
LOADSYSEND:
                LD      A, (CURRENTDISK)
                OUT     (IOE_SELDSK), A
                LD      A, (CURRENTTRACK)
                OUT     (IOE_SETTRK), A
                LD      A, (CUREENTSECTOR)
                OUT     (IOE_SETSEC), A
                LD      BC, DEFAULTDMAADDR
                CALL    SETDMA
                RET

BOOTTRK:        .ds     1
BOOTSEC:        .ds     1
BOOTADDR:       .dw     1

CURRENTTRACK:   .ds     1
CUREENTSECTOR:  .ds     1
DMAADDR:        .dw     1
PREVDISK:       .ds     1

DIRBUF:         .ds     128
ALV00:          .ds     31
ALV01:          .ds     31
ALV02:          .ds     31
ALV03:          .ds     31
CSV00:          .ds     16
CSV01:          .ds     16
CSV02:          .ds     16
CSV03:          .ds     16
                .end