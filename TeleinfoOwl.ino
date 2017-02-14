// Emulate the OWL CM180 Energy Monitor using "TeleInfo"

// rf433 lib: https://github.com/tazounet/OwlSender

// serial debug
#define DEBUG 1

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include "OwlSender.h"

const uint8_t txPin = 4; // digital pin connected to the RF transmitter
const uint8_t ledPin = 13; // digital pin for LED

OwlSender sender;

void setup()
{
  uint8_t sensorId;
  uint16_t crcRead;
  uint16_t crcComputed;

#if DEBUG
  Serial.begin(9600);
  Serial.println("Setup");
#endif

  // read config from eeprom
  EEPROM.get(0, sensorId);

  // read CRC
  EEPROM.get(sizeof(uint8_t), crcRead);

  // check CRC
  crcComputed = crc16((char*)&sensorId, sizeof(uint8_t));

#if DEBUG
    Serial.print("CRC read=");
    Serial.print(crcRead);
    Serial.print(", CRC computed=");
    Serial.println(crcComputed);
#endif

  if (crcComputed != crcRead)
  {
    randomSeed(analogRead(0));
    sensorId = random (0x0, 0xFF);

    // write config to eeprom
    EEPROM.put(0, sensorId);

    // write crc to eeprom
    crcComputed = crc16((char*)&sensorId, sizeof(uint8_t));
    EEPROM.put(sizeof(uint8_t), crcComputed);

#if DEBUG
    Serial.print("Write to EEPROM, Sensor Id=");
    Serial.println(sensorId);
#endif
  }
  else
  {
#if DEBUG
    Serial.print("Read from EEPROM, Sensor Id=");
    Serial.println(sensorId);
#endif
  }

  sender.setup(txPin, sensorId);
  pinMode(ledPin, OUTPUT);
}

void loop()
{
  int rtCons = 2500;
  long accuCons = 18000;

#if DEBUG
  Serial.print("rtCons: ");
  Serial.print(rtCons);
  Serial.print("W, accuCons: ");
  Serial.print(accuCons);
  Serial.println("Wh");
#endif

  // send the data
  sender.send(rtCons, accuCons);

  blinkLed();

  // delay for 12 seconds
  delay(12000);
}

void blinkLed()
{
  digitalWrite(ledPin, HIGH);
  delay(30);   
  digitalWrite(ledPin, LOW);
}

unsigned short crc16(char *msg, size_t size) {
  int i = 0;
  unsigned short crc = 0xffffU;

  while(size--)
  {
    crc ^= (unsigned short) *msg++ << 8;

    for(i = 0; i < 8; ++i)
      crc = crc << 1 ^ (crc & 0x8000U ? 0x1021U : 0U);
  }
  return(crc & 0xffffU);
}


