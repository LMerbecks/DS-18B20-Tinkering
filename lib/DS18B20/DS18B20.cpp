#include <DS18B20.h>

DS18B20::DS18B20(int64_t address, OneWire& sensorOneWireBus){
    this->address = address;
    this->sensorOneWireBus = sensorOneWireBus;
    this->conversionTriggered = false;
}

float DS18B20::getTemperatureCelsius(){
    byte status = sensorOneWireBus.read_bit();
    if(conversionTriggered && status == 0){
        return;
    } 
    if(conversionTriggered && status == 1){
        conversionTriggered = false;
    }
}