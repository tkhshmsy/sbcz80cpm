#include "storage.h"

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

// FILE_WRITE includes "O_APPEND",  NO NEED !
#define FILE_READWRITE (O_READ | O_WRITE)

class Storage::Private {
public:
    void resetOffset();
    bool changeWriteMode(bool write);

    int numOfBootImages = 0;
    String boot2ndFilename;
    String prefix;
    String filename;
    uint64_t fileSize = 0;
    File file;

    int sectorSize = 128;
    int sectorsInTrack = 26;
    int tracksInDisk = 77;
    uint8_t disk = 0;
    uint8_t track = 0;
    uint8_t sector = 0;

    uint64_t offsetToSector = 0;
    int offset = 0;
    uint8_t sectorBuffer[2048];
    bool isWriting = false;
    bool debug = false;
};

void Storage::Private::resetOffset()
{
    offsetToSector = (((uint64_t)track * (uint64_t)sectorsInTrack) + (uint64_t)sector) * (uint64_t)sectorSize;
    offset = 0;
    if (file) {
        file.seek(offsetToSector);
    }
    if (debug) {
        Serial.println("resetOffset: " + String(track) + ":" + String(sector) + " -> " + String((long)offsetToSector, DEC));
    }
}

bool Storage::Private::changeWriteMode(bool write)
{
    if (!file) {
        file = SD.open(filename, FILE_READWRITE);
        if (!file) {
            Serial.println("[FAIL] file not valid:" + filename);
            return false;
        }
    }
    if (isWriting != write) {
        isWriting = write;
        resetOffset();
    }
    return true;
}

Storage::Storage()
: d{new Private}
{
    pinMode(SPI_SS, OUTPUT);
    bool sd = SD.begin(SPI_SS);
    Serial.println("SD with SPI: " + String(sd ? "OK" : "NG"));
    Serial.println();
}

Storage::~Storage()
{
    delete d;
}

void Storage::setDebug(bool debug)
{
    d->debug = debug;
}

void Storage::showMenu()
{
    Serial.println("BOOT IMAGES:");
    d->numOfBootImages = 0;

    File bootCfg = SD.open("/boot.cfg", FILE_READ);
    if(!bootCfg) {
        Serial.println("[FAIL] unable to open /boot.cfg");
        while(1) {
            delay(100);
        }
    }

    while (bootCfg.available()) {
        String line = bootCfg.readStringUntil('\n');
        if (line.startsWith("#")) {
            continue;
        }
        int tail = line.indexOf('\t');
        if (tail != -1) {
            String name = line.substring(0, tail);
            String buf = String(d->numOfBootImages) + ": " + name;
            Serial.println(buf);
            d->numOfBootImages++;
        }
    }
    bootCfg.close();
}

bool Storage::selectMenu(uint8_t select)
{
    bool result = false;
    File bootCfg = SD.open("/boot.cfg", FILE_READ);
    if(!bootCfg) {
        Serial.println("[FAIL] unable to open /boot.cfg");
        return false;
    }

    int index = 0;
    while (bootCfg.available()) {
        // boot.cfg format (TSV)
        // name, 2nd-boot-loader-filename, boot-image-filename, prefix
        // head '#' means the comment line.
        String line = bootCfg.readStringUntil('\n');
        if (line.startsWith("#")) {
            continue;
        }
        if (select != index) {
            index++;
            continue;
        }

        int head = 0;
        int tail = line.indexOf('\t');
        if (tail <= 0) {
            index++;
            continue;
        }
        String name = line.substring(head, tail);
        head = tail + 1;
        tail = line.indexOf('\t', head);
        if (tail <= 0) {
            index++;
            continue;
        }
        d->boot2ndFilename = line.substring(head, tail);
        head = tail + 1;
        tail = line.indexOf('\t', head);
        if (tail <= 0) {
            index++;
            continue;
        }
        d->filename = line.substring(head, tail);
        head = tail + 1;
        d->prefix = line.substring(head);
        if (SD.exists(d->boot2ndFilename) == 0) {
            Serial.println("[FAIL] file not found: " + d->boot2ndFilename);
            break;
        }
        if (SD.exists(d->filename) == 0) {
            Serial.println("[FAIL] file not found: " + d->filename);
            break;
        }
        Serial.println();
        Serial.println("FOUND: " + name);
        result = true;
        break;
    }
    bootCfg.close();
    return result;
}

uint64_t Storage::openBoot1st()
{
    if (d->file) {
        d->file.close();
    }
    d->file = SD.open("/boot1st.bin", FILE_READ);
    if (!d->file) {
        Serial.println("[FAIL] unable to open /boot1st.bin");
        return -1;
    }
    d->fileSize = d->file.size();
    return d->fileSize;
}

uint64_t Storage::openBoot2nd()
{
    if (d->file) {
        d->file.close();
    }
    d->file = SD.open(d->boot2ndFilename, FILE_READ);
    if (!d->file) {
        Serial.println("[FAIL] unable to open " + d->boot2ndFilename);
        return -1;
    }
    d->fileSize = d->file.size();
    return d->fileSize;
}

uint64_t Storage::openBootImage()
{
    if (d->file) {
        d->file.close();
    }
    d->file = SD.open(d->filename, FILE_READ);
    if (!d->file) {
        Serial.println("[FAIL] unable to open " + d->filename);
        return -1;
    }
    d->fileSize = d->file.size();
    return d->fileSize;
}

uint64_t Storage::bootImageSize()
{
    return d->fileSize;
}

int Storage::sectorSize()
{
    return d->sectorSize;
}

void Storage::setSectorSize(int size)
{
    if (size > 512) {
        size = 512;
    }
    d->sectorSize = size;
}

int Storage::sectorsInTrack()
{
    return d->sectorsInTrack;
}

void Storage::setSectorsInTrack(int sectors)
{
    d->sectorsInTrack = sectors;
}

int Storage::tracksInDisk()
{
    return d->tracksInDisk;
}

void Storage::setTracksInDisk(int tracks)
{
    d->tracksInDisk = tracks;
}

uint8_t Storage::disk()
{
    return d->disk;
}

void Storage::setDisk(uint8_t disk)
{
    if (d->file) {
        d->file.close();
    }
    d->disk = disk;
    d->filename = d->prefix + "/disk" + String(disk, DEC) + ".img";
    d->changeWriteMode(false);
    d->fileSize = d->file.size();
    if (d->debug) {
        Serial.println("set disk: " + String(disk) + " -> " + d->filename + " (" + String((long)d->fileSize) + ")");
    }
}

uint8_t Storage::track()
{
    return d->track;
}

void Storage::setTrack(uint8_t track)
{
    d->track = track;
    if (d->debug) {
        Serial.println("set track: " + String(track));
    }
    d->resetOffset();
}

uint8_t Storage::sector()
{
    return d->sector;
}

void Storage::setSector(uint8_t sector)
{
    d->sector = sector;
    if (d->debug) {
        Serial.println("set sector: " + String(sector));
    }
    d->resetOffset();
}

uint8_t Storage::readByteToBoot(bool *ok)
{
    if (!d->file || !d->file.available()) {
        Serial.println("[FAIL] file not available:" + d->filename);
        *ok = false;
        return 0x00;
    }
    *ok = true;
    return d->file.read();
}

uint8_t Storage::readByte(bool *ok)
{
    d->changeWriteMode(false);
    if (d->offset >= d->sectorSize) {
        d->resetOffset();
    }
    if (d->offset == 0) {
        d->file.read(d->sectorBuffer, d->sectorSize);
    }
    *ok = true;
    return d->sectorBuffer[d->offset++];
}

int Storage::writeByte(uint8_t data)
{
    d->changeWriteMode(true);
    if (d->offset >= d->sectorSize) {
        d->resetOffset();
    }
    d->sectorBuffer[d->offset++] = data;
    if (d->offset >= d->sectorSize) {
        d->file.write(d->sectorBuffer, d->sectorSize);
        d->file.flush();
    }
    return 1;
}
