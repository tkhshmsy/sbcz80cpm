# CP/M ver 2.2

62K CP/M ver 2.2 for sbcz80cpm

## Requirements
* 'zasm' assembler
  * [zasm](https://k1.spdns.de/Develop/Projects/zasm/Distributions/)
* 'cpmtools'
  * [Michael Haardt's cpmtools Main Page](http://www.moria.de/~michael/cpmtools/)
  * On Ubuntu, use `sudo apt install cpmtools`

## How to build
### BIOS and IPL
* bios.asm (bios.bin)
  * 62K CP/M ver 2.2 BIOS for sbcz80cpm
    * allocated at 0xF200
  * BOOT and WBOOT as CP/M API, loads system binary from 'disk0.img'
* cpm22ipl.asm (cpm22ipl.bin)
  * 2nd boot loader for sbcz80cpm
  * loads BIOS into 0xF200 and jump to 0xF200

```bash
/sbcz80cpm/firmware/cpm22$ ls
Makefile  README.md  bios.asm  cpm22ipl.asm

/sbcz80cpm/firmware/cpm22$ make
rm -f bios.bin cpm22ipl.bin *.lst *.hex
zasm -wuyb bios.asm -o bios.bin
assembled file: bios.asm
    368 lines, 2 passes, 0.0029 sec.
    no errors

zasm -wuyb cpm22ipl.asm -o cpm22ipl.bin
assembled file: cpm22ipl.asm
    57 lines, 2 passes, 0.0010 sec.
    no errors

/sbcz80cpm/firmware/cpm22$ ls
Makefile  README.md  bios.asm  bios.bin  bios.lst  cpm22ipl.asm  cpm22ipl.bin  cpm22ipl.lst
```
## How to set up system

### Get the system binary

1. Download OPERATING SYSTEMS binary from '[Unofficial CP/M Web site](http://www.cpm.z80.de/)'.
   * [CP/M 2.2 BINARY](http://www.cpm.z80.de/download/cpm22-b.zip)
2. Unpack to get the binary file 'CPM.SYS'.
   * 'CPM.SYS' includes CCP, BDOS, BIOS for 62K CP/M on Xerox1800 system.
3. Not using the BIOS for Xerox1800, so remove it and make CCP and BDOS binary.
   * `dd if=CPM.SYS of=ccpbdos.bin bs=128 count=44`
4. Make the system disk image at following step.

### Make the disk images

`bios.asm` is the 62K CP/M ver 2.2 BIOS for sbcz80cpm,
* supports _ibm-3740_ formatted disk image.
* supports 4 disks mounted.
  * `disk0.img`: system disk, must to install CCP and BDOS into boot track.
  * `disk1.img`: user disk (no needs the boot track)
  * `disk2.img`: user disk
  * `disk3.img`: user disk

> [!NOTE]
> ibm-3740 formatted disk image.
> * 77 tracks per disk
> * 26 sectors per track
> * 128 bytes per sector
> * 2 reserved tracks for system
> * skew factor is 6
> * Total: 256,256 bytes per disk

```bash
## System disk
### make empty disk image file (size = 128 * 26 * 77)
$ dd if=/dev/zero of=disk0.img bs=3328 count=77
### mkfs.cpm with ibm-3740 format and install the system binary into boot track
$ mkfs.cpm -f ibm-3740 -b ccpbdos.bin -L system disk0.img

## User disk (e.g. disk1.img)
$ dd if=/dev/zero of=disk1.img bs=3328 count=77
$ mkfs.cpm -f ibm-3740 disk1.img
```

To boot up CP/M, no files are needed but only the BIOS and the system binary in boot track.

### Control disk images
Using any other commands and files on CP/M, use cpmtools to deploy into disk images.

```bash
## list files in disk0.img
$ cpmls -f ibm-3740 disk0.img
## copy local file 'file.bin' into disk0.img (user 0)
$ cpmcp -f ibm-3740 disk0.img ./file.bin 0:
## copy file '0:file.bin' from disk0.img to local
$ cpmcp -f ibm-3740 disk0.img 0:file.bin ./
## remove '0:file.bin' from disk0.img
$ cpmrm -f ibm-3740 disk0.img 0:file.bin
```

> [!NOTE]
> _ibm-3740_ is the default format of cpmtools.
> so, `-f ibm-3740` option is able to be omitted.

### Setup sbcz80cpm

1. Copy `bios.bin` and `cpm22ipl.bin` into `/cpm22` directory in SDcard.
2. Copy disk images into `/cpm22` directory in SDcard.
3. Set up `/boot.cfg`
   1. set `/cpm22/cpm22ipl.bin` as 2nd boot loader
   2. set `/cpm22/bios.bin` as main image
   3. set `/cpm22` as prefix to access disk images
4. then, restart sbcz80cpm, and select above setting.
