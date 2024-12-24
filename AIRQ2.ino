#include <Wire.h>
#include <Adafruit_SHT31.h>       // SHT30 sensor library
#include "MHZ19.h"                // MH-Z19 CO2 sensor library
#include <U8g2lib.h>              // U8G2 OLED display library
#include <SoftwareSerial.h>       

#define RX_PIN 10
#define TX_PIN 11
#define BAUDRATE 9600
#define BUZZER_PIN 9              
MHZ19 myMHZ19;
Adafruit_SHT31 sht30 = Adafruit_SHT31();
SoftwareSerial mySerial(RX_PIN, TX_PIN);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

unsigned long getDataTimer = 0;
unsigned long buzzerTimer = 0;
bool buzzerOn = false;

void showBootupAnimation() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(35, 30, "Loading...");
    u8g2.sendBuffer();
    delay(300);

    for (int i = 0; i <= 100; i += 10) {
        u8g2.clearBuffer();
        u8g2.drawStr(35, 30, "  presents...");
        u8g2.drawStr(25, 20, "GROUP SEVEN");
        u8g2.drawFrame(13, 40, 108, 10);  
        u8g2.drawBox(13, 40, i + 8, 10);  
        u8g2.sendBuffer();
        delay(250);
    }
    delay(500);
}

void displayError(const char* errorMessage) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_ncenB08_tr);
    u8g2.drawStr(3, 30, "ERROR:");          
    u8g2.drawStr(3, 50, errorMessage);       
    u8g2.sendBuffer();
    while (true);  
}

void setup() {
    Serial.begin(9600);
    mySerial.begin(BAUDRATE);
    myMHZ19.begin(mySerial);
    myMHZ19.autoCalibration();
    
    pinMode(BUZZER_PIN, OUTPUT);
    noTone(BUZZER_PIN);  

    if (!sht30.begin(0x44)) {
        Serial.println("Couldn't find SHT30 sensor!");
        displayError("SHT30 Not Found");
    }
    
    u8g2.begin();
    showBootupAnimation();
}

void loop() {
    if (millis() - getDataTimer >= 2000) {
        getDataTimer = millis();
    
        int CO2 = myMHZ19.getCO2();
        int8_t TempMHZ19 = myMHZ19.getTemperature();
        
        float tempSHT30 = sht30.readTemperature();
        float humidity = sht30.readHumidity();
        
        if (isnan(tempSHT30) || isnan(humidity)) {
            displayError("SHT30 Read Err");
        }
        if (CO2 == -1) {
            displayError("MH-Z19 Read Err");
        }

        if (CO2 > 2500) {
            if (millis() - buzzerTimer >= 500) {  
                buzzerTimer = millis();
                if (buzzerOn) {
                    noTone(BUZZER_PIN);           
                } else {
                    tone(BUZZER_PIN, 1500, 200);  
                }
                buzzerOn = !buzzerOn;
            }
        } else {
            noTone(BUZZER_PIN);                  
        }

        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_ncenB14_tr);
        u8g2.setCursor(3, 15);                
        u8g2.print("Env Monitor");

        u8g2.setFont(u8g2_font_6x10_tf); 
        u8g2.setCursor(3, 30);                
        u8g2.print("CO2: "); u8g2.print(CO2); u8g2.print(" ppm");
        u8g2.setCursor(3, 40);                
        u8g2.print("Internal Temp: "); u8g2.print(TempMHZ19); u8g2.print(" C");
        u8g2.setCursor(3, 50);                
        u8g2.print("Temperature: "); u8g2.print(tempSHT30); u8g2.print(" C");
        u8g2.setCursor(3, 60);                
        u8g2.print("Humidity: "); u8g2.print(humidity); u8g2.print(" %");
        u8g2.sendBuffer();
    }
}
