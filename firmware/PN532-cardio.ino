#include "Config.h"
#include <Wire.h>
#include "src/PN532/PN532_I2C.h"
#include "src/PN532/PN532.h"
#include "src/Cardio.h"
#include "BUNDLEHID.h"

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

Cardio_ Cardio;

#if WITH_KEYPAD == 1
  /* Keypad declarations with new layout according to requirements */
  const char numpad[4][3] = {
   {'2', '1', '3'},    // GP14 row
   {'0', '\337', ','}, // GP18 row
   {'8', '7', '9'},    // GP20 row
   {'5', '4', '6'}     // GP21 row
  };

  uint8_t rowPins[4] = {PIN_ROW1, PIN_ROW2, PIN_ROW3, PIN_ROW4};
  uint8_t colPins[3] = {PIN_COL1, PIN_COL2, PIN_COL3};
#endif

#if WITH_NAVBUTTONS == 1
  uint8_t navPins[8] = {
    PIN_NAV1, PIN_NAV2, PIN_NAV3, PIN_NAV4,
    PIN_NAV5, PIN_NAV6, PIN_NAV7, PIN_NAV8
  };
#endif
 
void setup() {
  Serial.begin(115200);
  Serial.println("PN532-cardio starting...");

  /* Initialize BUNDLEHID */
#if WITH_KEYPAD == 1 || WITH_NAVBUTTONS == 1
  Serial.println("Initializing BUNDLEHID...");
  #if WITH_KEYPAD == 1 && WITH_NAVBUTTONS == 1
    BUNDLEHID.begin(rowPins, colPins, navPins);
    Serial.println("BUNDLEHID initialized with keypad and nav buttons");
  #elif WITH_KEYPAD == 1
    BUNDLEHID.begin(rowPins, colPins, NULL);
    Serial.println("BUNDLEHID initialized with keypad only");
  #elif WITH_NAVBUTTONS == 1
    BUNDLEHID.begin(NULL, NULL, navPins);
    Serial.println("BUNDLEHID initialized with nav buttons only");
  #endif
#endif

/* NFC */
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata)
  {
Serial.print("Didn't find PN53x board");
    while (1) {delay(10);};      // halt
  }

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();

//  memset(_prevIDm, 0, 8);

  Cardio.begin(false);
  Serial.println("Cardio initialized");
}

unsigned long lastReport = 0;
uint16_t cardBusy = 0;
unsigned long lastPollTime = 0;
unsigned long pollCount = 0;

// read cards loop
void loop() {
  /* Update BUNDLEHID - always call first for maximum responsiveness */
#if WITH_KEYPAD == 1 || WITH_NAVBUTTONS == 1
  BUNDLEHID.update();

  // Track polling rate
  pollCount++;
  if (millis() - lastPollTime >= 1000) {
    Serial.print("Polling rate: ");
    Serial.print(pollCount);
    Serial.println(" Hz");
    pollCount = 0;
    lastPollTime = millis();
  }
#endif

  /* NFC */
  if (millis()-lastReport < cardBusy) return;

  cardBusy = 0;
  uint8_t uid[8] = {0,0,0,0,0,0,0,0};
  uint8_t hid_data[8] = {0,0,0,0,0,0,0,0};
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  // check for FeliCa card
  uint8_t ret;
  uint16_t systemCode = 0xFFFF;
  uint8_t requestCode = 0x01;       // System Code request
  uint8_t idm[8];
  uint8_t pmm[8];
  uint16_t systemCodeResponse;

  // Check for FeliCa card
  ret = nfc.felica_Polling(systemCode, requestCode, idm, pmm, &systemCodeResponse, NFC_FELICA_TIMEOUT);

    if (ret == 1) {
      Serial.print("FeliCa card detected - IDm: ");
      for (int i = 0; i < 8; i++) {
        if (idm[i] < 0x10) Serial.print("0");
        Serial.print(idm[i], HEX);
      }
      Serial.print(" SystemCode: 0x");
      Serial.println(systemCodeResponse, HEX);

      Cardio.setUID(2, idm);
      Cardio.sendState();
      lastReport = millis();
      cardBusy = 3000;
      uidLength = 0;
      return;
    }

   // check for ISO14443 card
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, NFC_ISO14443_TIMEOUT)) {
      Serial.print("ISO14443A card detected - UID: ");
      for (int i = 0; i < uidLength; i++) {
        if (uid[i] < 0x10) Serial.print("0");
        Serial.print(uid[i], HEX);
      }
      Serial.print(" (Length: ");
      Serial.print(uidLength);
      Serial.println(" bytes)");

      // Apply UID masking for compatibility (like PN5180 implementation)
      uid[0] &= 0x0F; // some games won't accept cards with a first byte higher than 0x0F

      for (int i=0; i<8; i++) {
        hid_data[i] = uid[i%uidLength];
      }
      // Report as type 2 (FeliCa) for maximum compatibility
      Cardio.setUID(2, hid_data);
      Cardio.sendState();
      lastReport = millis();
      cardBusy = 3000;
      return;
    }
  // no card detected - reduced delay for better keypad responsiveness
  lastReport = millis();
  cardBusy = NFC_NO_CARD_DELAY;  // Delay between NFC polling cycles
}
