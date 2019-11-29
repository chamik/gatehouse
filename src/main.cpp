#include <Arduino.h>
#include <EEPROM.h>  // Definetly did not copy this code
#include <SPI.h>     // Or this
#include <MFRC522.h> // Or this
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RELAY 7
#define OPEN_BUTTON 6
#define WIPE_BUTTON_ALWAYS_ON 8
#define SS_PIN 10
#define RST_PIN 9
#define BUTTON_PIN A0
#define WELCOME_MESSAGE " OH HELLO THERE"

typedef uint8_t user_t[4];

enum CardType {
    Unauthorized,
    Boi,
    Master
};

user_t master;
user_t *bois;
int n_users = 0;

// Init RFID reader and lcd
MFRC522 mfrc(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Load user from specified place in EEPROM
void loadUser(char index) {
    for (int i = 0; i < 4; i++) {
        bois[index][i] = EEPROM.read(5 + (4 * index) + i);
    }
}

// Load all users from EEPROM
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


// Check the type of the card
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

// Senpai uwu
void masterMode() {
    Serial.println("I wasn't expecting you so early senpai *blush*"); // I want to fucking die
    lcd.clear();
    lcd.print("EPIC MASTER MODE");

    // load new cards and do some other boring stuff
    while (true) {
        // Is new card present?
        if (!mfrc.PICC_IsNewCardPresent())
            continue;

        // Is there a card?
        if (!mfrc.PICC_ReadCardSerial())
            continue;

        // Get info about the card
        CardType cardType = checkCard();

        // Card is known, delete it from the EEPROM
        if (cardType == Boi) {
            for (int i = 0; i < n_users; i++)
            {
                if (mfrc.uid.uidByte[0] == bois[i][0] &&
                    mfrc.uid.uidByte[1] == bois[i][1] &&
                    mfrc.uid.uidByte[2] == bois[i][2] &&
                    mfrc.uid.uidByte[3] == bois[i][3]) {

                    bois[i][0] = bois[n_users - 1][0];
                    bois[i][1] = bois[n_users - 1][1];
                    bois[i][2] = bois[n_users - 1][2];
                    bois[i][3] = bois[n_users - 1][3];

                    EEPROM.write(5 + 4 * i + 0, bois[n_users - 1][0]);
                    EEPROM.write(5 + 4 * i + 1, bois[n_users - 1][1]);
                    EEPROM.write(5 + 4 * i + 2, bois[n_users - 1][2]);
                    EEPROM.write(5 + 4 * i + 3, bois[n_users - 1][3]);

                    break;
                }
            }
            
            n_users -= 1;
            EEPROM.write(0, n_users);

            lcd.clear();
            Serial.print("I'll get rid of him... *loads gun with religious intent*");
            lcd.print("  BEGONE THOT!");
            delay(500);

        // Card is not known, save it into the EEPROM!
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
            
            Serial.println("new boi added xd");
            lcd.clear();
            lcd.print("NEW BOI ADDED XD");
            delay(500);

        // The card is master, leave master mode
        } else if (cardType == Master) {
            Serial.println("so long master uwu...");
            lcd.clear();
            lcd.print(" BYE SENPAI UWU");
            delay(1000);
            return;
        }

        lcd.clear();
        lcd.print("EPIC MASTER MODE");
    }
}

void setup() {
    // Init all the things
    pinMode(OPEN_BUTTON, OUTPUT);
    pinMode(WIPE_BUTTON_ALWAYS_ON, OUTPUT);
    pinMode(RELAY, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);

    digitalWrite(WIPE_BUTTON_ALWAYS_ON, HIGH);
    digitalWrite(OUTPUT, HIGH);
    digitalWrite(RELAY, HIGH);
    digitalWrite(OPEN_BUTTON, HIGH);


    Serial.begin(115200);

    SPI.begin();
    
    loadUsers();
    Serial.println("i am ready to serve you owo");

    lcd.init();
    lcd.backlight();
    lcd.home();
    lcd.print(WELCOME_MESSAGE);
    Serial.print("Let the games begin!");

    mfrc.PCD_Init();
    mfrc.PCD_SetAntennaGain(mfrc.RxGain_max);
}


void loop() {
    // Check if open button is pressed, if yes opend door
    Serial.println(analogRead(BUTTON_PIN));
    if (analogRead(BUTTON_PIN) < 5) {
        openDoor();
        return;
    }

    // Is new card present?
    if (!mfrc.PICC_IsNewCardPresent()) 
        return;

    // Is there a card?
    if (!mfrc.PICC_ReadCardSerial())
        return;

    // Get info about the card
    CardType cardType = checkCard();

    // If master, enter master mode
    if (cardType == Master) {
        masterMode();

    // If known, open door
    } else if (cardType == Boi) {
        Serial.println("i know the taste of this one uwu");
        lcd.clear();
        lcd.print(" WELCOME HUMAN!");
        openDoor();

    // Not known, begone thot!
    } else {
        Serial.println("Impossible. Perhaps the archives are incomplete?");
        lcd.clear();
        lcd.print("     OPENED");
        lcd.setCursor(0, 1);
        lcd.print(" jk go away lol");
        lcd.home();
        delay(1000);
    }

    lcd.clear();
    lcd.print(WELCOME_MESSAGE);
}

// Open Sesame!
void openDoor() {
    digitalWrite(RELAY, LOW);
    delay(3000);
    digitalWrite(RELAY, HIGH);
}
