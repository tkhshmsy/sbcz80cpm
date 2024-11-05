# Boot loaders for sbcz80cpm

## Contents
* Makefile
  * Makefile for all
* README.md
  * this document
* boot1st.asm
  * IPL for sbcz80cpm
    * load selected '2nd boot loader' into 0xff00, and jump to
* boot2nd.asm
  * general 2nd boot loader
    * load 32KB into 0x0000
* bootihex.asm
  * intel hex loader
    * load intel-hex formatted file
      * recommended to use 'slowtransfer.sh' because of no flow control.
    * after loaded, input 'StartAddress' to jump to
* slowtransfer.sh
  * script for 'slow' transfer
    * usage: `./slowtransfer.sh <device> <filename>`
    * send each bytes with interval
* test2nd.asm
  * test code of 2nd boot loader
    * output message and HALT
* testimg.asm
  * test code of main image
    * output message and HALT

## Requirements
* 'zasm' assembler
  * [zasm](https://k1.spdns.de/Develop/Projects/zasm/Distributions/)

## Build
* execute 'make' command

