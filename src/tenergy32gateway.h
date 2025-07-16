// File: tenergy32gateway.h
/***********************************************************************
 * Project      :     tenergy32gateway
 * Description  :     Library for Tenergy32 Gateway
 *                    This library provides functions to control various
 *                    peripherals and sensors on the Tenergy32 Gateway board.
 * Hardware     :     tenergy32gateway
 * Author       :     Tenergy Innovation Co., Ltd.
 * Date         :     29/04/2025
 * Revision     :     2.6
 * Rev1.0       :     Original
 * Rev1.1       :     Add Example for LoRa receive test [2025-05-02]
                :     Add Example for RTC test
 * Rev2.0       :     Include Ethernet library
 * Rev2.1       :     Add Ethernet Enable/Disable [2025-05-13]
 * Rev2.2       :     Swap parameter of begin() function [2025-05-13]
 * Rev2.3       :     ใช้ค่า Mac Address จากฟังก์ชั่น esp_read_mac(mac, ESP_MAC_ETH) แทนที่จะใช้เป็นการกำหนดค่า
 *                    เองในฟังก์ชั่น initEternet() [2025-05-14]
 * Rev2.4       :     เพิ่มฟ sync timezone สำหรับการตั้งค่าเวลาในฟังก์ชั่น setDateTime() [2025-05-14]
 * Rev2.5       :     Add clearOLED() function [2025-05-15]
 * Rev2.6       :     Improve initLCD() function for I2C LCD [2025-07-17]
 * website      :     http://www.tenergyinnovation.co.th
 * Email        :     uten.boonliam@tenergyinnovation.co.th
 * TEL          :     +66 89-140-7205
 ***********************************************************************/
#ifndef TENERGY32GATEWAY_H
#define TENERGY32GATEWAY_H

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_SSD1306.h>
#include <LiquidCrystal_I2C.h>
#include <Ticker.h> // Include Ticker library
#include <RTClib.h>
#include <Ethernet.h>

// PIN definitions (ปรับปรุงตามข้อมูลบอร์ด)
// Tactile Switches (SW1, SW2) สำหรับเขียนโปรแกรมและทดสอบ
#define PIN_SW1 34 // GPIO34: SW1 (INPUT ONLY)
#define PIN_SW2 35 // GPIO35: SW2 (INPUT ONLY)

// DIP Switches (SW3) สำหรับการตั้งค่า
#define PIN_DIP_SWITCH 36 // GPIO36: DIP switch (INPUT ONLY)

// Relays (4 ตัว) สำหรับควบคุมการเปิด-ปิดวงจร
#define PIN_RELAY1 32 // GPIO32: Relay-1 (Optional port)
#define PIN_RELAY2 33 // GPIO33: Relay-2 (Optional port)
#define PIN_RELAY3 25 // GPIO25: Relay-3 (Optional port)
#define PIN_RELAY4 12 // GPIO12: Relay-4 (Optional port)

// LoRa Module SLX1280 (433MHz) เชื่อมต่อแบบ SPI
#define PIN_LORA_RESET 14 // GPIO14: RESET สำหรับ LoRa
#define PIN_LORA_MOSI 23  // GPIO23: MOSI สำหรับ LoRa
#define PIN_LORA_MISO 19  // GPIO19: MISO สำหรับ LoRa
#define PIN_LORA_SCK 18   // GPIO18: SCK สำหรับ LoRa
#define PIN_LORA_NSS 5    // GPIO05: NSS สำหรับ LoRa
#define PIN_LORA_DIO0 15  // GPIO15: DIO0 สำหรับ LoRa

// Buzzer (สำหรับเล่นเสียง 2.048kHz)
#define PIN_BUZZER 13 // GPIO13: BUZZER

// RS485 connector สำหรับการสื่อสารข้อมูลระยะไกล
#define PIN_TX_485 17 // GPIO17: TX1 สำหรับ RS485
#define PIN_RX_485 16 // GPIO16: RX1 สำหรับ RS485

// I2C Connector (สำหรับ OLED Display, LCD, Sensor ฯลฯ)
#define PIN_I2C_SDA 21 // GPIO21: SDA (LCD Display + Sensor)
#define PIN_I2C_SCL 22 // GPIO22: SCL (LCD Display + Sensor)

// UART Connector สำหรับสื่อสารแบบ UART (และยังสามารถใช้เป็น digital input)
#define PIN_UART_TX 26 // GPIO26: TX ของ UART Connector
#define PIN_UART_RX 27 // GPIO27: RX ของ UART Connector

// W5500 Ethernet Module
#define PIN_W5500_ENABLE 4           // GPIO04: SS สำหรับ W5500-Enable
#define PIN_W5500_MOSI PIN_LORA_MOSI // ใช้ GPIO23: MOSI สำหรับ W5500
#define PIN_W5500_MISO PIN_LORA_MISO // ใช้ GPIO19: MISO สำหรับ W5500
#define PIN_W5500_SCK PIN_LORA_SCK   // ใช้ GPIO18: SCK สำหรับ W5500

// Builtin LED ของ ESP32-DOIT-DEVKIT-V1
#define PIN_BUILTIN_LED 2 // GPIO02: Builtin LED

// Defaults สำหรับอุปกรณ์ I2C
#define OLED_ADDRESS 0x3C
#define LCD_ADDRESS 0x27

class Tenergy32GateWay
{
public:
    const String _version = "2.6"; // Library version

public:
    Tenergy32GateWay();

    // show the library version
    void showLibraryVersion();

    // Initialize the Tenergy32 Gateway board
    bool begin(bool useEthernet = false, uint32_t loraFreq = 443E6);

    // Switches and sensors
    bool readSlideSwitch();
    bool readSW1();
    bool readSW2();

    // Actuators
    void relay1On();
    void relay1Off();
    void setRelay1(bool state);
    bool relay1State();
    void relay2On();
    void relay2Off();
    void setRelay2(bool state);
    bool relay2State();
    void relay3On();
    void relay3Off();
    void setRelay3(bool state);
    bool relay3State();
    void relay4On();
    void relay4Off();
    void setRelay4(bool state);
    bool relay4State();
    void setbuildingLED(bool on);
    void beep(uint8_t times, uint16_t ms = 100);

    // LoRa functions
    bool sendLoRa(const uint8_t *data, size_t len);
    bool receiveLoRa(uint8_t *buffer, size_t maxLen, int &received);

    // RS485 (simple TX/RX)
    void rs485TransmitMode(bool enable);
    size_t sendRS485(const uint8_t *data, size_t len);
    size_t receiveRS485(uint8_t *buffer, size_t maxLen);

    // I2C devices
    void initI2C();
    bool initOLED(uint8_t address = OLED_ADDRESS);
    void displayOLED(const char *text);
    void displayOLEDInfo();

    // Displays text on up to 4 separate lines on the OLED.
    // Each parameter is optional (default empty string) and at least line1 should be provided.
    void displayOLEDLines(const char *line1, const char *line2 = "", const char *line3 = "", const char *line4 = "");

        // clearOLED
    // Clears the OLED display and resets the cursor position.
    // This function is useful for preparing the display for new content.
    void clearOLED();

    bool initLCD(uint8_t address = LCD_ADDRESS, uint8_t cols = 16, uint8_t rows = 2);
    void displayLCD(const char *text, uint8_t col = 0, uint8_t row = 0);

    // Sound functions
    void marioSound();
    void angryBirdSound();

    // Blink functions for building LED using Ticker callbacks.
    // intervalMillis: full blink cycle duration.
    // If 0 is passed, blinking stops and the LED is turned off.
    void blinkbuildingLED(uint32_t intervalMillis);

    // --- Added public RTC functions ---
    bool setDateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec);
    void getTime(uint8_t &_hour, uint8_t &_min, uint8_t &_sec);
    void getDate(uint16_t &_year, uint8_t &_month, uint8_t &_day);
    void getDateTime(uint16_t &_year, uint8_t &_month, uint8_t &_day, uint8_t &_hour, uint8_t &_min, uint8_t &_sec);
    char *getDayOfTheWeekChar(void);
    uint8_t getDayOfTheWeekNum(void);
    float getTemperature(void);
    uint16_t getTimestamp(void);

    // --- Added public variables ---
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t sec;
    String dayofweek;
    float temperature;
    uint16_t timestamp;

    Adafruit_SSD1306 *_oled;
    LiquidCrystal_I2C *_lcd;

    // private  variables for DS3231 RTC
    RTC_DS3231 *_rtc;

    // Private Ticker object for asynchronous building LED blinking.
    Ticker _tickerBuilding;

    // Private state variable for building LED toggling.
    bool _buildingState;

    // Static pointer to allow callbacks to access this instance.
    static Tenergy32GateWay *_instance;

    // Static callback function used by Ticker for building LED.
    static void buildingLEDToggle();

    // private function to initialize the RTC
    const char *_ntpServer = "asia.pool.ntp.org";
    const long _gmtOffset_sec = 25200;
    const int _daylightOffset_sec = 0;
    const uint8_t _daysInMonth[12] PROGMEM = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    const char _daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

    bool initEternet(uint8_t *mac, uint8_t *ip, uint8_t *gw, uint8_t *subnet, bool useDHCP = true);
    bool initEternet(uint8_t *mac, bool useDHCP = true);
    bool initEternet(uint8_t *mac, uint8_t *ip, bool useDHCP = true);
    bool initEternet(uint8_t *mac, uint8_t *ip, uint8_t *gw, bool useDHCP = true);
};

#endif // TENERGY32GATEWAY_H
