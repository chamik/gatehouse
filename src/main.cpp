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
    for (int i = 0; i < 4; i++) {
        bois[index][i] = EEPROM.read(5 + (4 * index) + i);
    }
}

void loadUsers() {
    n_users = EEPROM.read(0);

    bois = (user_t*) malloc(n_users * sizeof(user_t));

    for (int i = 0; i < 4; i++) {
        master[i] = EEPROM.read(i + 1);
    }

    for (int i = 0; i < n_users; i++) {
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
    lcd.clear();
    lcd.print("     MASTER");
    delay(2000);

    // load new cards and do some other boring stuff
    while (true) {
        if (!mfrc.PICC_IsNewCardPresent())
            continue;

        if (!mfrc.PICC_ReadCardSerial())
            continue;

        CardType cardType = checkCard();
        if (cardType == Boi) {
            // delete the user
        } else if (cardType == Unauthorized) {
            bois = (user_t*) realloc(bois, (n_users + 1) * sizeof(user_t));

            bois[n_users][0] = mfrc.uid.uidByte[0];
            bois[n_users][1] = mfrc.uid.uidByte[1];
            bois[n_users][2] = mfrc.uid.uidByte[2];
            bois[n_users][3] = mfrc.uid.uidByte[3];

            EEPROM.write(5 + n_users * 4 + 0, bois[n_users][0]);
            EEPROM.write(5 + n_users * 4 + 1, bois[n_users][1]);
            EEPROM.write(5 + n_users * 4 + 2, bois[n_users][2]);
            EEPROM.write(5 + n_users * 4 + 3, bois[n_users][3]);

            n_users++;

            EEPROM.write(0, n_users);
            
        } else if (cardType == Master) {
            Serial.println("exitting master mode");
            delay(2000);
            return;
        }
    }
}

void setup() {
    pinMode(WIPE_BUTTON, INPUT);
    pinMode(WIPE_BUTTON_ALWAYS_ON, OUTPUT);
    pinMode(RELAY, OUTPUT);
    digitalWrite(WIPE_BUTTON_ALWAYS_ON, HIGH);
    digitalWrite(WIPE_BUTTON, LOW);


    Serial.begin(115200);
    delay(2000);
    Serial.println("boii");

    SPI.begin();
    
    loadUsers();
    Serial.println("users loaded");

    lcd.init();
    lcd.backlight();
    lcd.print("AAAAAAAAAAAAAAAA");

    mfrc.PCD_Init();
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

        digitalWrite(RELAY, HIGH);
        delay(3000);
        digitalWrite(RELAY, LOW);
    } else {
        Serial.println("boi we got some špión here");
        lcd.print("go out you špión");
    }
}
