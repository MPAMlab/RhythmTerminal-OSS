#include "src/DynamicHID/DynamicHID.h"

#define BUNDLE_KEYPAD_ROWS 4
#define BUNDLE_KEYPAD_COLS 3
#define BUNDLE_KEYPAD_BUTTONS 12
#define BUNDLE_NAV_BUTTONS 8

#define BUNDLE_KEYPAD_REPORT_ID 3
#define BUNDLE_NAV_REPORT_ID 4
#define BUNDLE_DEBOUNCE_MS 5

class BUNDLEHID_ {
    public:
        BUNDLEHID_(void);
        int begin(uint8_t row_pins[BUNDLE_KEYPAD_ROWS], uint8_t col_pins[BUNDLE_KEYPAD_COLS],
                  uint8_t nav_pins[BUNDLE_NAV_BUTTONS]);
        int update(void);
        int sendKeypadState(uint16_t button_state);
        int sendNavState(uint8_t button_state);
    private:
        uint8_t rowPins[BUNDLE_KEYPAD_ROWS];
        uint8_t colPins[BUNDLE_KEYPAD_COLS];
        uint8_t navPins[BUNDLE_NAV_BUTTONS];
        uint16_t lastKeypadState;
        uint8_t lastNavState;
        unsigned long lastKeypadChange;
        unsigned long lastNavChange;
        bool keypadEnabled;
        bool navEnabled;
};

extern BUNDLEHID_ BUNDLEHID;