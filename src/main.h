//
// Created by user on 16.11.2020.
//

#ifndef LONOFF_MAIN_H
#define LONOFF_MAIN_H

#include "lan.h"
#include <Arduino.h>
//#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>
//#include <avr/eeprom.h>
//#include <EEPROM.h>

#define REQUEST_INTERVAL 5000
//#define PIN_ON_INV 4
//#define PIN_OFF_INV 7
#define PIN_RX 8 //2
#define PIN_TX 9 //3
#define PIN_RE_DE 5
#define PIN_LOOP_CONNECT 6

void preTransmission();
void postTransmission();
static void initIO();
static void check_timer();
static void reConnect();
static void set_state(uint8_t state);
static void parseResp(String* reply);
static void callbackGETResponse(byte status, word off, word len);
String getValue(const String* data, char separator, int index);

#endif //LONOFF_MAIN_H
