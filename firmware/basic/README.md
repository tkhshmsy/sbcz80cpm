# NASCOM BASIC v4.7b

forked from [Microsoft BASIC 4.7b](http://searle.x10host.com/z80/sbc_NascomBasic.zip) of Grant Searle's project.  

* BASIC interpreter for Z80
* BIOS.ASM is customized for sbcz80cpm
  * use UART for console Input/Output

```
Initializing sbcz80cpm...

Memory top? 
Z80 BASIC Ver 4.7b
Copyright (C) 1978 by Microsoft
56958 Bytes free
Ok

10 FOR I=0 TO 10
20 PRINT "Hello world!"
30 NEXT
run
Hello world!
Hello world!
Hello world!
Hello world!
Hello world!
Hello world!
Hello world!
Hello world!
Hello world!
Hello world!
Hello world!
Ok
```

## Requirements
* 'zasm' assembler
  * [zasm](https://k1.spdns.de/Develop/Projects/zasm/Distributions/)

## Build
* download 'basic.asm' from Grant's HomePage.
* execute 'make' command to get 'msbasic.bin'
```bash
/sbcz80cpm/firmware/basic$ ls
Makefile  README.md  basic.asm  init.asm

/sbcz80cpm/firmware/basic$ make
zasm -wuyb init.asm -o init.obj
assembled file: init.asm
    107 lines, 2 passes, 0.0015 sec.
    no errors

zasm -wuyb basic.asm -o basic.obj
assembled file: basic.asm
    4341 lines, 2 passes, 0.0261 sec.
    no errors

cat init.obj basic.obj > msbasic.bin

/sbcz80cpm/firmware/basic$ ls
Makefile  README.md  basic.asm  basic.lst  basic.obj  init.asm  init.lst  init.obj  msbasic.bin
```

### Setup

1. Copy `msbasic.bin` into `/basic` directory in SDcard.
2. Set up `/boot.cfg`
   1. set `/boot2nd.bin` as 2nd boot loader
      1. the general 2nd boot loader loads too many bytes, but it's safe.
   2. set `/basic/msbasic.bin` as main image
   3. set `/basic` as prefix to access disk images
      1. this is not neccessary, but recommended to avoid conflicts.
3. then, restart sbcz80cpm, and select above setting.

## References
* [Grant's homebuilt electronics](http://searle.x10host.com/)
  * [Grant's 7-chip Z80 computer](http://searle.x10host.com/z80/SimpleZ80.html)
