

/************************************************************
[*] include
*************************************************************/
#include "main.h"

/************************************************************
[*] variable
*************************************************************/
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
byte Ethernet::buffer[500];
const char website[] PROGMEM = "test.itlab.com.ua";

static uint32_t timer;
static uint8_t regs_time_off = 0;
static uint32_t time_off = 0;
uint8_t isOn;
uint32_t timer_off_s;
uint8_t mcusr_f;
//uint8_t isOn_addr = 1;
//Stash stash;
//uint32_t time_off_addr = 2;
//uint8_t status_off_addr = 3;

//void(* resetFunc) (void) = 0;
/************************************************************
[*] code
*************************************************************/

void setup () {
    wdt_disable();
    mcusr_f = MCUSR;
    MCUSR = 0;
    Serial.begin(115200);
    pinMode(PIN_OUT_ON, OUTPUT);

    if (mcusr_f & (1 << EXTRF) || mcusr_f & (1 << PORF) || mcusr_f & (1 << WDRF)){
        isOn = 0;
        Serial.println("[*] Reboot ==> EXTRF OR WDRF");
        set_state(isOn);;
    }
    etherInit();
}

void loop () {
    ether.packetLoop(ether.packetReceive());

    if (millis() > timer) {
        timer = millis() + REQUEST_INTERVAL;
        Serial.println();
        Serial.println("[*] GET REQUEST");
        ether.browseUrl(PSTR("/api/1/"), " ", website, callback_response);
        check_timer();
    }
}

/************************************************************
[*] functions
*************************************************************/

void check_timer(){
    Serial.print("[*] State isOn ==> ");
    Serial.println(isOn);
    Serial.print("[*] Timer sec ==> ");
    Serial.println(regs_time_off * 5);
    if (regs_time_off != 0){
        if (isOn != 0){
            Serial.println("[*] Check timer >>>");
            if(time_off <= 0){
                isOn = 0;
//                EEPROM.write(isOn_addr, isOn);
                set_state(isOn);
                regs_time_off = 0;
                Serial.println("[*] STATE OFF");
                return;
            } else{
                if (regs_time_off == 12){
                    time_off--;
                    regs_time_off = 0;
                    Serial.print("[*] Current timer min ==> ");
                    Serial.println(time_off);
                }else{
                    regs_time_off++;
                    Serial.println("[*] ...");
                    return;
                }
            }
        } else{
            regs_time_off = 0;
            return;
        }
    }
    regs_time_off++;
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
        Serial.println(F("[*] Failed to access Ethernet controller"));
    }
    if (!ether.dhcpSetup())
        Serial.println(F("[*] DHCP failed"));

    ether.printIp("[*] IP:  ", ether.myip);
    ether.printIp("[*] GW:  ", ether.gwip);
    ether.printIp("[*] DNS: ", ether.dnsip);

#if 1
    // use DNS to resolve the website's IP address
    if (!ether.dnsLookup(website))
        Serial.println("[*] DNS failed");
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

    ether.printIp("[*] SRV: ", ether.hisip);

}

static void callback_response(byte status, word off, word len){

    Serial.println();
    Serial.println("[*] RESPONSE");
    Ethernet::buffer[off+300] = 0;
    const char* reply = (const char*) Ethernet::buffer + off;
//    Serial.print(reply);
    if (strncmp(reply + 9, "200 OK", 6) != 0) {
        Serial.print("[*] Status code ==> ");
        Serial.println(reply);
        return;
    }
    String resp = reply;
    resp = resp.substring(resp.indexOf('{'), resp.lastIndexOf('}') + 1);

    Serial.print("[*] ");
    Serial.println(resp);
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, resp);
    if (error) {
        Serial.println("[*] deserializeJson() failed: ");
        Serial.println(error.c_str());
//        check_timer();
    } else{
        wdt_reset();
        regs_time_off = 0;
        isOn = doc["IsOn"];
        time_off = doc["Time"];
        set_state(isOn);
    }

    Serial.print("[*] State isOn/Time ==> ");
    Serial.print(isOn);
    Serial.print("/");
    Serial.println(time_off);
    Serial.println("[*] END RESPONSE");
}