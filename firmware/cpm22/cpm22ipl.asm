;=======================================================
;
; 2nd boot loader for CP/M 62K on sbcz80cpm
;
;=======================================================
                .z80
DST             .equ    0f200h   ; BIOS start address
REPEAT          .equ    0ch
LENGTH          .equ    00h
READPORT        .equ    08fh
UART            .equ    081h

                .org    0ff00h
START:
                LD      HL, MSG1
                LD      A, 03h
                LD      C, READPORT
                OUT     (C), A          ; ready boot image
MSG1LOOP:
                LD      A, (HL)
                CP      00h
                JR      Z, MAIN
                OUT     (UART), A
                INC     HL
                JP      MSG1LOOP
MAIN:
                LD      HL, DST
                LD      D, REPEAT
LOOP:
                LD      B, LENGTH
                INIR                    ; copy binary from IO to RAM
                LD      A, D
                AND     001h
                JR      NZ, NEXT
                LD      A, '.'
                OUT     (UART), A
NEXT:
                DEC     D
                JR      NZ, LOOP
MAINEND:
                LD      HL, MSG2
MSG2LOOP:
                LD      A, (HL)
                CP      00h
                JR      Z, END
                OUT     (UART), A
                INC     HL
                JP      MSG2LOOP
END:
                JP      DST

MSG1:
                .byte   " Loading BIOS ", 00h
MSG2:
                .byte   " Finished", 0dh, 0ah, 00h

        .end