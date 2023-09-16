/************************************************************
[*] include
*************************************************************/
#include "lan.h"

/************************************************************
[*] init variables
*************************************************************/
static byte mymac[] = {0x74, 0x69, 0x69, 0x2D, 0x30, 0x31};
byte Ethernet::buffer[400];
SoftwareSerial master(PIN_RX, PIN_TX);

/**
 * @brief Initializes the Ethernet module.
 *
 * This function is responsible for initializing the Ethernet module. It performs the necessary
 * setup and configuration to enable the Ethernet communication.
 *
 * @note This function must be called before any other Ethernet-related functions are called.
 *
 * @return void.
 */
void etherInit() {
#ifdef DEV
    master.println(F("\n[webClient]"));
#endif


    if (ether.begin(sizeof Ethernet::buffer, mymac, SS) == 0) {
#ifdef DEV
        master.println(F("[*] Failed Ethernet controller"));
#endif
    }
    if (!ether.dhcpSetup()) {
#ifdef DEV
        master.println(F("[*] DHCP failed"));
#endif
    }
    ether.printIp(F("[*] IP:  "), ether.myip);
//    ether.printIp("[*] GW:  ", ether.gwip);
//    ether.printIp("[*] DNS: ", ether.dnsip);

#if 1
    // use DNS to resolve the website's IP address
    if (!ether.dnsLookup(website)) {
#ifdef DEV
        master.println(F("[*] DNS failed"));
#endif
    }
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