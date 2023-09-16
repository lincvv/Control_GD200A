/************************************************************
[*] include
*************************************************************/
#include "main.h"

/************************************************************
[*] init variables
*************************************************************/
static uint32_t timer;
uint8_t isOn __attribute__ ((section (".noinit")));
uint8_t status_reg __attribute__ ((section (".noinit")));
uint16_t freq __attribute__ ((section (".noinit")));
uint8_t mcusr_f __attribute__ ((section (".noinit")));
static uint16_t time_off __attribute__ ((section (".noinit")));
static uint16_t timer_time_off __attribute__ ((section (".noinit")));
uint8_t count_notfound __attribute__ ((section (".noinit")));
uint8_t count_ether_failed __attribute__ ((section (".noinit")));
ModbusMaster node;
extern SoftwareSerial master;

//void(* resetFunc) (void) = 0;
/************************************************************
[*] code
*************************************************************/

void setup() {
    mcusr_f = MCUSR;
    MCUSR = 0;
    wdt_disable();
    initIO();

    if (mcusr_f & _BV(EXTRF) || mcusr_f & _BV(PORF)) {
#ifdef DEV
        master.println(F("[*] Reboot ==> EXTRF"));
#endif
        count_notfound = 0;
        count_ether_failed = 0;
        timer_time_off = 0;
        time_off = 0;
        isOn = STATE_OFF;
        freq = 2500;
        uint8_t modbus_res;
        modbus_res = node.writeSingleRegister(A_FREQ_REGISTER_R_W, freq);
        if (modbus_res != ModbusMaster::ku8MBSuccess) {
#ifdef DEV
            master.println(modbus_res);
#endif
        }
        modbus_res = node.writeSingleRegister(A_SET_STATE_REGISTER_W, isOn);
        if (modbus_res != ModbusMaster::ku8MBSuccess) {
#ifdef DEV
            master.println(modbus_res);
#endif
        }

    }

    if (mcusr_f & _BV(WDRF))
        master.println(F("[*] Reboot ==> WDRF"));


    wdt_enable(WDTO_4S);
    check_timer();
    etherInit();
    wdt_enable(WDTO_8S);
    digitalWrite(PIN_LOOP_CONNECT, HIGH);

}

void loop() {
    ether.packetLoop(ether.packetReceive());

    if (millis() > timer) {
        wdt_reset();
        timer = millis() + REQUEST_INTERVAL;
#ifdef DEV
//        master.println(count_notfound);
//        master.println(count_ether_failed);
#endif

        uint8_t res_f;
        res_f = node.readHoldingRegisters(A_FREQ_REGISTER_R_W, QUANTITY_REGISTER);
        if (res_f == ModbusMaster::ku8MBSuccess) {
            freq = node.getResponseBuffer(0);
#ifdef DEV
            master.println(freq);
#endif
        }

        uint8_t res_state;
        res_state = node.readHoldingRegisters(A_STATE_REGISTER_R, QUANTITY_REGISTER);
        if (res_state == ModbusMaster::ku8MBSuccess) {
            status_reg = node.getResponseBuffer(0);
        }
        wdt_reset();
#ifdef DEV
        master.print("status_reg: ");
        master.println(status_reg);
#endif
        char get_r[20];
        sprintf(get_r, "%d/%d", status_reg, freq);

        ether.browseUrl(PSTR("/api/devices/1/"), get_r, website, PSTR("accept: application/json"), callbackGETResponse);

        digitalWrite(PIN_LOOP_CONNECT, !digitalRead(PIN_LOOP_CONNECT));
        check_timer();
        wdt_reset();
    }
}

/**
 * @brief Checks the timer to determine if a certain amount of time has elapsed.
 *
 * This function checks the timer to determine if a certain amount of time, specified by the user,
 * has elapsed since the timer was started.
 *
 * @param start_time The starting time of the timer, in milliseconds.
 * @param elapsed_time The elapsed time, in milliseconds, to check against the start time.
 * @return void.
 */

static void check_timer() {
    if (isOn != STATE_OFF) {
        if (timer_time_off == 12) { // check that callbackGETResponse was not called 12 times
            time_off--;
#ifdef DEV
            master.printf(F("[*] timer min ==> %d\n"), time_off);
#endif
            if (time_off == 0) {
                time_off = 0;
                isOn = STATE_OFF;
                uint8_t modbus_res = node.writeSingleRegister(A_SET_STATE_REGISTER_W, isOn);
                if (modbus_res != ModbusMaster::ku8MBSuccess) {
#ifdef DEV
                    master.println(modbus_res);
#endif
                }
            }
            reConnect();
        }
    } else if ((timer_time_off >= 6)) { // If there is no network within 30 seconds, perform a reset
        reConnect();
    }
#ifdef DEV
//    master.println(timer_time_off);
#endif
    timer_time_off++;
    wdt_reset();
}


static void reConnect() {
    count_ether_failed++;
    timer_time_off = 0;
    wdt_enable(WDTO_15MS);
    delay(20);
}

/**
 * @brief Get the value at a specific index from a string data separated by a given separator character.
 *
 * @param data The string data to extract value from.
 * @param separator The character used to separate the values in the string.
 * @param index The index of the value to retrieve.
 * @return The value at the specified index, or an empty string if the index is out of range.
 *
 * This function takes a string, `data`, that contains values separated by a specified `separator` character.
 * It extracts the value at the specified `index` and returns it as a new string.
 * If the `index` is greater than or equal to the number of values in `data`, an empty string is returned.
 *
 * Example usage:
 *
 * ```cpp
 * String data = "apple,banana,orange,grape";
 * char separator = ',';
 * int index = 2;
 * String value = getValue(&data, separator, index);
 * // value = "orange"
 * ```
 */

String getValue(const String *data, char separator, int index) {
    int found = 0;
    int strIndex[] = {0, -1};
    unsigned int maxIndex = data->length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data->charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i + 1 : i;
        }
    }
    return found > index ? data->substring(strIndex[0], strIndex[1]) : "";
}

/**
 * @brief Parses the given response data.
 *
 * This function is responsible for parsing the response data received as input.
 * The response data is expected to be in the format of a string.
 *
 * @param res_data Pointer to the response data string to be parsed.
 *
 * @return void.
 */

static void parseResp(String *res_data) {
    wdt_reset();
    timer_time_off = 0;
    uint8_t modbus_res;
    *res_data = res_data->substring(res_data->indexOf('{') + 1, res_data->lastIndexOf('}'));
#ifdef DEV
    master.println(*res_data);
#endif

    //state
    String temp_json = getValue(res_data, ',', 0);
    temp_json = getValue(&temp_json, ':', 1);
    uint8_t data_isOn = temp_json.toInt();
    if (isOn != data_isOn) {
        isOn = data_isOn;
        modbus_res = node.writeSingleRegister(A_SET_STATE_REGISTER_W, isOn);
        if (modbus_res != ModbusMaster::ku8MBSuccess) {
#ifdef DEV
            master.println(modbus_res);
#endif
        }
        return;
    }

    //freq
    temp_json = getValue(res_data, ',', 2);
    temp_json = getValue(&temp_json, ':', 1);
    uint16_t data_freq = temp_json.toInt();
    if (freq != data_freq) {
        freq = data_freq;
        modbus_res = node.writeSingleRegister(A_FREQ_REGISTER_R_W, freq);
        if (modbus_res != ModbusMaster::ku8MBSuccess) {
#ifdef DEV
            master.println(modbus_res);
#endif
        }
    }

    //time
    temp_json = getValue(res_data, ',', 1);
    temp_json = getValue(&temp_json, ':', 1);
    time_off = temp_json.toInt();
    wdt_reset();

}

/**
 *@brief Initializes the Input/Output (IO) module.
 *
 * This function performs necessary initialization steps for the IO module.
 * It sets up any required configurations, opens communication channels,
 * and prepares the system for performing input and output operations.
 *
 * @return Void.
 */

static void initIO() {
    master.begin(9600);
    Serial.begin(9600);
    node.begin(1, Serial);
    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);
    pinMode(PIN_RE_DE, OUTPUT);
    pinMode(PIN_LOOP_CONNECT, OUTPUT);
    digitalWrite(PIN_RE_DE, LOW);
}

/************************************************************
[*] callbacks
*************************************************************/
/**
 * @brief Callback function for handling GET response.
 *
 * This callback function is called when a GET request has been sent and a response is received. The function can be
 * implemented by the user to handle the response accordingly.
 *
 * @param status The status code of the response.
 * @param off The offset of the response data.
 * @param len The length of the response data.
 */
static void callbackGETResponse(byte status, word off, word len) {

    Ethernet::buffer[off + 300] = 0;
    const char *reply = (const char *) Ethernet::buffer + off;

    if (strncmp(reply + 9, "200 OK", 6) != 0) {
        count_notfound++;
#ifdef DEV
        master.println(reply);
#endif
        return;
    }

    String resp = reply;
    parseResp(&resp);
#ifdef DEV
    master.printf(F("\nisOn: %d\n"), isOn);
#endif
    wdt_reset();
}

void preTransmission() {
    digitalWrite(PIN_RE_DE, HIGH);
}

void postTransmission() {
    digitalWrite(PIN_RE_DE, LOW);
}

/************************************************************
[*]
*************************************************************/