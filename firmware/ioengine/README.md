# I/O Engine for sbcz80cpm

the I/O device for sbcz80cpm powered by [ATMEGA4809](https://www.microchip.com/en-us/product/atmega4809).

## Features
* Boot control
  * provides IPL for no ROM / full RAM system
* I/O control, mostly for CP/M
  * Console I/O which is connected to host via UART
  * Disk I/O
  * RTC
  * communicate with I2C devices

## Design

### Connections
* for devices
  * UART
    * for host communication, 115200bps 8N1
  * I2C
    * RTC
    * other i2c devices
  * SPI
    * SD card, formatted VFAT/FAT32 
* for Z80
  * /WAIT, /IOREQ
  * /WR
  * /RD
  * AddressBus
  * DataBus
  * /MREQ

### Boot sequence and IPL
_sbcz80cpm_ is all RAM system, and has no ROM. Therefore, the IPL should be written into RAM by any way.
For this, at booting, _I/O Engine_ turns _Z80_ into bus master and writes IPL to the top of RAM.
However, as the _I/O Engine_(ATMEGA4809) has not enough I/O ports for address bus, the IPL needs to execute within a limited address space.

1. reset _Z80_
2. Loading _IPL_ (`/boot1st.bin`)
   1. assert /BUSREQ = 0
   2. wait until /BUSACK = 0 to enter BusMaster
   3. assert /BOOT = 0
      1. A15-8,A6-4 is 0
      2. /MREQ is 0 (SRAM is activated)
   4. assert A7 = 0
   5. using /WR, A0-3, write IPL from 0x0000 to 0x000F
   6. release /BOOT = 1
   7. release /BUSREQ = 1 to leave BusMaster
3. reset _Z80_, start _IPL_ from 0x0000
4. (_I/O Engine_ starts main loop)
5. _IPL_ (`/boot1st.bin`) is executed
   1. read 256 times from I/O port 0x8F
      1. I/O Engine returns binary of _2nd boot loader_
   2. write each bytes from 0xFF00 to 0xFFFF.
   3. jump to 0xFF00
6. _2nd boot loader_ (e.g. `/boot2nd.bin`) is executed
   1. read from I/O port 0x8F
      1. I/O Engine returns binary of _main image_
   2. write each bytes from 0x0000 to 0x7FFF.
   3. jump to 0x0000

At here, the _IPL_ (boot1st.bin) has to be less than 16 bytes of code. Which is quite possible with the extra instructions of _Z80_.
Similarly, the _2nd Boot Loader_ has also to be less than 256 bytes of code, but for the Z80, 256 bytes is enough space to perform various operations.

### I/O access sequence
The _I/O Engine_ needs to respond to IN/OUT instructions of _Z80_.
However, even though the _I/O Engine_ is running at 20MHz, it is still too slow to respond to the _Z80_ without WAITing.

So, it is solved this in the same way as other similar projects (e.g. [Z80MBC2](https://github.com/SuperFabius/Z80-MBC2), [Z80MBC3](https://github.com/eprive/Z80-MBC3)).

1. _I/O Engine_ waits until /WAIT = 0
   1. _Z80_ executes the instruction 'OUT'or 'IN', so assert /IOREQ = 0
   2. because /IOREQ connected to /WAIT with resistor, it turns /WAIT = 0
   3. IN/OUT command-cycle is 'T1, T2, TW, T3', _Z80_ waits until /WAIT =1.
      1. refer to [Z80 CPU User Manual](https://www.zilog.com/docs/z80/um0080.pdf) page 11
      2. in TW cycle, /RD,/WR, Address and Data is already asserted.
2. _I/O Engine_ reads /WR, /RD, Address and Data bus to execute it's function.
3. _I/O Engine_ releases bus,
   1. asserts /BUSREQ = 0
   2. release Data bus if the case that _I/O Engine_ was output any data.
   3. asserts /WAIT = 1 as output port (at here, still /IOREQ = 0 yet)
      1. _Z80_ start to continues executing the T3 cycle and the next instruction.
      2. But because /BUSREQ = 0, _Z80_ waits at the head of T1 in M1 cycle of the next instruction.
         1.  refer to [Z80 CPU User Manual](https://www.zilog.com/docs/z80/um0080.pdf) page 12
   4. _Z80_ is released its buses and waits until /BUSACK = 0.
   5. release /WAIT = High-Z as input port.
   6. asserts /BUSREQ = 1 to release all buses.
4. _Z80_ continues to next cycle...

## Boot configuration
At booting, _I/O Engine_ read `/boot.cfg` from SD card.
`boot.cfg` is written in the following TSV (Tab Separated Value) format and contains the following items.

* (1) the title of the setting
  * e.g. `CP/M 2.2`
* (2) the full-path of 2nd boot loader. 
  * Normally `/boot2nd.bin` is used, but a modified one is able to use.
  * e.g. `/cpm22/cpm22ipl.bin`
* (3) the full-path of main image
  * e.g. `/cpm22/bios.bin`
* (4) the prefix of the disk-image-files
  * e.g. `/cpm22`

The _prefix_ in the 4th item is used for creating _the image-filename_ of _the floppy-disk-emulation_ and is expanded as follows:
* `prefix` + `/disk` + \<disk no.\> + `.img`


## I/O port API
Mainly for CP/M, the _I/O Engine_ provides an API centered around FloppyDisk emulation.
It is placed under 8-bit addresses as a _Z80_'s I/O device.

About floppy-disk emulation, the default setting is set as _ibm-3740_ format as follows:
* max tracks: 77 (allows 0-76)
* max sectors: 26 (allows 0-25)
* sector length: 128 bytes

### for READ
| Address | Function | Description |
|---------|----------|-------------|
| 0x80    | UARTST   | the status of UART |
| 0x81    | UART     | receive from UART |
| 0x82    | SELDSK   | return the number of current Disk |
| 0x83    | SETTRK   | return the number of current track |
| 0x84    | SETSEC   | return the number of current sector |
| 0x85    | DISKIO   | read the sector, must to read sector length continuously |
| 0x86    | ----     | Not Implemented yet |
| 0x87    | ----     | Not Implemented yet |
| 0x88    | ----     | Not Implemented yet |
| 0x89    | ----     | Not Implemented yet |
| 0x8A    | RTC      | read RTC |
| 0x8B    | I2CTRANS | return the result of transmit |
| 0x8C    | I2CREQ   | return the result of available() of i2c |
| 0x8D    | I2CDATA  | read from i2c |
| 0x8E    | BANK     | return the current bank of SRAM |
| 0x8F    | misc output | return the binary data for IPL |

### for WRITE
| Address | Function | Description |
|---------|----------|-------------|
| 0x80    | UARTST   | No response |
| 0x81    | UART     | transmit to UART |
| 0x82    | SELDSK   | select the current disk |
| 0x83    | SETTRK   | set the current track |
| 0x84    | SETSEC   | set the current sector |
| 0x85    | DISKIO   | write the sector, must to write sector length continuously |
| 0x86    | ----     | Not Implemented yet |
| 0x87    | ----     | Not Implemented yet |
| 0x88    | ----     | Not Implemented yet |
| 0x89    | ----     | Not Implemented yet |
| 0x8A    | RTC      | write RTC |
| 0x8B    | I2CTRANS | send transmit to i2c |
| 0x8C    | I2CREQ   | send request to i2c |
| 0x8D    | I2CDATA  | write into i2c |
| 0x8E    | BANK     | set the current bank of SRAM |
| 0x8F    | misc input ||

misc input
| value | description |
|-------|-------------|
| 0x00  | set the dummy data 0x00 |
| 0x01  | set the binary of fixed _1st boot loader_ |
| 0x02  | set the binary of selected _2nd boot loader_ |
| 0x03  | set the binary of selected _main image_ |
| 0xFF  | reset _Z80_ |

## References
* Zilog
  * [Z80 CPU User Manual](https://www.zilog.com/docs/z80/um0080.pdf)
* Microchip ATMEGA4809
  * [ATMEGA4809](https://www.microchip.com/en-us/product/atmega4809)
  * [Datasheet](https://ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/DataSheets/ATmega4808-09-DataSheet-DS40002173C.pdf)
* Other Project
  * [Z80MBC2](https://github.com/SuperFabius/Z80-MBC2)
  * [Z80MBC3](https://github.com/eprive/Z80-MBC3)

## License
* MIT

## Author
* [tkhshmsy@gmail.com](tkhshmsy@gmail.com)
