#include <Wire.h>
#include <SPI.h>
#include "sbd360_client.h"

SBD360Client client;

void setup() {
    Serial.begin(115200);
    client.begin();

    // Initialize components
    client.initOLED();
    client.initLoRa();
    client.initRelay();
    client.initRS485();
}

void loop() {
    client.update();

    // Example of reading sensor data
    if (client.isMotionDetected()) {
        Serial.println("Motion detected!");
    }

    if (client.isLeakDetected()) {
        Serial.println("Water leak detected!");
    }

    // Example of sending data via LoRa
    client.sendData("Hello from SBD360!");

    // Example of controlling the relay
    client.turnOnRelay();
    delay(1000);
    client.turnOffRelay();
    delay(1000);
}