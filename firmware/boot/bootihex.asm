;
; intel hex loader as 2nd boot loader
;
; no checked: record-type
;
                .z80
UARTST          .equ    080h
UART            .equ    081h

CR              .equ     0DH
LF              .equ     0AH
CS              .equ     0CH

                .org 0ff00h
START:
                LD      C, UART
                LD      HL, STARTMSG
                CALL    PRINTSTR
READLINE:       CALL    CHARRD
                CP      ':'
                JR      NZ, READLINE
                XOR     A
                LD      (CHECKSUMBUF), A
                CALL    BYTERD
                LD      B, A
                CALL    BYTERD
                LD      H, A
                CALL    BYTERD
                LD      L, A
                CALL    BYTERD          ; record type
CHECKLOADED:    CP      01h
                JR      Z, LOADED
DATAIN:         CALL    BYTERD
                LD      (HL), A
                INC     HL
                DJNZ    DATAIN
CHECKSUM:       CALL    BYTERD          ; read checksum
                LD      A, '='
                OUT     (UART), A
                LD      A, (CHECKSUMBUF)
                CALL    PRINTHEX
                CP      00h
                JR      Z, READLINENEXT
                LD      A, '!'
                OUT     (UART), A
READLINENEXT:   
                LD      A, 0dh
                OUT     (UART), A
                LD      A, 0ah
                OUT     (UART), A
                JR      READLINE

CHARRD:         IN      A, (UART)
                CP      01fh
                JR      NC, CHARRDEND   ; control code
                CP      061h
                JR      C, CHARRDEND    ; convert to uppercase
                SUB     020h
CHARRDEND:      OUT     (UART), A
                RET
BYTERD:         LD      D, 00h
                CALL    HEXCON
                ADD     A, A
                ADD     A, A
                ADD     A, A
                ADD     A, A
                LD      D, A
                CALL    HEXCON
CALCCHECKSUM:   LD      D, A
                LD      A, (CHECKSUMBUF)
                ADD     D
                LD      (CHECKSUMBUF), A
                LD      A, D
                RET
HEXCON:         CALL    CHARRD
                SUB     30h
                CP      0ah
                JR      C, NALPHA
                SUB     07h
NALPHA:         OR      D
                RET

LOADED:         CALL    BYTERD          ; remove the last checksum 'FF'
                CALL    CHARRD          ; remove last char
                LD      HL, LOADEDMSG
                CALL    PRINTSTR
INPUT:          CALL    BYTERD
                LD      H, A
                CALL    BYTERD
                LD      L, A
                JP      (HL)

PRINTSTR:       LD      A, (HL)
                OR      A
                JR      Z, PRINTSTREND
                OUT     (UART), A
                INC     HL
                JR      PRINTSTR
PRINTSTREND:    RET
PRINTHEX:       LD      D, A
                SRA     A
                SRA     A
                SRA     A
                SRA     A
                AND     0fh
                ADD     A, 090H
                DAA
                ADC     A, 040H
                DAA
                OUT     (UART), A
                LD      A, D
                AND     0fh
                ADD     A, 090H
                DAA
                ADC     A, 040H
                DAA
                OUT     (UART), A
                LD      A, D
                RET

CHECKSUMBUF:    .ds     1
STARTMSG:       .byte   CR, LF, "HexLoading...", CR, LF, 00h
LOADEDMSG:      .byte   CR, LF, "Input StartAddress...", 00h
                .end
