#include <Arduino.h>
#include <OneWire.h>

float convertWordToCelsiusTemperature(int16_t bitData);
float convertTemperatureCelsiusToFarenheit(float temperatureCelsius);
int64_t convertArrayAdressTo64BitAddress(byte* addr);
bool searchForAddress(byte* addressContainer);
void printAddress(byte* addressContainer);
bool throwCheckSumCheckResultError(byte* addressContainer);
byte printChipIdentifier(byte* addressContainer);
void triggerConversion(byte* addressContainer);
byte triggerScratchpadRead(byte* addressContainer);
void printReadData(byte* dataContainer, byte devicePresent);
float convertDataFromAddressToTemperatureCelsius(byte* dataContainer, byte* addressContainer, byte isTypeS);
void printTemperaturesCF(float temperatureCelsius, float temperatureFahrenheit);

OneWire sensorBus(D0);

byte i;
byte devicePresent = 0;
byte isTypeS;
byte data[9];
byte addr[8];
float temperatureCelsius, temperatureFahrenheit;

void setup() {
  Serial.begin(115200);
}

void loop() {
  if(!searchForAddress(addr)){
    return;
  }
  printAddress(addr);
  if(throwCheckSumCheckResultError(addr)){
    return;
  }
  isTypeS = printChipIdentifier(addr);
  if(isTypeS < 0) return;
  triggerConversion(addr);
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a sensorBus.depower() here, but the reset will take care of it.
  triggerScratchpadRead(addr);
  printReadData(data, devicePresent);
  temperatureCelsius = convertDataFromAddressToTemperatureCelsius(data, addr, isTypeS);
  temperatureFahrenheit = temperatureCelsius * 1.8 + 32.0;
  printTemperaturesCF(temperatureCelsius, temperatureFahrenheit);
}







float convertWordToCelsiusTemperature(int16_t bitData){
  float temperatureCelsius = (float)bitData / 16.0; 
  return temperatureCelsius;
};

float convertTemperatureCelsiusToFarenheit(float temperatureCelsius){
  float temperatureFahrenheit = temperatureCelsius * 1.8 + 32.0;
  return temperatureFahrenheit;
}

int64_t convertArrayAdressTo64BitAddress(byte* addr){
  int64_t adress64Bit = 0;
  for(byte k = 0; k < 8; k++){
    adress64Bit = (int64_t) (adress64Bit << 8) + addr[k];
  }
  return adress64Bit;
}

bool searchForAddress(byte* addressContainer){
  if ( !sensorBus.search(addressContainer)) {
    Serial.println("No more addresses.");
    Serial.println();
    sensorBus.reset_search();
    delay(250);
    return false;
  }
  return true;
}

void printAddress(byte* addressContainer){
  Serial.print("ROM =");  
  for(byte i = 0; i < 8; i++) {
    Serial.write(' ');
    Serial.print(addressContainer[i], HEX);
  }
}

bool throwCheckSumCheckResultError(byte* addressContainer){
  if (OneWire::crc8(addressContainer, 7) != addressContainer[7]) {
      Serial.println("CRC is not valid!");
      return true;
  }
  return false;
}

byte printChipIdentifier(byte* addressContainer){
  // the first ROM byte indicates which chip
  byte isTypeS = 0;
  switch (addressContainer[0]) {
    case 0x10:
      Serial.println("  Chip = DS18S20");  // or old DS1820
      isTypeS = 1;
      break;
    case 0x28:
      Serial.println("  Chip = DS18B20");
      isTypeS = 0;
      break;
    case 0x22:
      Serial.println("  Chip = DS1822");
      isTypeS = 0;
      break;
    default:
      Serial.println("Device is not a DS18x20 family device.");
      isTypeS = -1;
  } 
  return isTypeS;
}

void triggerConversion(byte* addressContainer){
  sensorBus.reset();
  sensorBus.select(addressContainer);
  sensorBus.write(0x44, 1);        // start conversion, with parasite power on at the end
}

byte triggerScratchpadRead(byte* addressContainer){
  byte devicePresent = sensorBus.reset();
  sensorBus.select(addressContainer);    
  sensorBus.write(0xBE);         // Read Scratchpad
  return devicePresent;
}

void printReadData(byte* dataContainer, byte devicePresent){
  Serial.print("  Data = ");
  Serial.print(devicePresent, HEX);
  Serial.print(" ");
  for (byte i = 0; i < 9; i++) {           // we need 9 bytes
    dataContainer[i] = sensorBus.read();
    Serial.print(dataContainer[i], HEX);
    Serial.print(" ");
  }
  Serial.print(" CRC=");
  Serial.print(OneWire::crc8(dataContainer, 8), HEX);
  Serial.println();
}

// Convert the data to actual temperature
// because the result is a 16 bit signed integer, it should
// be stored to an "int16_t" type, which is always 16 bits
// even when compiled on a 32 bit processor.
float convertDataFromAddressToTemperatureCelsius(byte* dataContainer, byte* addressContainer, byte isTypeS){
  int16_t raw = (dataContainer[1] << 8) | dataContainer[0];
  if (isTypeS) {
    raw = raw << 3; // 9 bit resolution default
    if (dataContainer[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - dataContainer[6];
    }
  } else {
    byte cfg = (dataContainer[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  float temperatureCelsius = convertWordToCelsiusTemperature(raw);
  return temperatureCelsius;
}

void printTemperaturesCF(float temperatureCelsius, float temperatureFahrenheit){
  Serial.print("  Temperature = ");
  Serial.print(temperatureCelsius);
  Serial.print(" Celsius, ");
  Serial.print(temperatureFahrenheit);
  Serial.println(" Fahrenheit");
  return;
}