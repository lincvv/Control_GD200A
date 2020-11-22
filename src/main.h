//
// Created by user on 16.11.2020.
//

#ifndef LONOFF_MAIN_H
#define LONOFF_MAIN_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <EtherCard.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>

#define REQUEST_INTERVAL 5000


void check_timer();
void set_state(uint8_t state);

#endif //LONOFF_MAIN_H
