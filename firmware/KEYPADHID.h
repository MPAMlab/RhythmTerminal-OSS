#include "HID.h"

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 3
#define KEYPAD_BUTTONS 12

#define KEYPAD_REPORT_ID 5
#define KEYPAD_DEBOUNCE_MS 5

class KEYPADHID_ : public PluggableUSBModule {
    public:
        KEYPADHID_(void);
        int begin(uint8_t row_pins[KEYPAD_ROWS], uint8_t col_pins[KEYPAD_COLS], const char keymap[KEYPAD_ROWS][KEYPAD_COLS]);
        int update(void);
        int send_state(uint16_t button_state);
    protected:
        uint8_t epType[1];
        uint8_t rowPins[KEYPAD_ROWS];
        uint8_t colPins[KEYPAD_COLS];
        const char* keyMap;
        uint16_t lastState;
        unsigned long lastUpdateTime[KEYPAD_BUTTONS];
        bool currentState[KEYPAD_BUTTONS];

        int getInterface(uint8_t* interface_count);
        int getDescriptor(USBSetup& setup);
        bool setup(USBSetup& setup);
};

extern KEYPADHID_ KEYPADHID;