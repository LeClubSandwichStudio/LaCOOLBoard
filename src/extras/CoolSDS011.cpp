#include "CoolSDS011.h"
#include <Arduino.h>
#include <Wire.h>


SDS011::SDS011(){}

bool SDS011::start(){
  Wire.beginTransmission(SDS011_ADRESS);
  Wire.write(SDS011_START);
  Wire.endTransmission();
  state = true;
  return true;
}

bool SDS011::stop(){
  Wire.beginTransmission(SDS011_ADRESS);
  Wire.write(SDS011_STOP);
  Wire.endTransmission();
  state = false;
  return true;
}

bool SDS011::read(){
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  char temp10[5];
  char temp25[5];
  int16_t ftemp10 = -1;
  int16_t ftemp25 = -1;

  Wire.beginTransmission(SDS011_ADRESS);
  Wire.write(SDS011_QUERY);
  Wire.endTransmission();
  delay(200);
  Wire.requestFrom(SDS011_ADRESS, 8);
  delay(200);
  while (Wire.available() >= 8) { 
    temp10[0] = Wire.read();
    temp10[1] = Wire.read();
    temp10[2] = Wire.read();
    temp10[3] = Wire.read();
    temp10[4] = '\0';
    temp25[0] = Wire.read();
    temp25[1] = Wire.read();
    temp25[2] = Wire.read();
    temp25[3] = Wire.read();
    temp25[4] = '\0';
    ftemp10 = atoi(temp10);
    ftemp25 = atoi(temp25);
    Serial.println("RAW Wire from SDS    " + String(ftemp10) + "    " + String(ftemp25));
    lastPM10 = float(ftemp10) / 10.0;
    lastPM25 = float(ftemp25) / 10.0;
    return true;
  }
  return false;
}

float SDS011::pm10() {
  read();
  return lastPM10;
}

float SDS011::pm25() {
  read();
  return lastPM25;
}