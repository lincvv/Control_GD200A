/************************************************************
[*] include
*************************************************************/
#include "lan.h"
/************************************************************
[*] init variables
*************************************************************/
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
byte Ethernet::buffer[400];
SoftwareSerial master(PIN_RX, PIN_TX);

/************************************************************
[*] functions
*************************************************************/

void etherInit(){
    Serial.println(F("\n[webClient]"));

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