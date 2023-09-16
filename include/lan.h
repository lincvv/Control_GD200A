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

/**
 * @brief Initiates the connection to the Ethereum network.
 *
 * This function opens a connection to the Ethereum network and initializes the necessary components.
 * It establishes a communication channel with the network and prepares the system for interacting
 * with Ethereum contracts and performing transactions.
 *
 * @return void.
 */
void etherInit();
#endif //CONTROL_GD200A_LAN_H
