
#include "main.h"

#define PIN_OUT_ON 4

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[500];

const char website[] PROGMEM = "pythoff.com";

static uint32_t timer;
static byte session;
Stash stash;


static void request () {
    byte sd = stash.create();

    stash.print("apikey=");
    stash.print("ADD YOUR API KEY HERE");

    stash.print("&application=");
    stash.print("arduino");

    stash.print("&event=");
    stash.print("Ethercard ");

    stash.print("&description=");
    stash.print("Test message from an Arduino!");

    stash.print("&priority=");
    stash.print("0");

    stash.save();
    int stash_size = stash.size();

    Stash::prepare(PSTR("GET /api/moisture/setting/1/" "\r\n"
                        "Host: $F" "\r\n"
                        "Content-Length: $D" "\r\n"
                        "Content-Type: application/x-www-form-urlencoded" "\r\n"
                        "\r\n"
                        "$H"),
                   website, stash_size, sd);
    session = ether.tcpSend();
    Serial.println("Send");

}

void setup () {
    pinMode(PIN_OUT_ON, OUTPUT);
    digitalWrite(PIN_OUT_ON, LOW);

    Serial.begin(115200);
    Serial.println(F("\n[webClient]"));

    if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0)
        Serial.println(F("Failed to access Ethernet controller"));
    if (!ether.dhcpSetup())
        Serial.println(F("DHCP failed"));

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
}

void loop () {
    ether.packetLoop(ether.packetReceive());

    if (millis() > timer) {
        timer = millis() + 5000;
        Serial.println();
        // Serial.print("<<< REQ ");
        request();
    }

    const char* reply = ether.tcpReply(session);
    if (reply != 0) {
        Serial.println("Got a response!");
        Serial.println(reply);

        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, reply);
        if (error) {
            Serial.println("deserializeJson() failed: ");
            Serial.println(error.c_str());
        }

        String notify = doc["notify"];
        Serial.println(notify);
        if (notify == "true")
        {
            digitalWrite(PIN_OUT_ON, HIGH);
        }
        else
        {
            digitalWrite(PIN_OUT_ON, LOW);
        }



    }
}
