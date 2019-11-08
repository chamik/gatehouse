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

typedef uint8_t user_t[4];

enum CardType {
    Unauthorized,
    Boi,
    Master
};

user_t master;
user_t *bois;
int n_users = 0;

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

CardType checkCard() {
    if (mfrc.uid.uidByte[0] == master[0] && 
        mfrc.uid.uidByte[1] == master[1] &&
        mfrc.uid.uidByte[2] == master[2] &&
        mfrc.uid.uidByte[3] == master[3])
        return Master;

    for (int i = 0; i < n_users; i++)
    {
        if (mfrc.uid.uidByte[0] == bois[i][0] &&
            mfrc.uid.uidByte[1] == bois[i][1] &&
            mfrc.uid.uidByte[2] == bois[i][2] &&
            mfrc.uid.uidByte[3] == bois[i][3])
            return Boi;
    }
    
    return Unauthorized;
}

void masterMode() {
    Serial.println("boi we got master");
    lcd.print("MASTER");

    // load new cards and do some other boring stuff
}

void setup() {
    pinMode(WIPE_BUTTON, INPUT);
    pinMode(WIPE_BUTTON_ALWAYS_ON, OUTPUT);
    digitalWrite(WIPE_BUTTON_ALWAYS_ON, HIGH);
    digitalWrite(WIPE_BUTTON, LOW);

    Serial.begin(115200);

    SPI.begin();
    mfrc.PCD_Init();
    master[0] = 0xA0;
    master[1] = 0xDD;
    master[2] = 0x49;
    master[3] = 0x1A;

    lcd.init();
    lcd.backlight();
    lcd.print("AAAAAAAAAAAAAAAA");

    mfrc.PCD_SetAntennaGain(mfrc.RxGain_max);
}


void loop() {
    if (!mfrc.PICC_IsNewCardPresent())
        return;

    if (!mfrc.PICC_ReadCardSerial())
        return;

    CardType cardType = checkCard();
    if (cardType == Master) {
        masterMode();
    } else if (cardType == Boi) {
        Serial.println("boi we got some boi");
        lcd.print("Welcome you motherfucker");
    } else {
        Serial.println("boi we got some špión here");
        lcd.print("go out you špión");
    }
}