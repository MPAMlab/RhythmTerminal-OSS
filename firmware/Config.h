#ifndef CONFIG_h
#define CONFIG_h

/* ISO14443 support (for older Aime/Nesica/BANAPASSPORT cards... reader will pretend it was a FeliCa for maximum cardio compatibility) */
#define WITH_ISO14443 1

/* Use a matrix keypad with new HID implementation */
#define WITH_KEYPAD 1
  /* New keypad pinout - Rows: GP14, GP18, GP20, GP21; Columns: GP16, GP15, GP19 */
  #define PIN_ROW1 14    // GP14
  #define PIN_ROW2 18    // GP18
  #define PIN_ROW3 20    // GP20
  #define PIN_ROW4 21    // GP21
  #define PIN_COL1 16    // GP16
  #define PIN_COL2 15    // GP15
  #define PIN_COL3 19    // GP19

/* Navigation buttons (8 independent joystick buttons) */
#define WITH_NAVBUTTONS 1
  #define PIN_NAV1 1     // GP1
  #define PIN_NAV2 4     // GP4
  #define PIN_NAV3 5     // GP5
  #define PIN_NAV4 6     // GP6
  #define PIN_NAV5 7     // GP7
  #define PIN_NAV6 8     // GP8
  #define PIN_NAV7 9     // GP9
  #define PIN_NAV8 10    // GP10

/* Player ID (1 or 2) */
#define CARDIO_ID 1
/* Enable custom VID/PID (will require to hold reset button to reflash firmware) */
#define CUSTOM_VIDPID 1

#endif