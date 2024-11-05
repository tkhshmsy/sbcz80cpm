#ifndef __I2CDEVICE_H__
#define __I2CDEVICE_H__

#include "ioengine.h"

class I2CDevice {
public:
    I2CDevice();
    ~I2CDevice();
    void setDebug(bool debug);

    void showMenu();
    bool selectMenu(uint8_t select);

    void search();
    bool isAlive(uint8_t address);

    // raw access
    uint8_t transmit(uint8_t address);
    void request(uint8_t quantity);
    uint8_t available();
    void write(uint8_t data);
    uint8_t read();

    // for RTC
    void showRTC();
    void setRTC();
    void readRTC(uint8_t *buffer);
    void writeRTC(uint8_t *buffer);

private:
    class Private;
    Private *d;
};

#endif // __I2CDEVICE_H__