#include <SPI.h>
#include <MFRC522.h>
#include <IRremote.h>

#define RFID_SS_PIN 10
#define RFID_RST_PIN 9
#define MAGNET_PIN 8
#define FULL_BUTTON_PIN 7
#define TEMP_BUTTON_PIN 6
#define DINAMIC_PIN 5
#define DIN_TRANSISTOR_PIN 4
#define RGB_R_PIN 3
#define RGB_G_PIN 2
#define IR_PIN A0

String registeredCards[] = {}; //insert card UIDs to here

enum class LedColor {Red, Green, Off};
int outPins[] = {
    MAGNET_PIN,
    DINAMIC_PIN,
    DIN_TRANSISTOR_PIN,
    RGB_R_PIN,
    RGB_G_PIN
};
int inPins[] = {
    FULL_BUTTON_PIN,
    TEMP_BUTTON_PIN,
    IR_PIN
};
bool isMagnetLocked = false;

MFRC522 rfid(RFID_SS_PIN, RFID_RST_PIN);

void setup() {
    Serial.begin(9600);
    Serial.println("Starting initialization...");
    SPI.begin();
    IrReceiver.begin(IR_PIN);
    rfid.PCD_Init();

    for (int i: outPins) {
        pinMode(i, OUTPUT);
    }
    for (int i: inPins) {
        pinMode(i, INPUT);
    }

    Serial.println("Initialization complete");
}

void loop() {
    if (isButtonPressed(FULL_BUTTON_PIN)) {
        if (isMagnetLocked) {
            unlockMagnet();
        } else {
            lockMagnet();
        }
    }
    else if (isButtonPressed(TEMP_BUTTON_PIN)) {
        if (isMagnetLocked) {
            unlockMagnet(5);
        } else {
            lockMagnet();
        }
    }
    else if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
        if (isKeyValid(getCardUID())) {
            unlockMagnet(5);
        } else {
            playErrorSound();
        }
    }
    else if (IrReceiver.decode()) {
        if (IrReceiver.decodedIRData.command == 22) {
            unlockMagnet();
        } else if (IrReceiver.decodedIRData.command == 28) {
            unlockMagnet(5);
        } else if (IrReceiver.decodedIRData.command == 13) {
            lockMagnet();
        }
        IrReceiver.resume();
    }
}

bool isKeyValid(String key) {
    for (String i: registeredCards) {
        if (i == key) {
            return true;
        }
    }
    return false;
}

String getCardUID() {
    return decodeBytes(rfid.uid.uidByte, rfid.uid.size);
}

String decodeBytes(byte *buffer, byte bufferSize) {
    String str;
    for (byte i = 0; i < bufferSize; i++) {
        str += " " + String(buffer[i], HEX);
    }
    str.trim();
    str.toUpperCase();
    return str;
}

bool isButtonPressed(int buttonPin) {
    if (!digitalRead(buttonPin)) {
        return false;
    }

    delay(5);
    while (digitalRead(buttonPin)) {
        delay(3);
    }
    return true;
}

void lockMagnet() {
    isMagnetLocked = true;
    digitalWrite(MAGNET_PIN, HIGH);
    setLed(LedColor::Red);
    playSuccessSound();
}

void unlockMagnet() {
    isMagnetLocked = false;
    digitalWrite(MAGNET_PIN, LOW);
    setLed(LedColor::Green);
    playSuccessSound();
}

void unlockMagnet(int _seconds) {
    isMagnetLocked = false;
    digitalWrite(MAGNET_PIN, LOW);
    for (int i = 0; i < _seconds; i++) {
        setLed(LedColor::Green);
        tone(DINAMIC_PIN, 700);
        delay(300);
        setLed(LedColor::Off);
        noTone(DINAMIC_PIN);
        delay(700);
    }
    lockMagnet();
}

void playSuccessSound() {
    for (int i = 0; i < 2; i++) {
        tone(DINAMIC_PIN, 700);
        delay(200);
        noTone(DINAMIC_PIN);
        delay(100);
    }
}

void playErrorSound() {
    for (int i = 0; i < 2; i++) {
        tone(DINAMIC_PIN, 200);
        delay(200);
        noTone(DINAMIC_PIN);
        delay(100);
    }
}
 
void setLed(LedColor color) {
    if (color == LedColor::Red) {
        digitalWrite(RGB_G_PIN, LOW);
        digitalWrite(RGB_R_PIN, HIGH);
    } else if (color == LedColor::Green) {
        digitalWrite(RGB_R_PIN, LOW);
        digitalWrite(RGB_G_PIN, HIGH);
    } else {
        digitalWrite(RGB_G_PIN, LOW);
        digitalWrite(RGB_R_PIN, LOW);
    }
}