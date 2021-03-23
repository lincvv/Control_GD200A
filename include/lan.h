//
// Created by user on 04.03.2021.
//

#ifndef LONOFF_LAN_H
#define LONOFF_LAN_H
#include <EtherCard.h>
#include <avr/wdt.h>

const char website[] PROGMEM = "test.itlab.com.ua";

void etherInit();
void POSTRequest(int id, uint8_t ison, uint16_t freq);
void GetStatus();
#endif //LONOFF_LAN_H
