;
; Test for main boot image
;

                .z80
UART            .equ    081h

                .org    0000h
START:
                LD      C, UART
                LD      HL, MSG
LOOP:
                LD      A, (HL)
                CP      00h
                JR      Z, END
                OUT     (C), A
                INC     HL
                JP      LOOP
END:
                HALT

MSG:
                .byte   0dh, 0ah, "[TEST] BootImage is loaded, and executed, OK !", 0dh, 0ah, 00h
                .end
