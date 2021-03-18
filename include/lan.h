//
// Created by user on 04.03.2021.
//

#ifndef LONOFF_LAN_H
#define LONOFF_LAN_H
#include <EtherCard.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>

#define DEV

#define PIN_RX 8 //2
#define PIN_TX 9 //3

const char website[] PROGMEM = "fe.laboratory.co.il";

void etherInit();
#endif //LONOFF_LAN_H
