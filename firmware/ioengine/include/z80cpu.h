#ifndef __Z80_CPU_H__
#define __Z80_CPU_H__

#include "ioengine.h"

class Z80CPU {
public:
    Z80CPU();
    ~Z80CPU();
    void setDebug(bool debug);
    void debug();

    void assertReset();
    void releaseReset();
    void reset();

    void blockUntilWait();
    void prepareToReleaseWait();
    void releaseWait();

    void assertBusReq();
    void blockUntilBusAck();
    void releaseBusReq();

    void assertDataBus(uint8_t data);
    void releaseDataBus();
    void releaseAddressControl();

    void blockUntilBusMaster(bool withReset = false);
    void blockUntilBusMasterWithReset();
    void assertBootMode();
    void releaseBootMode();
    void releaseBusMaster();

    uint8_t bank();
    void setBank(uint8_t bank);
    uint8_t readBusMaster(uint8_t address);
    void writeBusMaster(uint8_t address, uint8_t data);

    uint8_t address();
    uint8_t data();
    bool wr();
    bool rd();

    uint8_t memoryCheck();

private:
    class Private;
    Private *d;
};

#endif // __Z80_CPU_H__