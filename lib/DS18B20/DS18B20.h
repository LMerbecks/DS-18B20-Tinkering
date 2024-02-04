#ifndef DS18B20HEADDERGUARD
#define DS18B20HEADDERGUARD

#include <Arduino.h>
#include <OneWire.h>

/* SCRATCHPAD INFO*/
#define SCRATCHPAD_SIZE 9 
#define TEMPERATURE_LSB_ADDRESS 0 // byte with the lowest value
#define TEMPERATURE_MSB_ADDRESS 1 // byte with the largest value
#define TEMPERATUREALARM_HIGH_ADDRESS 2
#define TEMPERATUREALARM_HIGH_ADDRESS 3
#define CONFIGURATION_ADDRESS 4
#define CRC_ADDRESS 8 // CRC = cyclic redundancy check

#define CONFIGURATION_12_BIT_RES 0b01111111
#define CONFIGURATION_11_BIT_RES 0b01011111
#define CONFIGURATION_10_BIT_RES 0b00111111
#define CONFIGURATION_9_BIT_RES 0b0011111

#define COMMAND_CONVERT 0x44
#define COMMAND_WRITE_SCRATCHPAD 0x4E
#define COMMAND_READ_SCRATCHPAD 0xBE
#define COMMAND_COPY_SCRATCHPAD 0x48
#define COMMAND_RECALL_EEPROM 0xB8
#define COMMAND_READ_POWER_SUPPLY 0xB4


#define MASK_RES_11 ~(int) 0b1
#define MASK_RES_10 ~(int) 0b11
#define MASK_RES_9 ~(int) 0b111




class DS18B20
{
private:
    int64_t address;
    OneWire sensorOneWireBus;
    word temperatureRegister;
    byte sketchPadData[SCRATCHPAD_SIZE];
    bool conversionInProgress;
    bool conversionTriggered;
public:
    DS18B20(int64_t address, OneWire& sensorOneWireBus);
    ~DS18B20();

    void triggerADconversion();
    void copyScratchpad();
    void writeScratchpad();
    void readScratchpad();
    void recallEEPROM();
    void readPowerSupply();
    float getTemperatureCelsius();
    void setBitResolution();
    void getBitResolution();
    void setupForFunctionCommand(); // this should run the initialize and sendROMCommand preparing the sensor for function input
    void initialize(); // reset the bus
    void sendROMCommand(); // send address
    void monitorDeviceBusy(); // check if device is busy by reading one wire if the wire is 0 the device is still converting if its 1 its done
};




#endif