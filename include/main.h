//
// Created by user on 16.11.2020.
//

#ifndef CONTROL_GD200A_MAIN_H
#define CONTROL_GD200A_MAIN_H

#include "lan.h"
#include <Arduino.h>
#include "ModbusMaster.h"
//#include <avr/eeprom.h>
//#include <EEPROM.h>

#define REQUEST_INTERVAL 5000
#define PIN_RE_DE 5
#define PIN_LOOP_CONNECT 6
#define A_FREQ_REGISTER_R_W 0x2001
#define A_STATE_REGISTER_R 0x2100
#define A_SET_STATE_REGISTER_W 0x2000
#define QUANTITY_REGISTER 01
#define STATE_OFF 5
//#define STATE_ON 1

/**
 * @brief Pre-transmission function.
 *
 * This function is called before the transmission of data. It performs the necessary pre-transmission operations
 * such as configuring the communication settings and preparing the data for transmission.
 *
 * @param None
 *
 * @return void.
 */
void preTransmission();

/**
 * @brief Performs necessary tasks after transmission.
 *
 * This function is called after the transmission of data to perform any required
 * post-transmission tasks. These tasks may include cleaning up resources, updating
 * internal state, or notifying other parts of the system about the completion of the
 * transmission.
 *
 * @note This function should be called after every successful transmission.
 *
 * @param None
 * @return void.
 *
 * @see transmitData()
 */
void postTransmission();

/**
 * @brief Initializes the Input/Output system.
 *
 * This function sets up the necessary configurations and initializations for the Input/Output system.
 * It should be called before any input or output operations are performed.
 *
 * @details
 * The function performs the following steps:
 *  - Initializes the drivers and hardware required for I/O operations.
 *  - Sets up the default configurations for I/O devices.
 *  - Enables the necessary peripherals and interrupts.
 *
 * @note
 * This function should be called once at the beginning of the program, typically in the main() function.
 *
 */
static void initIO();


/**
 * @brief Checks the timer value and performs necessary actions.
 *
 * This function checks the timer value and performs any actions that are required based on the timer value.
 * It is expected to be called periodically in the main program loop.
 *
 * @param None
 * @return void.
 *
 */
static void check_timer();


/**
 * @brief Reconnect function for establishing a connection
 *
 * This function is responsible for reconnecting to a specified network or server.
 * It attempts to establish a new connection by resetting the connection state and
 * re-executing the connection logic.
 *
 * @param None
 * @return void
 */
static void reConnect();


/**
 * @brief Parse the response received from a network request.
 *
 * This function takes a pointer to a string, which represents the response received from a network request.
 * It parses the response and performs any necessary processing on it.
 *
 * @param reply A pointer to a string containing the response to be parsed.
 *
 * @return void.
 */
static void parseResp(String *reply);


/**
 * @brief Callback function for GET response.
 *
 * This function is called when a GET request receives a response from the server.
 *
 * @param status The status code of the received response.
 * @param off The offset of the data within the received response.
 * @param len The length of the data within the received response.
 */
static void callbackGETResponse(byte status, word off, word len);


/**
 * @brief Retrieves the value at the specified index from the given data string.
 *
 * This function extracts the value at the specified index from the data string, considering the provided separator character.
 * The index should be zero-based, where 0 represents the first value, 1 represents the second value, and so on.
 * If the index is out of range or the data string is empty, an empty string will be returned.
 *
 * @param data      Pointer to the string containing the data.
 * @param separator The character that separates the values in the data string.
 * @param index     The zero-based index of the value to retrieve.
 *
 * @return The value at the specified index in the data string, or an empty string if the index is out of range or the data is empty.
 *
 * @note The data string should remain valid for the duration of the function call.
 */
String getValue(const String *data, char separator, int index);

#endif //CONTROL_GD200A_MAIN_H
