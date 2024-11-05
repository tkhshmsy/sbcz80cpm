;
; Test for 2nd boot loader
;
                .z80
UART            .equ    081h

                .org    0ff00h
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
                .byte   0dh, 0ah, "[TEST] 2nd boot is OK", 0dh, 0ah, 00h
                .end