;=====================================
;
; init NASCOM Basic 4.7 for sbcz80cpm
;
;=====================================
                .z80
UARTST          .equ     080H
UART            .equ     081H

BUF0            .equ     $2000
BUF1            .equ     BUF0+3FH
BUF2            .equ     BUF1+2
BUF3            .equ     BUF2+2
BASICSTART      .equ     BUF3+1
TEMPSTACK       .equ     $20ED

CR              .equ     0DH
LF              .equ     0AH
CS              .equ     0CH

                .org    0000H
RST00           JP      INIT

                .ORG    0008H
RST08           JP      TXA

                .ORG    0010H
RST10           JP      RXA

                .ORG    0018H
RST18           JP      RXCHK

                .ORG    0038H
RST38           RET

;--------
TXA             PUSH    AF
TXLOOP          IN      A, (UARTST)
                BIT     1, A
                JR      Z, TXLOOP
                POP     AF
                OUT     (UART), A
                RET

;--------
RXA             IN      A, (UARTST)
                BIT     0, A
                JR      Z, RXA
                IN      A, (UART)
                RET

;--------
RXCHK           IN      A, (UARTST)
                BIT     0, A
                RET

;--------
PRINT           LD      A, (HL)
                OR      A
                RET     Z
                RST     08H             ; TXA
                INC     HL
                JR      PRINT
                RET

;--------
INIT            DI
                LD      HL, TEMPSTACK
                LD      SP, HL
                XOR     A
                LD      HL, SIGMON1
                CALL    PRINT
                LD      A, (BASICSTART)
                CP      'Y'
                JR      NZ, COLDSTART
                LD      HL, SIGMON2
                CALL    PRINT
CORW            CALL    RXA
                AND     %11011111       ; make uppercase
                CP      'C'
                JR      NZ, CHECKWARM
                RST     08H
                LD      A, $0D
                RST     08H
                LD      A, $0A
                RST     08H
COLDSTART       LD      A, 'Y'
                LD      (BASICSTART), A
                JP      $0150           ; cold start
CHECKWARM       CP      'W'
                JR      NZ, CORW
                RST     08H
                LD      A, $0D
                RST     08H
                LD      A, $0A
                RST     08H
                JP      $0153           ; warm start

SIGMON1         .byte   CS
                .byte   "Initializing sbcz80cpm...", CR, LF, 0
SIGMON2         .byte   CR, LF
                .byte   "Cold or Warm start (C or W)?", 0

                .org    14FH
ENDOFAREA:
                .byte   0FFH
                .end