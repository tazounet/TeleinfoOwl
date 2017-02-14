// Emulate the OWL CM180 Energy Monitor using "TeleInfo"

// rf433 lib: https://github.com/tazounet/OwlSender
// teleinfo lib: https://github.com/tazounet/TeleInfo

// serial debug
#define DEBUG 0

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include "OwlSender.h"
#include "TeleInfo.h"

const uint8_t txPin = 4; // digital pin connected to the RF transmitter
const uint8_t rxPin = 8; // teleinfo
const uint8_t ledPin = 13; // digital pin for LED

OwlSender sender;
TeleInfo info;

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
  info.setup(rxPin);
  pinMode(ledPin, OUTPUT);
}

void loop()
{
  boolean teleInfoReceived;
  int rtCons = 0;
  long accuCons = 0;

  teleInfoReceived = info.readTeleInfo();
  if (teleInfoReceived)
  {
    rtCons = info.getPAPP();
    accuCons = info.getBASE();

#if DEBUG
    info.displayTeleInfo();
#endif

    // send the data
    sender.send(rtCons, accuCons);

    blinkLed();

    // delay for 12 seconds
    delay(12000);
  }
  else
  {
 #if DEBUG
    Serial.println("No teleinfo received !");
 #endif
  }
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


