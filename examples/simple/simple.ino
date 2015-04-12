#include <Judp.h>
#include <SPI.h>
#include <aJSON.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#define DEBUG

byte mac[] = {0x20, 0x14, 0x05, 0x23, 0x18, 0x17};
Judp server(6666);

void setup() {
    Serial.begin(115200);
    Ethernet.begin(mac);
    server.begin();
}

void loop() {
    server.processing();
}
