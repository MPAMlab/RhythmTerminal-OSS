#include "HID.h"

#define NAV_BUTTONS 8
#define NAV_BUTTON_PADDING (8 - (NAV_BUTTONS % 8))

#define NAV_REPORT_ID 3
#define NAV_DEBOUNCE_MS 5

class NAVHID_ : public PluggableUSBModule {
    public:
        NAVHID_(void);
        int begin(uint8_t button_pins[NAV_BUTTONS]);
        int update(void);
        int send_state(uint8_t button_state);
    protected:
        uint8_t epType[1];
        uint8_t buttonPins[NAV_BUTTONS];
        uint8_t lastState;
        unsigned long lastUpdateTime[NAV_BUTTONS];
        bool currentState[NAV_BUTTONS];

        int getInterface(uint8_t* interface_count);
        int getDescriptor(USBSetup& setup);
        bool setup(USBSetup& setup);
};

extern NAVHID_ NAVHID;