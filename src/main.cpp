/************************************************************
[*] include
*************************************************************/
#include "main.h"
/************************************************************
[*] init variables
*************************************************************/
static uint32_t timer;
uint8_t isOn __attribute__ ((section (".noinit")));
uint16_t freq __attribute__ ((section (".noinit")));
uint8_t mcusr_f __attribute__ ((section (".noinit")));
static uint16_t time_off __attribute__ ((section (".noinit")));
static uint16_t timer_time_off __attribute__ ((section (".noinit")));
uint8_t count_notfound __attribute__ ((section (".noinit")));
uint8_t count_ether_failed __attribute__ ((section (".noinit")));
SoftwareSerial master(PIN_RX, PIN_TX);
ModbusMaster node;

//void(* resetFunc) (void) = 0;
/************************************************************
[*] code
*************************************************************/

void setup () {
    mcusr_f = MCUSR;
    MCUSR = 0;
    wdt_disable();
    initIO();

    if (mcusr_f & _BV(EXTRF) || mcusr_f & _BV(PORF)){
        Serial.println(F("[*] Reboot ==> EXTRF"));
        count_notfound = 0;
        count_ether_failed = 0;
        timer_time_off = 0;
        time_off = 0;
        isOn = STATE_OFF;
        freq = 2500;
        uint8_t modbus_res;
        modbus_res = node.writeSingleRegister(A_FREQ_REGISTER_R_W, freq);
        if(modbus_res != node.ku8MBSuccess){
            Serial.println(modbus_res);
        }
        modbus_res = node.writeSingleRegister(A_SET_STATE_REGISTER_W, isOn);
        if(modbus_res != node.ku8MBSuccess){
            Serial.println(modbus_res);
        }

    }

    if (mcusr_f & _BV(WDRF)){
        Serial.println(F("[*] Reboot ==> WDRF"));
    }

    wdt_enable(WDTO_4S);
    check_timer();
    etherInit();
    wdt_enable(WDTO_8S);
    digitalWrite(PIN_LOOP_CONNECT, HIGH );

}

/************************************************************
[*] loop
*************************************************************/

void loop () {
    ether.packetLoop(ether.packetReceive());

    if (millis() > timer) {
        wdt_reset();
        timer = millis() + REQUEST_INTERVAL;
//        Serial.println(count_notfound);
//        Serial.println(count_ether_failed);

        uint8_t res_f;
        res_f = node.readHoldingRegisters(A_FREQ_REGISTER_R_W, QUANTITY_REGISTER);
        if(res_f == node.ku8MBSuccess){
            freq = node.getResponseBuffer(0);
            Serial.println(freq);
        }

        uint8_t status;
        status = node.readHoldingRegisters(A_STATE_REGISTER_R, QUANTITY_REGISTER);
        if(status == node.ku8MBSuccess){
            isOn = node.getResponseBuffer(0);
            }
        wdt_reset();
        Serial.print("status: ");
        Serial.println(status);
        char get_r[20];
        sprintf(get_r, "%d/%d/", isOn, freq);

        ether.browseUrl(PSTR("/api/devices/1/"), get_r, website, PSTR("accept: application/json"), callbackGETResponse);

        digitalWrite(PIN_LOOP_CONNECT, !digitalRead(PIN_LOOP_CONNECT));
        check_timer();
        wdt_reset();
    }
}

/************************************************************
[*] functions
*************************************************************/

static void check_timer(){
    if (isOn != 3){
        if (timer_time_off == 12){
            time_off--;
            Serial.printf(F("[*] timer min ==> %d\n"), time_off);
            if(time_off == 0){
                time_off = 0;
                isOn = STATE_OFF;
                uint8_t modbus_res = node.writeSingleRegister(A_SET_STATE_REGISTER_W, isOn);
                if(modbus_res != node.ku8MBSuccess){
                    Serial.println(modbus_res);
                }
            }
            reConnect();
        }
    }else if((timer_time_off >= 6)){ //если нет сети через 30 сек. сделать reset
        reConnect();
    }
//    Serial.println(timer_time_off);
    timer_time_off++;
    wdt_reset();
}

static void reConnect(){
    count_ether_failed++;
    timer_time_off = 0;
    wdt_enable(WDTO_15MS);
    delay(20);
}

String getValue(const String* data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    unsigned int maxIndex = data->length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data->charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data->substring(strIndex[0], strIndex[1]) : "";
}

static void parseResp(String* res_data){
    wdt_reset();
    timer_time_off = 0;
    uint8_t modbus_res;
    *res_data = res_data->substring(res_data->indexOf('{') + 1 , res_data->lastIndexOf('}'));
    Serial.println(*res_data);

    //state
    String temp_json = getValue(res_data, ',', 0);
    temp_json = getValue(&temp_json, ':', 1);
    uint8_t data_isOn = temp_json.toInt();
    if (isOn != data_isOn){
        isOn = data_isOn;
        modbus_res = node.writeSingleRegister(A_SET_STATE_REGISTER_W, isOn);
        if(modbus_res != node.ku8MBSuccess){
            Serial.println(modbus_res);
        }
    }

    //freq
    temp_json = getValue(res_data, ',', 2);
    temp_json = getValue(&temp_json, ':', 1);
    uint16_t data_freq = temp_json.toInt();
    if (freq != data_freq){
        freq = data_freq;
        modbus_res = node.writeSingleRegister(A_FREQ_REGISTER_R_W, freq);
        if(modbus_res != node.ku8MBSuccess){
            Serial.println(modbus_res);
        }
    }

    //time
    temp_json = getValue(res_data, ',', 1);
    temp_json = getValue(&temp_json, ':', 1);
    time_off = temp_json.toInt();
    wdt_reset();

}

static void initIO(){
    master.begin(9600);
    Serial.begin(9600);
    node.begin(1, master);
    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);
    pinMode(PIN_RE_DE, OUTPUT);
    pinMode(PIN_LOOP_CONNECT, OUTPUT);
    digitalWrite(PIN_RE_DE, LOW);
}

/************************************************************
[*] callbacks
*************************************************************/
static void callbackGETResponse(byte status, word off, word len){

    Ethernet::buffer[off+300] = 0;
    const char* reply = (const char*) Ethernet::buffer + off;

    if (strncmp(reply + 9, "200 OK", 6) != 0) {
        count_notfound++;
        Serial.println(reply);
        return;
    }

    String resp = reply;
    parseResp(&resp);
    Serial.printf(F("\nisOn: %d\n"), isOn);
    wdt_reset();
}

void preTransmission()
{
    digitalWrite(PIN_RE_DE, HIGH);
}

void postTransmission()
{
    digitalWrite(PIN_RE_DE, LOW);
}

/************************************************************
[*]
*************************************************************/