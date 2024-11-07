# Example files for sbcz80cpm

## How to make SDCard

At first, make partition with the type `0c` (W95 FAT32 (LBA)).
```
$ sudo fdisk /dev/sda

Welcome to fdisk (util-linux 2.37.2).
Changes will remain in memory only, until you decide to write them.
Be careful before using the write command.

Command (m for help): p

Disk /dev/sda: 14.44 GiB, 15502147584 bytes, 30277632 sectors
Disk model: USB3.0 CRW   -SD
Units: sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
Disklabel type: dos
Disk identifier: 0x00000000

Device     Boot Start      End  Sectors  Size Id Type
/dev/sda1        2048 30277631 30275584 14.4G  c W95 FAT32 (LBA)
```
Next, make filesystem.
```bash
## make vfat/fat32 filesystem
$ sudo mkdosfs -F32 /dev/sda1
```

> [!NOTE]
> because of the implementation of SDFAT library,
> filename in SDCard is limited '8.3' format.

## Contents

Then, copy the files under 'sdcard' directory into the root of SDCard.

```text
sdcard/
|--boot.cfg
|--boot1st.bin
|--boot2nd.bin
|--bootihex.bin
|--basic/
|  |--msbasic.rom
|--cpm22/
|  |--bios.bin
|  |--ccpbdos.bin
|  |--cpm22ipl.bin
|  |--disk0.img
|  |--disk1.img
|  |--disk2.img
|  |--disk3.img
|--test/
   |--test2nd.bin
   |--testimg.bin
```

* Boot loaders
  * `/boot1st.bin`
    * IPL, 1st boot loader (code: 0x0000 to 0x0010)
    * loads '2nd boot loader' into 0xff00
    * after loaded, jump to 0xff00
  * `/boot2nd.bin`
    * general 2nd boot loader (code: 0xFF00 to 0xFFFF)
    * loads target binary into 0x0000
    * after loaded, jump to 0x0000
  * `/bootihex.bin`
    * Intel-Hex file loader
      * executes as 2nd boot loader
      * after loaded, input 'StartAddress' to jump to 
* [TEST] boot 2nd loader
  * for test
  * output message and HALT
* [TEST] main image boot
  * for test
  * output message and HALT
* NASCOM BASIC v4.7
  * refs : [../firmware/basic/README.md](../firmware/basic/README.md)
  * NASCOM BASIC v4.7 known as "Microsoft BASIC"
    * this codes are forked from [Grant's homebuilt electronics](http://searle.x10host.com/)
* CP/M 2.2
  * refs : [../firmware/cpm22/README.md](../firmware/cpm22/README.md)
  * use `/cpm22/cpm22ipl.bin` as 2nd boot loader
    * loads BIOS into 0xF200 
    * after loaded, jump to cold-BOOT(0xF200)
    * BIOS loads CCP and BDOS into 0xDC00 from boot-track of disk0.img
  * disk-images contain any CP/M 2.2 binaries from following sites
      * [Unofficial CP/M Web site](http://www.cpm.z80.de/)
        * [Digital Research Binary Files](http://www.cpm.z80.de/binary.html)
      * [the Retrocomputing Archive](http://www.retroarchive.org/)
        * [CP/M Programming Languages and Tools](http://www.retroarchive.org/cpm/lang/lang.htm)
        * [CP/M Games](http://www.retroarchive.org/cpm/games/games.htm)
        * [CPM/CDROM](http://www.retroarchive.org/cpm/cdrom/)
