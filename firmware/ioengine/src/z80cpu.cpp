#include "z80cpu.h"

#include <Arduino.h>

class Z80CPU::Private
{
public:
    uint8_t bank = 0;
    uint8_t max_bank = 0;
    bool isBusMaster = false;
    bool isBootMode = false;
    bool debug = false;
};

Z80CPU::Z80CPU()
    : d{new Private}
{
    pinMode(Z80_RST, OUTPUT);
    releaseReset();
    //pinMode(Z80_WAIT, INPUT);
    releaseWait();
    pinMode(Z80_BUSREQ, OUTPUT);
    releaseBusReq();
    pinMode(Z80_BUSACK, INPUT_PULLUP);

    pinMode(BOOT_SEL, OUTPUT);
    releaseBootMode();
    releaseAddressControl();
    releaseDataBus();

    BANKSEL.DIRSET = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm;
    d->isBusMaster = false;
    d->isBootMode = false;
}

Z80CPU::~Z80CPU()
{
    delete d;
}

void Z80CPU::setDebug(bool debug)
{
    d->debug = debug;
}

void Z80CPU::debug()
{
    if (d->debug) {
        Serial.println(String("[Z80CPU:Debug] ")
                    + "Rst:" + String(digitalRead(Z80_RST), BIN)
                    + " Wait:" + String(digitalRead(Z80_WAIT))
                    + " bREQ: " + String(digitalRead(Z80_BUSREQ), BIN)
                    + " bACK: " + String(digitalRead(Z80_BUSACK), BIN)
                    + " Bank: " + String(BANKSEL.IN, DEC)
                    + " WR: " + String(digitalRead(Z80_WR), BIN)
                    + " RD: " + String(digitalRead(Z80_RD), BIN)
                    + " Addr: " + String(Z80_ADDRESS.IN, BIN)
                    + " Data: " + String(Z80_DATA.IN, BIN));
    }
}

void Z80CPU::assertReset()
{
    if (d->debug) {
        Serial.println("assertReset");
    }
    digitalWrite(Z80_RST, LOW);
}

void Z80CPU::releaseReset()
{
    if (d->debug) {
        Serial.println("releaseReset");
    }
    digitalWrite(Z80_RST, HIGH);
}

void Z80CPU::reset()
{
    assertReset();
    delay(5);
    releaseReset();
}

void Z80CPU::blockUntilWait()
{
    if (d->debug) {
        Serial.println("blockUntilWait...");
    }
    while (digitalRead(Z80_WAIT) == HIGH){
        delay(1);
        if (d->debug) {
            Serial.print("wait..." + String(digitalRead(Z80_WAIT), BIN) + "\r");
        }
    };
    if (d->debug) {
        Serial.println("next");
    }
    PORTC.OUTCLR = PIN1_bm;
    pinMode(Z80_WAIT, OUTPUT);
    digitalWrite(Z80_WAIT, LOW);
}

void Z80CPU::prepareToReleaseWait()
{
    if (d->debug) {
        Serial.println("prepareToReleaseWait");
    }
    PORTC.OUTSET = PIN1_bm;
}

void Z80CPU::releaseWait()
{
    if (d->debug) {
        Serial.println("releaseWait");
    }
    pinMode(Z80_WAIT, INPUT);
    PORTC.OUTCLR = PIN1_bm;
}

void Z80CPU::assertBusReq()
{
    if (d->debug) {
        Serial.println("assertBusReq");
    }
    digitalWrite(Z80_BUSREQ, LOW);
}

void Z80CPU::blockUntilBusAck()
{
    if (d->debug) {
        Serial.println("blockUntilBusAck...");
    }
    while (digitalRead(Z80_BUSACK) == HIGH) {
        delay(1);
        if (d->debug) {
            Serial.print("wait..." + String(digitalRead(Z80_BUSACK), BIN) + "\r");
        }
    };
    if (d->debug) {
        Serial.println("next");
    }
}

void Z80CPU::releaseBusReq()
{
    if (d->debug) {
        Serial.println("releaseBusReq");
    }
    digitalWrite(Z80_BUSREQ, HIGH);
}

void Z80CPU::assertDataBus(uint8_t data)
{
    if (d->debug) {
        Serial.println("assertDataBus " + String(data, HEX));
    }
    Z80_DATA.DIRSET = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
    Z80_DATA.OUT = data;
}

void Z80CPU::releaseDataBus()
{
    if (d->debug) {
        Serial.println("releaseDataBus");
    }
    Z80_DATA.DIRCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm | PIN5_bm | PIN6_bm | PIN7_bm;
}

void Z80CPU::releaseAddressControl()
{
    if (d->debug) {
        Serial.println("releaseAddressControl");
    }
    pinMode(Z80_WR, INPUT_PULLUP);
    pinMode(Z80_RD, INPUT_PULLUP);
    Z80_ADDRESS.DIRCLR = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm;
}

void Z80CPU::blockUntilBusMaster(bool withReset)
{
    if (d->debug) {
        Serial.println("blockUntilBusMaster" + String(withReset ? " with reset" : ""));
    }
    if (withReset) {
        assertReset();
        assertBusReq();
        releaseReset();
    } else {
        assertBusReq();
    }
    assertBusReq();
    blockUntilBusAck();

    pinMode(Z80_WR, OUTPUT);
    digitalWrite(Z80_WR, HIGH);
    pinMode(Z80_RD, OUTPUT);
    digitalWrite(Z80_RD, HIGH);
    Z80_ADDRESS.DIRSET = PIN0_bm | PIN1_bm | PIN2_bm | PIN3_bm | PIN4_bm;
    d->isBusMaster = true;
}

void Z80CPU::releaseBusMaster()
{
    if (d->debug) {
        Serial.println("releaseBusMaster");
    }
    releaseAddressControl();
    releaseDataBus();
    releaseBusReq();
    d->isBusMaster = false;
}

void Z80CPU::assertBootMode()
{
    if (d->debug) {
        Serial.println("assertBootMode");
    }
    d->isBootMode = true;
    digitalWrite(BOOT_SEL, LOW);
}

void Z80CPU::releaseBootMode()
{
    if (d->debug) {
        Serial.println("releaseBootMode");
    }
    digitalWrite(BOOT_SEL, HIGH);
    d->isBootMode = false;
}

uint8_t Z80CPU::bank()
{
    return d->bank;
}

void Z80CPU::setBank(uint8_t bank)
{
    uint8_t b = BANKSEL.OUT;
    d->bank = bank & 0x0f;
    BANKSEL.OUT = (b & 0xf0) | d->bank;
}

uint8_t Z80CPU::readBusMaster(uint8_t address)
{
    if (d->debug) {
        Serial.println("readBusMaster " + String(address, HEX));
    }
    uint8_t addr = ((address & 0x80) >> 3) | (address & 0x0f);
    uint8_t out = Z80_ADDRESS.OUT;
    Z80_ADDRESS.OUT = (out & 0xe0) | (address & 0x1f);
    releaseDataBus();
    digitalWrite(Z80_RD, LOW);
    delay(1);
    uint8_t data = Z80_DATA.IN;
    digitalWrite(Z80_RD, HIGH);
    delay(1);
    return data;
}

void Z80CPU::writeBusMaster(uint8_t address, uint8_t data)
{
    if (d->debug) {
        Serial.println("writeBusMaster " + String(address, HEX) + " " + String(data, HEX));
    }
    uint8_t addr = ((address & 0x80) >> 3) | (address & 0x0f);
    uint8_t out = Z80_ADDRESS.OUT;
    Z80_ADDRESS.OUT = (out & 0xe0) | (address & 0x1f);
    digitalWrite(Z80_WR, LOW);
    assertDataBus(data);
    delay(1);
    digitalWrite(Z80_WR, HIGH);
    delay(1);
}

uint8_t Z80CPU::address()
{
    uint8_t addr = Z80_ADDRESS.IN;
    return ((addr & 0x10) << 3) | (addr & 0x0f);
}

uint8_t Z80CPU::data()
{
    return Z80_DATA.IN;
}

bool Z80CPU::wr()
{
    return digitalRead(Z80_WR) == LOW;
}

bool Z80CPU::rd()
{
    return digitalRead(Z80_RD) == LOW;
}

uint8_t Z80CPU::memoryCheck()
{
    blockUntilBusMaster(true);
    assertBootMode();

    Serial.println("MEMORY CHECK...");
    if (!d->isBusMaster || !d->isBootMode) {
        Serial.println("Z80CPU: Not in bus master mode or boot mode");
        return 0x0000;
    }

    uint8_t result = 0;
    uint8_t backup[16];
    uint8_t bankBackup = d->bank;
    uint8_t w = 0x00;
    for (int i = 0; i < 16; i++) {
        setBank(i);
        backup[i] = readBusMaster(0x00);
    }
    for (int i = 0; i < 16; i++) {
        setBank(i);
        writeBusMaster(0x00, 0x00);
    }
    for (int i = 0; i < 16; i++) {
        setBank(i);
        w = readBusMaster(0x00);
        if (w == 0xaa) {
            // already checked
            if (result > 0) {
                result -= 1;
            }
            break;
        }
        writeBusMaster(0x00, 0x55);
        w = readBusMaster(0x00);
        if (w != 0x55) {
            // failed to write 1
            if (result > 0) {
                result -= 1;
            }
            break;
        }
        writeBusMaster(0x00, 0xaa);
        w = readBusMaster(0x00);
        if (w != 0xaa) {
            // failed to write 2
            if (result > 0) {
                result -= 1;
            }
            break;
        }

        Serial.print("Bank " + String(i) + ": ");
        uint32_t address = i * 32768;
        String startAddress = String("00000" + String(address, HEX));
        startAddress = startAddress.substring(startAddress.length() - 5);
        startAddress.toUpperCase();
        String endAddress = String("00000" + String((address + 32767), HEX));
        endAddress = endAddress.substring(endAddress.length() - 5);
        endAddress.toUpperCase();
        Serial.print("0x" + startAddress + " - 0x" + endAddress);

        setBank(15);
        w = readBusMaster(0x00);
        if (w == 0xaa) {
            Serial.print(" (mapped to HIMEM)");
            break;
        }
        Serial.println();
        result += 1;
    }
    Serial.println();

    d->max_bank = result;
    Serial.println(String(result, DEC) + " banks, " + String(32 * result, DEC) + "KB + 32KB is available.");
    Serial.println();

    // restore backup
    for (int i = 0; i < 16; i++) {
        setBank(i);
        writeBusMaster(0x00, backup[i]);
    }
    setBank(bankBackup);

    releaseBootMode();
    releaseBusMaster();

    return result;
}
