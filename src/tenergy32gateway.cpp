// File: tenergy32gateway.cpp
#include "tenergy32gateway.h"
#include <RTClib.h>
#include <Ethernet.h>
#include <esp_system.h>

// Initialize static instance pointer to NULL.
Tenergy32GateWay *Tenergy32GateWay::_instance = nullptr;
/***********************************************************************
 * FUNCTION:    Tenergy32GateWay
 * DESCRIPTION: Constructor for the Tenergy32GateWay class.
 *              Initializes member variables and sets the static instance pointer.
 ***********************************************************************/
Tenergy32GateWay::Tenergy32GateWay() : _oled(nullptr), _lcd(nullptr),
                                       _buildingState(false)
{
    _instance = this;
}

/***********************************************************************
 * FUNCTION:    showLibraryVersion
 * DESCRIPTION: Displays the library version on the serial monitor and OLED.
 * PARAMETERS:  None
 * RETURNED:    None
 ***********************************************************************/
void Tenergy32GateWay::showLibraryVersion()
{
    Serial.print("Tenergy32GateWay Library Version: ");
    Serial.println(_version);
    if (_oled)
    {
        _oled->clearDisplay();
        _oled->setCursor(0, 0);
        _oled->println("Tenergy32GateWay");
        _oled->println("Library Version:");
        _oled->println(_version);
        _oled->display();
    }
}

// ...existing code...
/***********************************************************************
 * FUNCTION:    begin
 * DESCRIPTION: Initializes the Tenergy32GateWay hardware.
 *              Sets up serial communication, pin modes, I2C bus, OLED display,
 *              LoRa module, RTC, and (optionally) Ethernet.
 * PARAMETERS:
 *      useEthernet - true = initialize Ethernet, false = skip Ethernet
 *      loraFreq - Frequency for LoRa communication (default is 443E6)
 * RETURNED:    true if all initializations are successful, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::begin(bool useEthernet, uint32_t loraFreq)
{
    // Initialize serial communication
    Serial.begin(115200);
    Serial.println("Initializing Tenergy32GateWay...");

    // Set up pin modes for buzzer, charger reset and built-in LED
    Serial.println("Setting pin modes...");
    pinMode(PIN_BUZZER, OUTPUT);
    digitalWrite(PIN_BUZZER, LOW);
    pinMode(PIN_BUILTIN_LED, OUTPUT);
    digitalWrite(PIN_BUILTIN_LED, LOW);

    // Set up I2C bus
    Serial.println("Initializing I2C...");
    initI2C();

    // Initialize OLED display
    Serial.println("Initializing OLED...");
    _oled = new Adafruit_SSD1306(128, 32, &Wire);
    if (!_oled->begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS))
    {
        Serial.println("Failed to initialize OLED");
        return false;
    }
    _oled->clearDisplay();
    _oled->setCursor(0, 0);
    _oled->println("OLED Init OK");
    _oled->display();
    vTaskDelay(1000);

    // Initialize LoRa module
    Serial.println("Initializing LoRa...");
    if (_oled)
    {
        _oled->clearDisplay();
        _oled->setTextSize(1);
        _oled->setTextColor(SSD1306_WHITE);
        _oled->setCursor(0, 0);
        _oled->println("Init LoRa...");
        _oled->display();
        vTaskDelay(1000);
    }
    SPI.begin(PIN_LORA_SCK, PIN_LORA_MISO, PIN_LORA_MOSI);
    LoRa.setPins(PIN_LORA_NSS, PIN_LORA_RESET, PIN_LORA_DIO0);
    if (!LoRa.begin(loraFreq))
    {
        Serial.println("Failed to initialize LoRa");
        if (_oled)
        {
            _oled->setCursor(0, 10);
            _oled->println("LoRa Init Fail");
            _oled->display();
            vTaskDelay(1000);
        }
        return false;
    }
    else
    {
        Serial.println("LoRa initialized successfully.");
        if (_oled)
        {
            _oled->setCursor(0, 10);
            _oled->println("LoRa OK");
            _oled->display();
            vTaskDelay(1000);
        }
    }

    // --- Ethernet Initialization (optional) ---
    if (useEthernet)
    {
        Serial.println("Initializing Ethernet...");
        uint8_t mac[6];
        esp_read_mac(mac, ESP_MAC_ETH); // ใช้ MAC ของ Ethernet
        uint8_t ip[4];
        uint8_t gw[4];
        uint8_t subnet[4];

        // ใช้งาน DHCP (useDHCP = true) หรือใช้ static (useDHCP = false)
        if (_oled)
        {
            _oled->clearDisplay();
            _oled->setCursor(0, 0);
            _oled->println("Ethernet Init...");
            _oled->display();
        }

        if (!initEternet(mac, ip, gw, subnet, true)) // เปลี่ยนพารามิเตอร์สุดท้ายเป็น false สำหรับ static
        {
            Serial.println("Ethernet initialization failed.");
            if (_oled)
            {
                _oled->setCursor(0, 10);
                _oled->println("Ethernet Fail");
                _oled->display();
                vTaskDelay(1000);
            }
        }
        else
        {
            Serial.println("Ethernet configured successfully.");
            Serial.print("IP: ");
            Serial.print(Ethernet.localIP()[0]);
            Serial.print(".");
            Serial.print(Ethernet.localIP()[1]);
            Serial.print(".");
            Serial.print(Ethernet.localIP()[2]);
            Serial.print(".");
            Serial.print(Ethernet.localIP()[3]);
            Serial.print("  GW: ");
            Serial.print(Ethernet.gatewayIP()[0]);
            Serial.print(".");
            Serial.print(Ethernet.gatewayIP()[1]);
            Serial.print(".");
            Serial.print(Ethernet.gatewayIP()[2]);
            Serial.print(".");
            Serial.print(Ethernet.gatewayIP()[3]);
            Serial.print("  SN: ");
            Serial.print(Ethernet.subnetMask()[0]);
            Serial.print(".");
            Serial.print(Ethernet.subnetMask()[1]);
            Serial.print(".");
            Serial.print(Ethernet.subnetMask()[2]);
            Serial.print(".");
            Serial.println(Ethernet.subnetMask()[3]);
            Serial.print("DNS: ");
            Serial.print(Ethernet.dnsServerIP()[0]);
            Serial.print(".");
            Serial.print(Ethernet.dnsServerIP()[1]);
            Serial.print(".");
            Serial.print(Ethernet.dnsServerIP()[2]);
            Serial.print(".");
            Serial.print(Ethernet.dnsServerIP()[3]);
            Serial.println();
            Serial.print("MAC: ");
            Serial.print(mac[0], HEX);
            Serial.print(":");
            Serial.print(mac[1], HEX);
            Serial.print(":");
            Serial.print(mac[2], HEX);
            Serial.print(":");
            Serial.print(mac[3], HEX);
            Serial.print(":");
            Serial.print(mac[4], HEX);
            Serial.print(":");
            Serial.print(mac[5], HEX);
            Serial.println();

            // แสดงผลข้อมูลบน OLED
            if (_oled)
            {
                _oled->setTextSize(1);
                _oled->setTextColor(SSD1306_WHITE);
                _oled->setCursor(0, 8);
                _oled->println("Ethernet OK");
                _oled->setCursor(0, 16);
                _oled->print("IP: ");
                _oled->println(Ethernet.localIP());
                _oled->setCursor(0, 25);
                _oled->print("GW: ");
                _oled->println(Ethernet.gatewayIP());
                _oled->setCursor(0, 32);
                _oled->print("SN: ");
                _oled->println(Ethernet.subnetMask());
                _oled->display();
                vTaskDelay(1000);
            }
        }
    }

    // RTC initialization
    Serial.println("Initializing RTC...");
    _rtc = new RTC_DS3231();
    if (!_rtc->begin())
    {
        Serial.println("Failed to initialize RTC");
        if (_oled)
        {
            _oled->setCursor(0, 10);
            _oled->println("RTC Init Fail");
            _oled->display();
        }
    }
    else
    {
        Serial.println("RTC initialized successfully.");
        DateTime now = _rtc->now();
        Serial.print("Current RTC Date/Time: ");
        Serial.print(now.year(), DEC);
        Serial.print('/');
        Serial.print(now.month(), DEC);
        Serial.print('/');
        Serial.print(now.day(), DEC);
        Serial.print(" ");
        Serial.print(now.hour(), DEC);
        Serial.print(':');
        Serial.print(now.minute(), DEC);
        Serial.print(':');
        Serial.println(now.second(), DEC);
        if (_oled)
        {
            _oled->clearDisplay();
            _oled->setCursor(0, 0);
            _oled->println("RTC Init OK");
            _oled->setCursor(0, 10);
            _oled->print(now.year());
            _oled->print('/');
            _oled->print(now.month());
            _oled->print('/');
            _oled->print(now.day());
            _oled->setCursor(0, 20);
            _oled->print(now.hour());
            _oled->print(':');
            _oled->print(now.minute());
            _oled->print(':');
            _oled->print(now.second());
            _oled->display();
        }
    }
    vTaskDelay(1000);

    // -------------------------------

    // Set up remaining peripheral pins: switches and relay pins
    Serial.println("Setting additional peripheral pin modes...");
    pinMode(PIN_DIP_SWITCH, INPUT);
    pinMode(PIN_SW1, INPUT_PULLUP);
    pinMode(PIN_SW2, INPUT_PULLUP);
    // Initialize relay pins
    pinMode(PIN_RELAY1, OUTPUT);
    digitalWrite(PIN_RELAY1, LOW);
    pinMode(PIN_RELAY2, OUTPUT);
    digitalWrite(PIN_RELAY2, LOW);
    pinMode(PIN_RELAY3, OUTPUT);
    digitalWrite(PIN_RELAY3, LOW);
    pinMode(PIN_RELAY4, OUTPUT);
    digitalWrite(PIN_RELAY4, LOW);

    Serial.println("Initialization complete.");
    if (_oled)
    {
        _oled->clearDisplay();
        _oled->setCursor(0, 0);
        _oled->println("Initialization");
        _oled->setCursor(0, 10);
        _oled->println("Complete");
        _oled->display();
        vTaskDelay(1000);
    }

    // Show the library version on the OLED
    showLibraryVersion();
    vTaskDelay(1000);

    beep(2, 100);
    return true;
}
// ...existing code...
/***********************************************************************
 * FUNCTION:    readSlideSwitch
 * DESCRIPTION: Reads the current state of the slide switch (DIP switch).
 * RETURNED:    true if the slide switch is ON, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::readSlideSwitch() { return digitalRead(PIN_DIP_SWITCH) == HIGH; }

/***********************************************************************
 * FUNCTION:    readSW1
 * DESCRIPTION: Reads the state of tactile switch SW1.
 * RETURNED:    true if SW1 is pressed, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::readSW1() { return digitalRead(PIN_SW1) == LOW; }

/***********************************************************************
 * FUNCTION:    readSW2
 * DESCRIPTION: Reads the state of tactile switch SW2.
 * RETURNED:    true if SW2 is pressed, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::readSW2() { return digitalRead(PIN_SW2) == LOW; }

/************************************
 * Relay 1 Functions
 ************************************/

/***********************************************************************
 * FUNCTION:    relay1On
 * DESCRIPTION: Turns Relay 1 ON.
 ***********************************************************************/
void Tenergy32GateWay::relay1On() { digitalWrite(PIN_RELAY1, HIGH); }

/***********************************************************************
 * FUNCTION:    relay1Off
 * DESCRIPTION: Turns Relay 1 OFF.
 ***********************************************************************/
void Tenergy32GateWay::relay1Off() { digitalWrite(PIN_RELAY1, LOW); }

/***********************************************************************
 * FUNCTION:    setRelay1
 * DESCRIPTION: Sets the state of Relay 1.
 * PARAMETERS:  state - true to turn on, false to turn off.
 ***********************************************************************/
void Tenergy32GateWay::setRelay1(bool state)
{
    if (state)
        relay1On();
    else
        relay1Off();
}
/***********************************************************************
 * FUNCTION:    relay1State
 * DESCRIPTION: Returns the current state of Relay 1.
 * RETURNED:    true if Relay 1 is ON, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::relay1State() { return digitalRead(PIN_RELAY1) == HIGH; }

/************************************
 * Relay 2 Functions
 ************************************/

/***********************************************************************
 * FUNCTION:    relay2On
 * DESCRIPTION: Turns Relay 2 ON.
 ***********************************************************************/
void Tenergy32GateWay::relay2On() { digitalWrite(PIN_RELAY2, HIGH); }

/***********************************************************************
 * FUNCTION:    relay2Off
 * DESCRIPTION: Turns Relay 2 OFF.
 ***********************************************************************/
void Tenergy32GateWay::relay2Off() { digitalWrite(PIN_RELAY2, LOW); }

/***********************************************************************
 * FUNCTION:    setRelay2
 * DESCRIPTION: Sets the state of Relay 2.
 * PARAMETERS:  state - true to turn on, false to turn off.
 ***********************************************************************/
void Tenergy32GateWay::setRelay2(bool state)
{
    if (state)
        relay2On();
    else
        relay2Off();
}

/***********************************************************************
 * FUNCTION:    relay2State
 * DESCRIPTION: Returns the current state of Relay 2.
 * RETURNED:    true if Relay 2 is ON, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::relay2State() { return digitalRead(PIN_RELAY2) == HIGH; }

/************************************
 * Relay 3 Functions
 ************************************/

/***********************************************************************
 * FUNCTION:    relay3On
 * DESCRIPTION: Turns Relay 3 ON.
 ***********************************************************************/
void Tenergy32GateWay::relay3On() { digitalWrite(PIN_RELAY3, HIGH); }

/***********************************************************************
 * FUNCTION:    relay3Off
 * DESCRIPTION: Turns Relay 3 OFF.
 ***********************************************************************/
void Tenergy32GateWay::relay3Off() { digitalWrite(PIN_RELAY3, LOW); }

/***********************************************************************
 * FUNCTION:    setRelay3
 * DESCRIPTION: Sets the state of Relay 3.
 * PARAMETERS:  state - true to turn on, false to turn off.
 ***********************************************************************/
void Tenergy32GateWay::setRelay3(bool state)
{
    if (state)
        relay3On();
    else
        relay3Off();
}

/***********************************************************************
 * FUNCTION:    relay3State
 * DESCRIPTION: Returns the current state of Relay 3.
 * RETURNED:    true if Relay 3 is ON, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::relay3State() { return digitalRead(PIN_RELAY3) == HIGH; }

/************************************
 * Relay 4 Functions
 ************************************/

/***********************************************************************
 * FUNCTION:    relay4On
 * DESCRIPTION: Turns Relay 4 ON.
 ***********************************************************************/
void Tenergy32GateWay::relay4On() { digitalWrite(PIN_RELAY4, HIGH); }

/***********************************************************************
 * FUNCTION:    relay4Off
 * DESCRIPTION: Turns Relay 4 OFF.
 ***********************************************************************/
void Tenergy32GateWay::relay4Off() { digitalWrite(PIN_RELAY4, LOW); }

/***********************************************************************
 * FUNCTION:    setRelay4
 * DESCRIPTION: Sets the state of Relay 4.
 * PARAMETERS:  state - true to turn on, false to turn off.
 ***********************************************************************/
void Tenergy32GateWay::setRelay4(bool state)
{
    if (state)
        relay4On();
    else
        relay4Off();
}
/***********************************************************************
 * FUNCTION:    relay4State
 * DESCRIPTION: Returns the current state of Relay 4.
 * RETURNED:    true if Relay 4 is ON, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::relay4State() { return digitalRead(PIN_RELAY4) == HIGH; }

/***********************************************************************
 * FUNCTION:    setbuildingLED
 * DESCRIPTION: Controls the built-in LED (used as building LED) ON/OFF.
 * PARAMETERS:  on - true to turn on, false to turn off.
 ***********************************************************************/
void Tenergy32GateWay::setbuildingLED(bool on)
{
    pinMode(PIN_BUILTIN_LED, OUTPUT);
    digitalWrite(PIN_BUILTIN_LED, on ? HIGH : LOW);
}

/***********************************************************************
 * FUNCTION:    beep
 * DESCRIPTION: Activates the buzzer to play a tone for a given duration.
 * PARAMETERS:  times - number of beeps,
 *              ms - duration of each beep in milliseconds.
 ***********************************************************************/
void Tenergy32GateWay::beep(uint8_t times, uint16_t ms)
{
    const uint8_t channel = 0;
    const uint32_t frequency = 2048; // Hz
    const uint8_t resolution = 8;
    ledcSetup(channel, frequency, resolution);
    ledcAttachPin(PIN_BUZZER, channel);
    for (uint8_t i = 0; i < times; i++)
    {
        ledcWriteTone(channel, frequency);
        delay(ms);
        ledcWriteTone(channel, 0);
        delay(50);
    }
}

/***********************************************************************
 * FUNCTION:    sendLoRa
 * DESCRIPTION: Sends data via LoRa communication.
 * PARAMETERS:  data - pointer to the data to send,
 *              len - length of the data.
 * RETURNED:    true if the packet was sent successfully, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::sendLoRa(const uint8_t *data, size_t len)
{
    LoRa.beginPacket();
    LoRa.write(data, len);
    return LoRa.endPacket() == 1;
}

/***********************************************************************
 * FUNCTION:    receiveLoRa
 * DESCRIPTION: Receives data via LoRa communication.
 * PARAMETERS:  buffer - pointer to the buffer to store data,
 *              maxLen - maximum length of the buffer,
 *              received - reference to store number of bytes received.
 * RETURNED:    true if data was received, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::receiveLoRa(uint8_t *buffer, size_t maxLen, int &received)
{
    int packetSize = LoRa.parsePacket();
    if (!packetSize)
        return false;
    received = LoRa.readBytes(buffer, min((int)maxLen, packetSize));
    return true;
}

/***********************************************************************
 * FUNCTION:    rs485TransmitMode
 * DESCRIPTION: Configures the RS485 module to either transmit or receive mode.
 * PARAMETERS:  enable - true to enable transmit mode, false to disable.
 ***********************************************************************/
void Tenergy32GateWay::rs485TransmitMode(bool enable)
{
    // TODO: Implement RS485 DE/RE control if available.
}

/***********************************************************************
 * FUNCTION:    sendRS485
 * DESCRIPTION: Sends data via RS485.
 * PARAMETERS:  data - pointer to the data,
 *              len - length of the data.
 * RETURNED:    Number of bytes sent.
 ***********************************************************************/
size_t Tenergy32GateWay::sendRS485(const uint8_t *data, size_t len)
{
    Serial2.begin(9600, SERIAL_8N1, PIN_RX_485, PIN_TX_485);
    return Serial2.write(data, len);
}

/***********************************************************************
 * FUNCTION:    receiveRS485
 * DESCRIPTION: Receives data via RS485.
 * PARAMETERS:  buffer - pointer to the buffer,
 *              maxLen - maximum length of the buffer.
 * RETURNED:    Number of bytes received.
 ***********************************************************************/
size_t Tenergy32GateWay::receiveRS485(uint8_t *buffer, size_t maxLen)
{
    Serial2.begin(9600, SERIAL_8N1, PIN_RX_485, PIN_TX_485);
    return Serial2.readBytes(buffer, maxLen);
}

/***********************************************************************
 * FUNCTION:    initI2C
 * DESCRIPTION: Initializes the I2C bus with specified SDA and SCL pins.
 ***********************************************************************/
void Tenergy32GateWay::initI2C()
{
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
}

/***********************************************************************
 * FUNCTION:    initOLED
 * DESCRIPTION: Initializes the OLED display using the I2C bus.
 * PARAMETERS:  addr - I2C address of the OLED display.
 * RETURNED:    true if initialization is successful, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::initOLED(uint8_t addr)
{
    _oled = new Adafruit_SSD1306(128, 32, &Wire);
    return _oled->begin(SSD1306_SWITCHCAPVCC, addr);
}

/***********************************************************************
 * FUNCTION:    displayOLED
 * DESCRIPTION: Displays a single line of text on the OLED display.
 * PARAMETERS:  text - pointer to the text string.
 ***********************************************************************/
void Tenergy32GateWay::displayOLED(const char *text)
{
    if (!_oled)
        return;
    _oled->clearDisplay();
    _oled->setTextSize(1);
    _oled->setTextColor(SSD1306_WHITE);
    _oled->setCursor(0, 0);
    _oled->println(text);
    _oled->display();
}

/***********************************************************************
 * FUNCTION:    displayOLEDInfo
 * DESCRIPTION: Displays preset information on the OLED (e.g., project title).
 ***********************************************************************/
void Tenergy32GateWay::displayOLEDInfo()
{
    if (!_oled)
        return;
    _oled->clearDisplay();
    _oled->setTextSize(1);
    _oled->setTextColor(SSD1306_WHITE);
    const char *line1 = "Tenergy Innovation";
    int16_t x, y;
    uint16_t w, h;
    _oled->getTextBounds(line1, 0, 0, &x, &y, &w, &h);
    int xpos = (128 - w) / 2;
    _oled->setCursor(xpos, 0);
    _oled->println(line1);
    const char *line2 = "Tenergy32GateWay IoT";
    _oled->getTextBounds(line2, 0, 0, &x, &y, &w, &h);
    xpos = (128 - w) / 2;
    _oled->setCursor(xpos, 10);
    _oled->println(line2);
    _oled->display();
}

/***********************************************************************
 * FUNCTION:    initLCD
 * DESCRIPTION: Initializes the LCD display using the I2C bus.
 * PARAMETERS:  addr - I2C address of the LCD display.
 * RETURNED:    true if initialization is successful.
 ***********************************************************************/
bool Tenergy32GateWay::initLCD(uint8_t addr)
{
    _lcd = new LiquidCrystal_I2C(addr, 20, 4);
    _lcd->init();
    _lcd->backlight();
    return true;
}

/***********************************************************************
 * FUNCTION:    displayLCD
 * DESCRIPTION: Displays text on the LCD at the specified column and row.
 * PARAMETERS:  text - pointer to text string,
 *              col - column position,
 *              row - row position.
 ***********************************************************************/
void Tenergy32GateWay::displayLCD(const char *text, uint8_t col, uint8_t row)
{
    if (!_lcd)
        return;
    _lcd->setCursor(col, row);
    _lcd->print(text);
}

/***********************************************************************
 * FUNCTION:    displayOLEDLines
 * DESCRIPTION: Displays up to four lines of text on the OLED display.
 * PARAMETERS:  line1, line2, line3, line4 - pointers to text strings.
 ***********************************************************************/
void Tenergy32GateWay::displayOLEDLines(const char *line1, const char *line2, const char *line3, const char *line4)
{
    if (!_oled)
        return;
    _oled->clearDisplay();
    _oled->setTextSize(1);
    _oled->setTextColor(SSD1306_WHITE);
    const int displayHeight = 32;
    const char *lines[4] = {line1, line2, line3, line4};
    int count = 0;
    for (int i = 0; i < 4; i++)
    {
        if (lines[i] && strlen(lines[i]) > 0)
            count++;
    }
    if (count == 0)
        return;
    int baseLineHeight = 8;
    int extraSpacing = 8;
    int lineHeight = baseLineHeight + extraSpacing;
    if (count * lineHeight > displayHeight)
        lineHeight = displayHeight / count;
    int offset = (displayHeight - (count * lineHeight)) / 2;
    int printed = 0;
    for (int i = 0; i < 4; i++)
    {
        if (lines[i] && strlen(lines[i]) > 0)
        {
            _oled->setCursor(0, offset + printed * lineHeight);
            _oled->println(lines[i]);
            printed++;
        }
    }
    _oled->display();
}

/***********************************************************************
 * FUNCTION:    marioSound
 * DESCRIPTION: Plays a melody (Mario theme) using the buzzer.
 ***********************************************************************/
#define NOTE_E7 2637
#define NOTE_C7 2093
#define NOTE_G7 3135
#define NOTE_G6 1568
void Tenergy32GateWay::marioSound()
{
    int melody[] = {NOTE_E7, NOTE_E7, 0, NOTE_E7,
                    0, NOTE_C7, NOTE_E7, 0,
                    NOTE_G7, 0, 0, 0,
                    NOTE_G6, 0, 0, 0};
    int noteDurations[] = {150, 150, 150, 150,
                           150, 150, 150, 150,
                           150, 150, 150, 150,
                           150, 150, 150, 150};
    const int numNotes = sizeof(melody) / sizeof(melody[0]);
    const uint8_t channel = 0;
    const uint8_t resolution = 8;
    ledcSetup(channel, 2000, resolution);
    ledcAttachPin(PIN_BUZZER, channel);
    for (int i = 0; i < numNotes; i++)
    {
        int noteDuration = noteDurations[i];
        if (melody[i] == 0)
        {
            ledcWriteTone(channel, 0);
            delay(noteDuration);
        }
        else
        {
            ledcWriteTone(channel, melody[i]);
            delay(noteDuration);
            ledcWriteTone(channel, 0);
            delay(50);
        }
    }
}

/***********************************************************************
 * FUNCTION:    angryBirdSound
 * DESCRIPTION: Plays a melody (Angry Birds theme) using the buzzer.
 ***********************************************************************/
void Tenergy32GateWay::angryBirdSound()
{
    int melody[] = {
        2200, 2100, 2000, 1900, 1800, 1700,
        1600, 1700, 1800, 1900, 2000, 2100,
        2200, 2200, 0, 2200,
        2100, 2100, 0, 2100,
        2000, 0, 2000, 2100};
    int noteDurations[] = {
        150, 150, 150, 150, 150, 150,
        150, 150, 150, 150, 150, 150,
        200, 200, 100, 200,
        150, 150, 100, 150,
        200, 100, 200, 150};
    const int numNotes = sizeof(melody) / sizeof(melody[0]);
    const uint8_t channel = 0;
    const uint8_t resolution = 8;
    ledcSetup(channel, 2000, resolution);
    ledcAttachPin(PIN_BUZZER, channel);
    for (int i = 0; i < numNotes; i++)
    {
        int duration = noteDurations[i];
        if (melody[i] == 0)
        {
            ledcWriteTone(channel, 0);
            delay(duration);
        }
        else
        {
            ledcWriteTone(channel, melody[i]);
            delay(duration);
            ledcWriteTone(channel, 0);
            delay(50);
        }
    }
}

/***********************************************************************
 * FUNCTION:    buildingLEDToggle
 * DESCRIPTION: Ticker callback for toggling the building LED state.
 ***********************************************************************/
void Tenergy32GateWay::buildingLEDToggle()
{
    _instance->_buildingState = !_instance->_buildingState;
    _instance->setbuildingLED(_instance->_buildingState);
}

/***********************************************************************
 * FUNCTION:    blinkbuildingLED
 * DESCRIPTION: Starts or stops blinking the building LED using Ticker callbacks.
 * PARAMETERS:  intervalMillis - duration of full blink cycle.
 *              If 0 is passed, blinking stops and LED is turned off.
 ***********************************************************************/
void Tenergy32GateWay::blinkbuildingLED(uint32_t intervalMillis)
{
    _tickerBuilding.detach();
    if (intervalMillis == 0)
    {
        setbuildingLED(false);
        _buildingState = false;
        return;
    }
    _tickerBuilding.attach_ms(intervalMillis / 2, buildingLEDToggle);
}

/***********************************************************************
 * FUNCTION:    setDateTime
 * DESCRIPTION: Sets the date and time in the RTC.
 * PARAMETERS:  _year - year (e.g., 2023),
 *              _month - month (1-12),
 *              _day - day of the month (1-31),
 *              _hour - hour (0-23),
 *              _min - minute (0-59),
 *              _sec - second (0-59).
 * RETURNED:    true if the date and time were set successfully, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::setDateTime(uint16_t _year, uint8_t _month, uint8_t _day, uint8_t _hour, uint8_t _min, uint8_t _sec)
{
    if (!_rtc)
        return false;
    // Set the RTC time (using DateTime from RTClib)
    _rtc->adjust(DateTime(_year, _month, _day, _hour, _min, _sec));
    return true;
}

/***********************************************************************
 * FUNCTION:    getTime
 * DESCRIPTION: Reads the current time from the RTC.
 * PARAMETERS:  _hour - reference to store hour,
 *              _min - reference to store minute,
 *              _sec - reference to store second.
 ***********************************************************************/
void Tenergy32GateWay::getTime(uint8_t &_hour, uint8_t &_min, uint8_t &_sec)
{
    if (!_rtc)
        return;
    DateTime now = _rtc->now();
    _hour = now.hour();
    _min = now.minute();
    _sec = now.second();
}

/***********************************************************************
 * FUNCTION:    getDate
 * DESCRIPTION: Reads the current date from the RTC.
 * PARAMETERS:  _year - reference to store year,
 *              _month - reference to store month,
 *              _day - reference to store day.
 ***********************************************************************/
void Tenergy32GateWay::getDate(uint16_t &_year, uint8_t &_month, uint8_t &_day)
{
    if (!_rtc)
        return;
    DateTime now = _rtc->now();
    _year = now.year();
    _month = now.month();
    _day = now.day();
}

/***********************************************************************
 * FUNCTION:    getDateTime
 * DESCRIPTION: Reads the current date and time from the RTC.
 * PARAMETERS:  _year - reference to store year,
 *              _month - reference to store month,
 *              _day - reference to store day,
 *              _hour - reference to store hour,
 *              _min - reference to store minute,
 *              _sec - reference to store second.
 ***********************************************************************/
void Tenergy32GateWay::getDateTime(uint16_t &_year, uint8_t &_month, uint8_t &_day, uint8_t &_hour, uint8_t &_min, uint8_t &_sec)
{
    if (!_rtc)
        return;
    DateTime now = _rtc->now();
    _year = now.year();
    _month = now.month();
    _day = now.day();
    _hour = now.hour();
    _min = now.minute();
    _sec = now.second();
}

/***********************************************************************
 * FUNCTION:    getDayOfTheWeekChar
 * DESCRIPTION: Returns the current day of the week as a string.
 * RETURNED:    Pointer to a string representing the day of the week.
 ***********************************************************************/
char *Tenergy32GateWay::getDayOfTheWeekChar(void)
{
    if (!_rtc)
        return nullptr;
    DateTime now = _rtc->now();
    uint8_t dow = now.dayOfTheWeek(); // 0 = Sunday, 1 = Monday, ...
    // ผมใช้ cast จาก const char* เป็น char* (โปรดระวังในกรณีที่นำไปแก้ไข)
    return (char *)_daysOfTheWeek[dow];
}

/***********************************************************************
 * FUNCTION:    getDayOfTheWeekNum
 * DESCRIPTION: Returns the current day of the week as a number.
 * RETURNED:    Number representing the day of the week (0 = Sunday, 1 = Monday, ...).
 ***********************************************************************/
uint8_t Tenergy32GateWay::getDayOfTheWeekNum(void)
{
    if (!_rtc)
        return 0;
    DateTime now = _rtc->now();
    return now.dayOfTheWeek();
}

/***********************************************************************
 * FUNCTION:    getTemperature
 * DESCRIPTION: Reads the current temperature from the RTC.
 * RETURNED:    Temperature in Celsius.
 ***********************************************************************/
float Tenergy32GateWay::getTemperature(void)
{
    if (!_rtc)
        return 0.0;
    return _rtc->getTemperature();
}

/***********************************************************************
 * FUNCTION:    getTimestamp
 * DESCRIPTION: Returns the current timestamp from the RTC.
 * RETURNED:    Timestamp as a 16-bit unsigned integer.
 ***********************************************************************/
uint16_t Tenergy32GateWay::getTimestamp(void)
{
    if (!_rtc)
        return 0;
    DateTime now = _rtc->now();
    return (uint16_t)(now.unixtime() % 65536);
}

/***********************************************************************
 * FUNCTION:    initEternet
 * DESCRIPTION: Initializes the Ethernet module with DHCP or static IP.
 * PARAMETERS:  mac - pointer to MAC address,
 *              ip - pointer to IP address,
 *              gw - pointer to gateway address,
 *              subnet - pointer to subnet mask,
 *              useDHCP - true for DHCP, false for static IP.
 * RETURNED:    true if initialization is successful, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::initEternet(uint8_t *mac, uint8_t *ip, uint8_t *gw, uint8_t *subnet, bool useDHCP)
{
    // ตั้งค่าขา CS สำหรับโมดูล W5500 และเริ่มต้น SPI
    pinMode(PIN_W5500_ENABLE, OUTPUT);
    digitalWrite(PIN_W5500_ENABLE, LOW);
    Ethernet.init(PIN_W5500_ENABLE);

    if (useDHCP)
    {
        // เริ่มต้น Ethernet โดยใช้ DHCP
        Ethernet.begin(mac);
        // รอให้การรับค่า DHCP เสร็จสิ้น
        delay(2000);

        // ตรวจสอบว่ามีการรับค่า IP จาก DHCP หรือไม่
        IPAddress local = Ethernet.localIP();
        if (local == IPAddress(0, 0, 0, 0))
        {
            Serial.println("DHCP configuration failed.");
            return false;
        }

        // คัดลอกค่า IP, gateway และ subnet mask ที่ได้จาก DHCP กลับไปยัง array ที่ส่งเข้ามา
        for (uint8_t i = 0; i < 4; i++)
        {
            ip[i] = local[i];
            gw[i] = Ethernet.gatewayIP()[i];
            subnet[i] = Ethernet.subnetMask()[i];
        }

        Serial.println("Ethernet configured successfully with DHCP.");
    }
    else
    {
        // สำหรับการตั้งค่า Static IP ต้องมีค่า IP, gateway และ subnet ผ่าน array ที่ส่งเข้ามา
        IPAddress ip_addr(ip[0], ip[1], ip[2], ip[3]);
        IPAddress gw_addr(gw[0], gw[1], gw[2], gw[3]);
        IPAddress subnet_addr(subnet[0], subnet[1], subnet[2], subnet[3]);
        // กำหนด DNS เริ่มต้น (เช่น Google DNS)
        IPAddress dns(8, 8, 8, 8);

        Ethernet.begin(mac, ip_addr, dns, gw_addr, subnet_addr);
        delay(1000);

        IPAddress local = Ethernet.localIP();
        if (local == IPAddress(0, 0, 0, 0))
        {
            Serial.println("Static IP configuration failed.");
            return false;
        }

        // คัดลอกค่า IP, gateway และ subnet mask กลับไปยัง array (อาจมีประโยชน์ในการตรวจสอบ)
        for (uint8_t i = 0; i < 4; i++)
        {
            ip[i] = local[i];
            gw[i] = Ethernet.gatewayIP()[i];
            subnet[i] = Ethernet.subnetMask()[i];
        }

        Serial.println("Ethernet configured successfully with static IP.");
    }
    return true;
}

/***********************************************************************
 * FUNCTION:    initEternet
 * DESCRIPTION: Initializes the Ethernet module with DHCP or static IP.
 * PARAMETERS:  mac - pointer to MAC address,
 *              useDHCP - true for DHCP, false for static IP.
 * RETURNED:    true if initialization is successful, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::initEternet(uint8_t *mac, bool useDHCP)
{
    // Set CS pin for W5500 and begin SPI
    pinMode(PIN_W5500_ENABLE, OUTPUT);
    digitalWrite(PIN_W5500_ENABLE, LOW);
    Ethernet.init(PIN_W5500_ENABLE);

    if (useDHCP)
    {
        // Start Ethernet using DHCP
        Ethernet.begin(mac);
        delay(2000);

        // Check if DHCP succeeded by verifying local IP
        IPAddress local = Ethernet.localIP();
        if (local == IPAddress(0, 0, 0, 0))
        {
            Serial.println("DHCP configuration failed.");
            return false;
        }

        Serial.println("Ethernet configured successfully with DHCP.");
    }
    else
    {
        // Static configuration requires IP, gw, and subnet arrays.
        Serial.println("Static IP configuration requires full parameters!");
        return false;
    }
    return true;
}

/***********************************************************************
 * FUNCTION:    initEternet
 * DESCRIPTION: Initializes the Ethernet module with DHCP or static IP.
 * PARAMETERS:  mac - pointer to MAC address,
 *              ip - pointer to IP address,
 *              useDHCP - true for DHCP, false for static IP.
 * RETURNED:    true if initialization is successful, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::initEternet(uint8_t *mac, uint8_t *ip, bool useDHCP)
{
    // Set CS pin for W5500 and begin SPI
    pinMode(PIN_W5500_ENABLE, OUTPUT);
    digitalWrite(PIN_W5500_ENABLE, LOW);
    Ethernet.init(PIN_W5500_ENABLE);

    if (useDHCP)
    {
        // Start Ethernet using DHCP
        Ethernet.begin(mac);
        delay(2000);

        // Check if DHCP succeeded by verifying local IP
        IPAddress local = Ethernet.localIP();
        if (local == IPAddress(0, 0, 0, 0))
        {
            Serial.println("DHCP configuration failed.");
            return false;
        }

        Serial.println("Ethernet configured successfully with DHCP.");
    }
    else
    {
        // Static configuration requires IP, gw, and subnet arrays.
        Serial.println("Static IP configuration requires full parameters!");
        return false;
    }
    return true;
}

/***********************************************************************
 * FUNCTION:    initEternet
 * DESCRIPTION: Initializes the Ethernet module with DHCP or static IP.
 * PARAMETERS:  mac - pointer to MAC address,
 *              ip - pointer to IP address,
 *              gw - pointer to gateway address,
 *              useDHCP - true for DHCP, false for static IP.
 * RETURNED:    true if initialization is successful, false otherwise.
 ***********************************************************************/
bool Tenergy32GateWay::initEternet(uint8_t *mac, uint8_t *ip, uint8_t *gw, bool useDHCP)
{
    // Set CS pin for W5500 and begin SPI
    pinMode(PIN_W5500_ENABLE, OUTPUT);
    digitalWrite(PIN_W5500_ENABLE, LOW);
    Ethernet.init(PIN_W5500_ENABLE);

    if (useDHCP)
    {
        // Start Ethernet using DHCP
        Ethernet.begin(mac);
        delay(2000);

        // Check if DHCP succeeded by verifying local IP
        IPAddress local = Ethernet.localIP();
        if (local == IPAddress(0, 0, 0, 0))
        {
            Serial.println("DHCP configuration failed.");
            return false;
        }

        Serial.println("Ethernet configured successfully with DHCP.");
    }
    else
    {
        // Static configuration requires IP, gw, and subnet arrays.
        Serial.println("Static IP configuration requires full parameters!");
        return false;
    }
    return true;
}