//
// Created by user on 04.03.2021.
//

#ifndef CONTROL_GD200A_LAN_H
#define CONTROL_GD200A_LAN_H
#include <EtherCard.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>

#define DEV

#define PIN_RX 8 //2
#define PIN_TX 9 //3

const char website[] PROGMEM = "MY SITE";

void etherInit();
#endif //CONTROL_GD200A_LAN_H
