#include <Arduino.h>
#include <EEPROM.h>  // We are going to read and write PICC's UIDs from/to EEPROM
#include <SPI.h>     // RC522 Module uses SPI protocol
#include <MFRC522.h> // Library for Mifare RC522 Devices
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RELAY 7
#define WIPE_BUTTON 6
#define WIPE_BUTTON_ALWAYS_ON 8
#define SS_PIN 10
#define RST_PIN 9

typedef char user_t[8];

user_t master;
user_t bois[8];

MFRC522 mfrc(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

void loadUser(char index) {
    for (int i = 0; i < 8; i++) {
        bois[index][i] = EEPROM.read(8 * index + 9);
    }
}

void loadUsers() {
    char count = EEPROM.read(0);

    for (int i = 0; i < 8; i++) {
        master[i] = EEPROM.read(i + 1);
    }
    
    for (int i = 0; i < count; i++) {
        loadUser(i);
    }
}

void setup() {
    pinMode(WIPE_BUTTON, INPUT);
    pinMode(WIPE_BUTTON_ALWAYS_ON, OUTPUT);
    digitalWrite(WIPE_BUTTON_ALWAYS_ON, HIGH);
    digitalWrite(WIPE_BUTTON, LOW);

    Serial.begin(115200);

    SPI.begin();
    mfrc.PCD_Init();

    lcd.init();
    lcd.backlight();
    lcd.print("AAAAAAAAAAAAAAAA");

    mfrc.PCD_SetAntennaGain(mfrc.RxGain_max);
}


void loop() {
    
}