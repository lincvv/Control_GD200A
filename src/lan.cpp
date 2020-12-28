/************************************************************
[*] include
*************************************************************/
#include "lan.h"
/************************************************************
[*] init variables
*************************************************************/
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
byte Ethernet::buffer[400];
byte session;

/************************************************************
[*] functions
*************************************************************/

void etherInit(){
//    Serial.println(F("\n[webClient]"));

    if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0) {
        Serial.println(F("[*] Failed Ethernet controller"));
    }
    if (!ether.dhcpSetup())
        Serial.println(F("[*] DHCP failed"));

    ether.printIp(F("[*] IP:  "), ether.myip);
//    ether.printIp("[*] GW:  ", ether.gwip);
//    ether.printIp("[*] DNS: ", ether.dnsip);

#if 1
    // use DNS to resolve the website's IP address
    if (!ether.dnsLookup(website))
        Serial.println(F("[*] DNS failed"));
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

//    ether.printIp("[*] SRV: ", ether.hisip);
    wdt_reset();

}

void POSTRequest(int id) {
    wdt_reset();
    Stash stash;
//    Serial.println();
//    Serial.println("[*] REQUEST POST...");

    byte sd = stash.create();

    stash.print(F("id="));
    stash.print(id);
    stash.print(F("&status="));
    stash.println(F("false"));
    stash.save();
    int stash_size = stash.size();

    Stash::prepare(PSTR("POST http://$F/api/val/ HTTP/1.1" "\r\n"
                        "Host: $F" "\r\n"
                        "Content-Length: $D" "\r\n"
                        "Content-Type: application/x-www-form-urlencoded \r\n"
                        "\r\n"
                        "$H"),
                   website, website, stash_size, sd);

    session = ether.tcpSend();
}

//void GetStatus() {
//    byte sd = stash.create();
//
//    stash.print("apikey=");
//    stash.print("ADD YOUR API KEY HERE");
//
//    stash.print("&application=");
//    stash.print("arduino");
//
//    stash.print("&event=");
//    stash.print("Ethercard ");
//
//    stash.print("&description=");
//    stash.print("Test message from an Arduino!");
//
//    stash.print("&priority=");
//    stash.print("0");
//
//    stash.save();
//    int stash_size = stash.size();
//
//    Stash::prepare(PSTR("GET /api/1/ HTTP/1.1" "\r\n"
//                        "Host: $F" "\r\n"
//                        "Content-Length: $D" "\r\n"
//                        "Content-Type: application/json" "\r\n"
//                        "\r\n"
//                        "$H"),
//                   website, stash_size, sd);
//    session = ether.tcpSend();
//
//}