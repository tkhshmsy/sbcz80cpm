;
; 1st boot loader for sbcz80cpm
;
; this code will be written into RAM with bus master
;

                .z80
DST             .equ    0ff00h
LENGTH          .equ    0ffh
READPORT        .equ    08fh

                .org    0000h
START:
                LD      A, 02h
                LD      B, LENGTH
                LD      C, READPORT
                LD      HL, DST
                OUT     (C), A          ; ready boot2nd image
                INIR                    ; copy binary from IO to RAM
                JP      DST

                .end