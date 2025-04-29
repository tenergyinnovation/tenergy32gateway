#include <Wire.h>
#include <SPI.h>
#include "sbd360_client.h"

SBD360Client client;

void setup() {
    Serial.begin(115200);
    client.begin();
    client.displayText("SBD360 Client Initialized");
}

void loop() {
    client.update();
    
    if (client.isMotionDetected()) {
        client.displayText("Motion Detected!");
    }
    
    if (client.isLeakDetected()) {
        client.displayText("Water Leak Detected!");
    }
    
    delay(1000);
}