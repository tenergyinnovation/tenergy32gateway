/***********************************************************************
 * Project      :     tenergy32gateway_template
 * Description  :     Test program for Tenergy32 Gateway board
 * Hardware     :     Tenergy32GateWay
 * Author       :     Tenergy Innovation Co., Ltd.
 * Date         :     27/04/2025
 ***********************************************************************/
#include <Arduino.h>
#include <tenergy32gateway.h>
#include <esp_task_wdt.h>

Tenergy32GateWay mcu;

void header_print(void)
{
    Serial.printf("\r\n***********************************************************************\r\n");
    Serial.printf("* Project      :     tenergy32gateway_template\r\n");
    Serial.printf("* Description  :     Template coding for Tenergy32GateWay on PlatformIO\r\n");
    Serial.printf("* Hardware     :     Tenergy32GateWay\r\n");
    Serial.printf("* Author       :     Tenergy Innovation Co., Ltd.\r\n");
    Serial.printf("* Date         :     27/04/2025\r\n");
    Serial.printf("* Revision     :     %s\r\n", mcu._version.c_str());
    Serial.printf("* website      :     http://www.tenergyinnovation.co.th\r\n");
    Serial.printf("* Email        :     uten.boonliam@tenergyinnovation.co.th\r\n");
    Serial.printf("* TEL          :     +66 89-140-7205\r\n");
    Serial.printf("***********************************************************************/\r\n");
}

void setup()
{
    Serial.begin(115200);
    header_print();

    // Initialize watchdog timer
    esp_task_wdt_init(10, true);
    esp_task_wdt_add(NULL);

    if (!mcu.begin())
    {
        Serial.println("Board initialization failed!");
        while (1)
            ;
    }

    // Delay to view initial info
    delay(1000);
}


void loop()
{

    // --- รับข้อมูลจาก LoRa ---
    uint8_t buffer[16];
    int received = 0;
    if (mcu.receiveLoRa(buffer, sizeof(buffer), received) && received > 0)
    {
        int rssi = LoRa.packetRssi();
        Serial.print("LoRa RX: ");
        for (int i = 0; i < received; ++i)
        {
            Serial.print(buffer[i]);
            Serial.print(" ");
        }
        Serial.print(" | RSSI: ");
        Serial.println(rssi);

        // แสดงผลบน OLED (ค่าที่รับและ RSSI)
        char loraMsg[32];
        snprintf(loraMsg, sizeof(loraMsg), "LoRa RX: %d", buffer[0]);
        char rssiMsg[32];
        snprintf(rssiMsg, sizeof(rssiMsg), "RSSI: %d dBm", rssi);
        mcu.displayOLEDLines(loraMsg, rssiMsg);
        // แสดงผลบน Serial Monitor
        Serial.println(loraMsg);
        Serial.println(rssiMsg);
    
    }

    // --- จบส่วนรับ LoRa ---


    esp_task_wdt_reset();
    delay(100);
}
