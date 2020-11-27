#include "main.h"

#define PIN_OUT_ON 4

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[500];

const char website[] PROGMEM = "test.itlab.com.ua";

//void(* resetFunc) (void) = 0;

static uint32_t timer;
static uint8_t count_off = 0;
//static uint8_t etherFailed = 0;
static uint32_t time_off = 0;
uint8_t isOn;
//Stash stash;
uint8_t isOn_addr = 1;
uint8_t time_off_addr = 2;
uint8_t status_off_addr = 3;


static void my_callback (byte status, word off, word len) {
    count_off = 0;
    Serial.println(">>>");
    Ethernet::buffer[off+300] = 0;
    const char* reply = (const char*) Ethernet::buffer + off;
//    Serial.print(reply);
    if (strncmp(reply + 9, "200 OK", 6) != 0) {
        Serial.println(reply);
    }
    String resp = reply;
    resp = resp.substring(resp.indexOf('{'), resp.lastIndexOf('}') + 1);

    Serial.println(resp);
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, resp);
    if (error) {
        Serial.println("deserializeJson() failed: ");
        Serial.println(error.c_str());
        check_timer();
    } else{
        wdt_reset();
        uint8_t temp_isOn;
        temp_isOn = doc["IsOn"];
        time_off = doc["Time"];

        if (temp_isOn != isOn){
            EEPROM.write(isOn_addr, temp_isOn);
            isOn = temp_isOn;
            Serial.println("update data");
        }
        set_state(isOn);
    }

    Serial.println(isOn);
    Serial.println(time_off);
    Serial.println("...");
}


void setup () {
//    uint8_t mcusr = MCUSR;
    Serial.begin(115200);
    pinMode(PIN_OUT_ON, OUTPUT);

    isOn = EEPROM.read(isOn_addr);
    Serial.print("isOn: ");
    Serial.println(isOn);

    if((isOn != 0) & (isOn != 1)){
        isOn = 0;
        Serial.println("Start ison");
        EEPROM.write(isOn_addr, isOn);
    }
    set_state(isOn);

    etherInit();
}

void loop () {
    ether.packetLoop(ether.packetReceive());
//    wdt_reset();

    if (millis() > timer) {
        timer = millis() + REQUEST_INTERVAL;
        Serial.println();

        ether.browseUrl(PSTR("/api/1/"), " ", website, my_callback);
        Serial.print("Check Timer: ");
        check_timer();

    }

//    const char* reply = ether.tcpReply(session);
//
//
//    if (reply != 0) {

}

//#########################################//
void check_timer(){

    if (count_off != 0){
        if (isOn != 0){
            if (count_off != 0){
                EEPROM.write(status_off_addr, 255);
                EEPROM.write(time_off_addr, time_off * 60);
            }
        }
        etherInit();
    }
    count_off++;
}

void set_state(uint8_t state){
    if (state == 1)
    {
        digitalWrite(PIN_OUT_ON, HIGH);
    }
    else
    {
        digitalWrite(PIN_OUT_ON, LOW);
    }
}

void etherInit(){
    wdt_disable();
    Serial.println(F("\n[webClient]"));

    if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0) {
        Serial.println(F("Failed to access Ethernet controller"));
    }
    if (isOn != 0){
        if (EEPROM.read(status_off_addr) == 255){
            time_off = EEPROM.read(time_off_addr) - 20;
            if (time_off <= 0){
                Serial.print("EEprom update ison");
                Serial.println(isOn);
                isOn = 0;
                EEPROM.write(isOn_addr, isOn);
                EEPROM.write(status_off_addr, 0);
                set_state(isOn);
        //            count_off = 0;
        //            time_off = 0;
        //            return;
            }
            else{
                EEPROM.write(time_off_addr, temp_off);
                _delay_ms(10000);
            }
            Serial.println(temp_off);
            Serial.println("---");
        }
    }

    wdt_enable(WDTO_8S);
    if (!ether.dhcpSetup())
        Serial.println(F("DHCP failed"));
    wdt_disable();
//    wdt_enable(WDTO_8S);

    ether.printIp("IP:  ", ether.myip);
    ether.printIp("GW:  ", ether.gwip);
    ether.printIp("DNS: ", ether.dnsip);

#if 1
    // use DNS to resolve the website's IP address
    if (!ether.dnsLookup(website))
        Serial.println("DNS failed");
#elif 2
    // if website is a string containing an IP address instead of a domain name,
  // then use it directly. Note: the string can not be in PROGMEM.
  char websiteIP[] = "192.168.1.1";
  ether.parseIp(ether.hisip, websiteIP);
#else
  // or provide a numeric IP address instead of a string
  byte hisip[] = { 192,168,1,1 };
  ether.copyIp(ether.hisip, hisip);
#endif

    ether.printIp("SRV: ", ether.hisip);
    count_off = 0;
    EEPROM.write(status_off_addr, 0);

}


//    if(MCUSR & (1 << PORF)) { // POR
//        Serial.println("POR");
//        digitalWrite(PIN_OUT_ON, LOW);
//    } else if (MCUSR & (1 << EXTRF)) { // External Reset
//        Serial.println("External Reset");
//        set_state(isOn);
//
//    } else if (MCUSR & (1 << WDRF)){ // Watchdog Reset
//        Serial.println("Watchdog Reset");
//        set_state(isOn);
//    }