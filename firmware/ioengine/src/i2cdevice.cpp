#include "i2cdevice.h"

#include <Arduino.h>
#include <Wire.h>
#include <DS3232RTC.h>

class I2CDevice::Private {
public:
    uint8_t transmittingAddress = 0x00;
    DS3232RTC *rtc = nullptr;
    bool debug = false;
};

I2CDevice::I2CDevice()
    : d{new Private}
{
    Wire.begin();
}

I2CDevice::~I2CDevice()
{
    delete d;
}

void I2CDevice::setDebug(bool debug)
{
    d->debug = debug;
}

void I2CDevice::showMenu()
{
    if (d->rtc) {
        Serial.println("d: RTC setting");
    }
}

bool I2CDevice::selectMenu(uint8_t select)
{
    if (select == 'd') {
        if (d->rtc) {
            Serial.println("RTC setting...");
            setRTC();
            return false; // no exit from menu
        } else {
            Serial.println("RTC is not available");
        }
    }
    return false;
}

void I2CDevice::search()
{
    Serial.println("I2C-BUS CHECK...");
    int count = 0;
    for (uint8_t address = 1; address < 127; address++) {
        if (isAlive(address)) {
            count++;
            Serial.print("found 0x");
            if (address < 16) {
                Serial.print("0");
            }
            Serial.print(address, HEX);
            if (address == 0x68) {
                Serial.print(" (DS3231, RTC)");
                d->rtc = new DS3232RTC(false);
            }
            Serial.println();
        }
    }
    Serial.println(String(count) + " devices are found.");
    Serial.println();
}

bool I2CDevice::isAlive(uint8_t address)
{
    Wire.beginTransmission(address);
    return Wire.endTransmission() == 0;
}

uint8_t I2CDevice::transmit(uint8_t address)
{
    if (d->transmittingAddress == 0x00) {
        if (address != 0x00) {
            d->transmittingAddress = address;
            Wire.beginTransmission(address);
        }
        return 0;
    }

    if (address != 0x00) {
        Serial.println("I2C is already transmitting " + String(d->transmittingAddress, HEX) + " but start " + String(address, HEX));
        Wire.endTransmission();
        d->transmittingAddress = address;
        Wire.beginTransmission(address);
        return 0;
    }

    d->transmittingAddress = address;
    return Wire.endTransmission();
}

void I2CDevice::request(uint8_t quantity)
{
    Wire.requestFrom(d->transmittingAddress, quantity);
}

uint8_t I2CDevice::available()
{
    return Wire.available();
}

void I2CDevice::write(uint8_t data)
{
    Wire.write(data);
}

uint8_t I2CDevice::read()
{
    return Wire.read();
}

void I2CDevice::showRTC()
{
    if (!d->rtc) {
        Serial.println("RTC is not available");
        return;
    }
    uint8_t buf[6];
    readRTC(buf);
    String dt = String((int)buf[0] + 1970, DEC) + "/" + String(buf[1], DEC) + "/" + String(buf[2], DEC);
    dt += " ";
    dt += String(buf[3], DEC) + ":" + String(buf[4], DEC) + ":" + String(buf[5], DEC);
    Serial.println("RTC: " + dt);
}

void I2CDevice::setRTC()
{
    if (!d->rtc) {
        Serial.println("RTC is not available");
        return;
    }

    uint8_t buf[15];
    uint8_t dt[6];
    bool valid = false;
    Serial.setTimeout(1000 * 10);
    Serial.print("Set DateTime (YYYYMMDDhhmmss): ");
    Serial.readBytes(buf, 14);
    for (int i = 0; i < 14; i++) {
        if (buf[i] < '0' || '9' < buf[i]) {
            valid = false;
            break;
        }
        valid = true;
    }
    buf[15] = 0x00;
    Serial.println(String((char *)buf));

    if (valid) {
        for (int i = 0; i < 14; i++) {
            buf[i] -= '0';
        }
        dt[0] = (uint8_t)((int)(buf[0] * 1000 + buf[1] * 100 + buf[2] * 10 + buf[3]) - 2000);
        dt[1] = (uint8_t)(buf[4] * 10 + buf[5]);
        dt[2] = (uint8_t)(buf[6] * 10 + buf[7]);
        dt[3] = (uint8_t)(buf[8] * 10 + buf[9]);
        dt[4] = (uint8_t)(buf[10] * 10 + buf[11]);
        dt[5] = (uint8_t)(buf[12] * 10 + buf[13]);
        if (d->debug) {
            Serial.println(String("DT=") + String(dt[0], DEC) + "/" + String(dt[1], DEC) + "/" + String(dt[2], DEC) + " " + String(dt[3], DEC) + ":" + String(dt[4], DEC) + ":" + String(dt[5], DEC));
        }
        writeRTC(dt);
        Serial.println("RTC is set.");
    }
    showRTC();
}

void I2CDevice::readRTC(uint8_t *buffer)
{
    if (!d->rtc) {
        Serial.println("RTC is not available");
        return;
    }
    tmElements_t tm;
    d->rtc->read(tm);
    buffer[0] = tm.Year;
    buffer[1] = tm.Month;
    buffer[2] = tm.Day;
    buffer[3] = tm.Hour;
    buffer[4] = tm.Minute;
    buffer[5] = tm.Second;
}

void I2CDevice::writeRTC(uint8_t *buffer)
{
    if (!d->rtc) {
        Serial.println("RTC is not available");
        return;
    }
    setTime(buffer[3], buffer[4], buffer[5], buffer[2],buffer[1], buffer[0]);
    d->rtc->set(now());
}
