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
    // อัพเดทวันที่และเวลา จาก RTC
    uint16_t yr;
    uint8_t mon, d, hr, min, sec;
    mcu.getDateTime(yr, mon, d, hr, min, sec);

    char datetimeStr[32];
    snprintf(datetimeStr, sizeof(datetimeStr), "%04d/%02d/%02d %02d:%02d:%02d", yr, mon, d, hr, min, sec);

    // แสดงวันที่และเวลาผ่าน Serial และ OLED
    Serial.println(datetimeStr);
    mcu.displayOLED(datetimeStr);

    // ตรวจสอบปุ่มกด
    if (mcu.readSW1())
    {
        // เมื่อกด SW1 ให้เล่นเสียง Mario, เปิด relay1 และ relay3, ปิด relay2 และ relay4
        mcu.marioSound();
        mcu.relay1On();
        mcu.relay3On();
        mcu.relay2Off();
        mcu.relay4Off();
        Serial.println("SW1 pressed: MarioSound, Relay1 & Relay3 ON, Relay2 & Relay4 OFF");
        delay(1000);
    }
    else if (mcu.readSW2())
    {
        // เมื่อกด SW2 ให้เล่นเสียง AngryBird, ปิด relay1 และ relay3, เปิด relay2 และ relay4
        mcu.angryBirdSound();
        mcu.relay1Off();
        mcu.relay3Off();
        mcu.relay2On();
        mcu.relay4On();
        Serial.println("SW2 pressed: AngryBirdSound, Relay1 & Relay3 OFF, Relay2 & Relay4 ON");
        delay(1000);
    }

    // รีเซ็ต watchdog timer และ delay ก่อนรอบถัดไป
    esp_task_wdt_reset();
    delay(1000);
}
