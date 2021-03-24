//
// Created by user on 16.11.2020.
//

#ifndef LONOFF_MAIN_H
#define LONOFF_MAIN_H

#include "lan.h"
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <ModbusMaster.h>
//#include <ArduinoJson.h>
//#include <avr/eeprom.h>
//#include <EEPROM.h>

#define REQUEST_INTERVAL 5000
#define PIN_RX 8 //2
#define PIN_TX 9 //3
#define PIN_RE_DE 5
#define PIN_LOOP_CONNECT 6
#define A_FREQ_REGISTER_R_W 0x2001
#define A_STATE_REGISTER_R 0x2100
#define A_SET_STATE_REGISTER_W 0x2000
#define QUANTITY_REGISTER 01
#define STATE_OFF 5

void preTransmission();
void postTransmission();
static void initIO();
static void check_timer();
static void reConnect();
static void parseResp(String* reply);
static void callbackGETResponse(byte status, word off, word len);
String getValue(const String* data, char separator, int index);

#endif //LONOFF_MAIN_H
