#include <Arduino.h>
#include "ioengine.h"
#include "z80cpu.h"
#include "storage.h"
#include "i2cdevice.h"

Z80CPU *z80 = nullptr;
Storage *storage = nullptr;
I2CDevice *i2cDevice = nullptr;
uint8_t max_bank = 0;
uint8_t boot_mode = 0;
uint64_t boot_image_size = 0;
uint64_t boot_image_offset = 0;
uint8_t boot_uptime_buffer[2];
uint8_t boot_uptime_offset = 0;
uint8_t rtc_buffer[6];
uint8_t rtc_offset = 0;
uint8_t i2c_result = 0;
bool debug = false;

void io_write(uint8_t address, uint8_t data);
uint8_t io_read(uint8_t address)
{
    if (debug) {
        Serial.println("[IOE] io_read: addr=" + String(address, HEX));
    }
    uint8_t result = 0x00;
    if (address == 0x80) {
        // UART status
        result = Serial.available() ? 0x01 : 0x00;
        result |= Serial.availableForWrite() ? 0x02 : 0x00;
    } else if (address == 0x81) {
        // UART receive
        while(Serial.available() <= 0) {
            delay(1);
        }
        result = Serial.read();
    } else if (address == 0x82) {
        // CPM/disk
        result = storage->disk();
    } else if (address == 0x83) {
        // CPM/track
        result = storage->track();
    } else if (address == 0x84) {
        // CPM/sector
        result = storage->sector();
    } else if (address == 0x85) {
        // CP/M read data (multi bytes)
        bool ok = false;
        result = storage->readByte(&ok);
        if (!ok) {
            Serial.println("[IOE] read error");
        }
    } else if (address == 0x86) {
        // not implemented yet
    } else if (address == 0x87) {
        // not implemented yet
    } else if (address == 0x88) {
        // not implemented yet
    } else if (address == 0x89) {
        // not implemented yet
    } else if (address == 0x8a) {
        // CP/M read rtc (6 bytes)
        if (rtc_offset == 0) {
            i2cDevice->readRTC(rtc_buffer);
        }
        result = rtc_buffer[rtc_offset++];
        if (rtc_offset >= 6) {
            rtc_offset = 0;
        }
    } else if (address == 0x8b) {
        // I2C/start/end transmit result
        result = i2c_result;
    } else if (address == 0x8c) {
        // I2C/available
        result = i2cDevice->available();
    } else if (address == 0x8d) {
        // I2C/read data
        result = i2cDevice->read();
    } else if (address == 0x8e) {
        // current bank
        result = z80->bank();
    } else if (address == 0x8f) {
        // misc output
        if (boot_mode != 0) {
            bool ok = false;
            result = storage->readByteToBoot(&ok);
            if (ok) {
                boot_image_offset++;
                if (boot_image_offset >= boot_image_size) {
                    Serial.println(" OK");
                    io_write(0x8f, 0x00);
                }
            }
        }
    }
    if (debug) {
        Serial.println("[IOE] io_read: data=" + String(result, HEX));
    }
    return result;
}

void io_write(uint8_t address, uint8_t data)
{
    if (debug) {
        Serial.println("[IOE] io_write: addr=" + String(address, HEX) + " data=" + String(data, HEX));
    }
    if (address == 0x80) {
        // not implemented yet
    } else if (address == 0x81) {
        // UART transmit
        Serial.write(data);
    } else if (address == 0x82) {
        // CPM/disk
        storage->setDisk(data);
    } else if (address == 0x83) {
        // CPM/track
        storage->setTrack(data);
    } else if (address == 0x84) {
        // CPM/sector
        storage->setSector(data);
    } else if (address == 0x85) {
        // CP/M write data (multi bytes)
        int s = storage->writeByte(data);
        if (s <= 0) {
            Serial.println("[IOE] write error");
        }
    } else if (address == 0x86) {
        // not implemented yet
    } else if (address == 0x87) {
        // not implemented yet
    } else if (address == 0x88) {
        // not implemented yet
    } else if (address == 0x89) {
        // not implemented yet
    } else if (address == 0x8a) {
        // CP/M write rtc (6 bytes)
        rtc_buffer[rtc_offset++] = data;
        if (rtc_offset >= 6) {
            i2cDevice->writeRTC(rtc_buffer);
            rtc_offset = 0;
        }
    } else if (address == 0x8b) {
        // I2C/transmit
        i2c_result = i2cDevice->transmit(data);
    } else if (address == 0x8c) {
        // I2C/request
        i2cDevice->request(data);
    } else if (address == 0x8d) {
        // I2C/write data
        i2cDevice->write(data);
    } else if (address == 0x8e) {
        // set bank
        z80->setBank(data);
    } else if (address == 0x8f) {
        // misc input
        if (data == 0x00) {
            boot_mode = 0;
            boot_image_offset = 0;
            boot_image_size = 0;
        } else if (data == 0x01) {
            boot_mode = 1;
            boot_image_offset = 0;
            boot_image_size = storage->openBoot1st();
            Serial.print("1stStage(" + String((long)boot_image_size) + "bytes): ");
        } else if (data == 0x02) {
            boot_mode = 2;
            boot_image_offset = 0;
            boot_image_size = storage->openBoot2nd();
            Serial.print("2ndStage(" + String((long)boot_image_size) + "bytes): ");
        } else if (data == 0x03) {
            boot_mode = 3;
            boot_image_offset = 0;
            boot_image_size = storage->openBootImage();
            Serial.print("bootImage(" + String((long)boot_image_size) + "bytes): ");
        } else if (data == 0xff) {
            Serial.println("[IOE] CPU Reset...");
            z80->reset();
        }
    }
}

void boot1stStage()
{
    z80->blockUntilBusMaster(true);
    z80->assertBootMode();

    io_write(0x8f, 0x01);
    for (uint8_t addr = 0; addr < boot_image_size; addr++) {
        z80->writeBusMaster(addr, io_read(0x8f));
    }

    z80->releaseBootMode();
    z80->releaseBusMaster();
    z80->reset();
}

void boot_message()
{
    Serial.println("\n\n");
    Serial.println("=================================");
    Serial.println(" sbcz80cpm I/O Engine ver. " EIO_VERSION);
    Serial.println("=================================");
    Serial.println();
}

void setup()
{
    Serial.begin(115200, SERIAL_8N1);
    boot_message();

    storage = new Storage();
    i2cDevice = new I2CDevice();
    z80 = new Z80CPU();

    max_bank = z80->memoryCheck();
    z80->assertReset();

    i2cDevice->search();
    i2cDevice->showRTC();
    Serial.println();

    storage->showMenu();
    i2cDevice->showMenu();
    while (1) {
        Serial.print("Select: ");
        while (Serial.available() <= 0) {
            delay(100);
        };
        char c = Serial.read();
        if ('0' <= c && c <= '9') {
            Serial.write(c);
            uint8_t index = c - '0';
            if (storage->selectMenu(index)){
                break;
            }
        } else if ('a' <= c && c <= 'z') {
            Serial.write(c);
            if (i2cDevice->selectMenu(c)) {
                break;
            }
        }
        Serial.println();
    }
    Serial.println();
    z80->releaseReset();

    boot1stStage();
}

void loop()
{
    z80->blockUntilWait();
    if (debug) {
        Serial.println(String("[IOE]")
                    + " addr=" + String(z80->address(), HEX)
                    + " data=" + String(z80->data(), HEX)
                    + " wr=" + String(z80->wr())
                    + " rd=" + String(z80->rd()));
    }
    if ((z80->address() & 0x80) != 0x00) {
        if (z80->rd()) {
            uint8_t data = io_read(z80->address());
            z80->assertDataBus(data);
        } else if (z80->wr()) {
            io_write(z80->address(), z80->data());
        } else {
            Serial.println("\n[IOE] invalid access");
        }
    }
    z80->assertBusReq();
    z80->prepareToReleaseWait();
    z80->blockUntilBusAck();
    z80->releaseWait();
    z80->releaseDataBus();
    z80->releaseBusReq();
}