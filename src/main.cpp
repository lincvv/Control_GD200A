/************************************************************
[*] include
*************************************************************/
#include "main.h"
/************************************************************
[*] init variables
*************************************************************/
extern byte session;
static uint32_t timer;
uint8_t isOn __attribute__ ((section (".noinit")));
uint8_t mcusr_f __attribute__ ((section (".noinit")));
static uint16_t time_off __attribute__ ((section (".noinit")));
static uint16_t timer_time_off __attribute__ ((section (".noinit")));
uint8_t count_notfound __attribute__ ((section (".noinit")));
uint8_t count_ether_failed __attribute__ ((section (".noinit")));
//SoftwareSerial master(PIN_RX, PIN_TX);
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
        count_notfound = 0;
        count_ether_failed = 0;
        timer_time_off = 0;
        time_off = 0;
        isOn = 0;
//        Serial.println("[*] Reboot ==> EXTRF");
        set_state(isOn);

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
        Serial.println(count_notfound);
//        Serial.println(count_ether_failed);
        POSTRequest(1);
        digitalWrite(PIN_LOOP_CONNECT, !digitalRead(PIN_LOOP_CONNECT));
        check_timer();
    }

    const char* reply = ether.tcpReply(session);

    if (reply != 0) {
        if (strncmp(reply + 9, "201 Created", 11) == 0){
            Serial.println(F("[*] 201"));
            timer_time_off = 0;
            delay(30);
            ether.browseUrl(PSTR("/api/1/"), " ", website, callbackGETResponse);
        } else{
            Serial.println(reply);
            count_notfound++;
        }
        wdt_reset();
    }
}

/************************************************************
[*] functions
*************************************************************/

static void check_timer(){
    if (isOn != 0){
        if (timer_time_off == 12){
            time_off--;
            Serial.printf(F("[*] timer min ==> %d\n"), time_off);
            if(time_off == 0){
                time_off = 0;
                isOn = 0;
                set_state(isOn);
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

static void set_state(uint8_t state){
    wdt_reset();
    Serial.println(F("set_state"));
    if (state == 1)
    {
//        node.writeSingleRegister(0x40001, 1);
        node.readHoldingRegisters(0x2100, 01);
//        digitalWrite(PIN_ON_INV, HIGH);
//        delay(1000);
//        digitalWrite(PIN_ON_INV, LOW);
    }
    else
    {
        node.readHoldingRegisters(0x2100, 01);

//        digitalWrite(PIN_OFF_INV, HIGH);
//        delay(1000);
//        digitalWrite(PIN_OFF_INV, LOW);
    }
    wdt_reset();
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
    *res_data = res_data->substring(res_data->indexOf('{') , res_data->lastIndexOf('}') + 1);
    String temp_json = res_data->substring(res_data->indexOf('{') + 1, res_data->lastIndexOf(','));
    temp_json = getValue(&temp_json, ':', 1);
    uint8_t data_isOn = temp_json.toInt();
    if (isOn != data_isOn){
        isOn = data_isOn;
        set_state(isOn);
    }
    temp_json = res_data->substring(res_data->indexOf(',') + 1, res_data->lastIndexOf('}'));
    temp_json = getValue(&temp_json, ':', 1);
    time_off = temp_json.toInt();
}

static void initIO(){
//    master.begin(115200);
    Serial.begin(9600);
    node.begin(1, Serial);
    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);
    pinMode(PIN_RE_DE, OUTPUT);
//    pinMode(PIN_ON_INV, OUTPUT);
//    pinMode(PIN_OFF_INV, OUTPUT);
    pinMode(PIN_LOOP_CONNECT, OUTPUT);
    digitalWrite(PIN_RE_DE, LOW);
}

/************************************************************
[*] callbacks
*************************************************************/
static void callbackGETResponse(byte status, word off, word len){

//    Serial.println("[*] GET RESPONSE");
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

//    const char* postDataChar = "id=1";
//    ether.httpPost(PSTR("/api/val/"), website, NULL,
//                   postDataChar,
//                   POSTCallbackResp);

//static void POSTCallbackResp(byte status, word off, word len){
//    Ethernet::buffer[off+300] = 0;
//    const char* reply = (const char*) Ethernet::buffer + off;
//    Serial.println(reply);
//}

//    String resp = reply;
//    resp = resp.substring(resp.indexOf('{'), resp.lastIndexOf('}') + 1);

//    StaticJsonDocument<200> doc;
//    DeserializationError error = deserializeJson(doc, reply);
//    if (error) {
//        Serial.println("[*] deserializeJson() failed: ");
//        Serial.println(error.c_str());
//        Serial.print("[*] ");
//        Serial.println(resp);
//        return;
//    } else{
//        timer_time_off = 0;
//        isOn = doc["IsOn"];
//        time_off = doc["Time"];
//        if (state_isOn != isOn){
//            set_state(isOn);
//            state_isOn = isOn;
//        }
//    }