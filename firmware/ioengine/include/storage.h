#ifndef __STORAGE_H__
#define __STORAGE_H__

#include "ioengine.h"

class Storage {
public:
    Storage();
    ~Storage();
    void setDebug(bool debug);

    void showMenu();
    bool selectMenu(uint8_t select);

    uint64_t openBoot1st();
    uint64_t openBoot2nd();
    uint64_t openBootImage();
    uint64_t bootImageSize();

    int sectorSize();
    void setSectorSize(int size);
    int sectorsInTrack();
    void setSectorsInTrack(int sectors);
    int tracksInDisk();
    void setTracksInDisk(int tracks);
    uint8_t disk();
    void setDisk(uint8_t disk);
    uint8_t track();
    void setTrack(uint8_t track);
    uint8_t sector();
    void setSector(uint8_t sector);

    // for boot sequence
    uint8_t readByteToBoot(bool *ok);
    // for CP/M I/F
    uint8_t readByte(bool *ok);
    int writeByte(uint8_t data);

private:
    class Private;
    Private *d;
};

#endif // __STORAGE_H__