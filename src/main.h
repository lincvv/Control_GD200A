//
// Created by user on 16.11.2020.
//

#ifndef LONOFF_MAIN_H
#define LONOFF_MAIN_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <EtherCard.h>
#include <avr/wdt.h>
//#include <avr/eeprom.h>
//#include <EEPROM.h>

#define REQUEST_INTERVAL 5000
#define PIN_ON_INV 4
#define PIN_OFF_INV 7
#define PIN_LOOP_CONNECT 6

static void callback_response(byte status, word off, word len);
void check_timer();
void set_state(uint8_t state);
void etherInit();


#endif //LONOFF_MAIN_H
