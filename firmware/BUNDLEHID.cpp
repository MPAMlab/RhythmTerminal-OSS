#include "BUNDLEHID.h"

BUNDLEHID_::BUNDLEHID_(void) {
    // Initialize state
    lastKeypadState = 0;
    lastNavState = 0;
    lastKeypadChange = 0;
    lastNavChange = 0;
    keypadEnabled = false;
    navEnabled = false;

    // Build HID Report Description
    uint8_t tempHidReportDescriptor[150];
    int hidReportDescriptorSize = 0;

    // USAGE_PAGE (Generic Desktop)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

    // USAGE (Joystick)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04;

    // COLLECTION (Application)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0xA1;
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

    // Keypad Report
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x85;  // REPORT_ID
    tempHidReportDescriptor[hidReportDescriptorSize++] = BUNDLE_KEYPAD_REPORT_ID;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;  // USAGE_PAGE (Button)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x19;  // USAGE_MINIMUM (1)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x29;  // USAGE_MAXIMUM (12)
    tempHidReportDescriptor[hidReportDescriptorSize++] = BUNDLE_KEYPAD_BUTTONS;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15;  // LOGICAL_MINIMUM (0)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x25;  // LOGICAL_MAXIMUM (1)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;  // REPORT_SIZE (1)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;  // REPORT_COUNT (12)
    tempHidReportDescriptor[hidReportDescriptorSize++] = BUNDLE_KEYPAD_BUTTONS;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;  // INPUT (Data,Var,Abs)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;

    // Padding to byte boundary
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;  // REPORT_SIZE (1)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;  // REPORT_COUNT (4)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x04;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;  // INPUT (Const,Var,Abs)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x03;

    // Nav Buttons Report
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x85;  // REPORT_ID
    tempHidReportDescriptor[hidReportDescriptorSize++] = BUNDLE_NAV_REPORT_ID;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x05;  // USAGE_PAGE (Button)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x09;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x19;  // USAGE_MINIMUM (1)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x29;  // USAGE_MAXIMUM (8)
    tempHidReportDescriptor[hidReportDescriptorSize++] = BUNDLE_NAV_BUTTONS;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x15;  // LOGICAL_MINIMUM (0)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x25;  // LOGICAL_MAXIMUM (1)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;  // REPORT_SIZE (1)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;  // REPORT_COUNT (8)
    tempHidReportDescriptor[hidReportDescriptorSize++] = BUNDLE_NAV_BUTTONS;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;  // INPUT (Data,Var,Abs)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x02;

    // Padding to byte boundary
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x75;  // REPORT_SIZE (1)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x01;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x95;  // REPORT_COUNT (0)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x00;

    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x81;  // INPUT (Const,Var,Abs)
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0x03;

    // END_COLLECTION
    tempHidReportDescriptor[hidReportDescriptorSize++] = 0xC0;

    // Create a copy of the HID Report Descriptor
    uint8_t *customHidReportDescriptor = new uint8_t[hidReportDescriptorSize];
    memcpy(customHidReportDescriptor, tempHidReportDescriptor, hidReportDescriptorSize);

    // Register HID Report Description
    DynamicHIDSubDescriptor *node = new DynamicHIDSubDescriptor(customHidReportDescriptor, hidReportDescriptorSize, false);
    DynamicHID().AppendDescriptor(node);
}

int BUNDLEHID_::begin(uint8_t row_pins[BUNDLE_KEYPAD_ROWS], uint8_t col_pins[BUNDLE_KEYPAD_COLS],
                       uint8_t nav_pins[BUNDLE_NAV_BUTTONS]) {
    // Store pin configuration
    if (row_pins && col_pins) {
        for (int i = 0; i < BUNDLE_KEYPAD_ROWS; i++) {
            rowPins[i] = row_pins[i];
            // Initialize as INPUT_PULLUP, will only change to OUTPUT during scanning
            pinMode(rowPins[i], INPUT_PULLUP);
        }
        for (int i = 0; i < BUNDLE_KEYPAD_COLS; i++) {
            colPins[i] = col_pins[i];
            pinMode(colPins[i], INPUT_PULLUP);
        }
        keypadEnabled = true;
    }

    if (nav_pins) {
        for (int i = 0; i < BUNDLE_NAV_BUTTONS; i++) {
            navPins[i] = nav_pins[i];
            pinMode(navPins[i], INPUT_PULLUP);
        }
        navEnabled = true;
    }

    return 0;
}

int BUNDLEHID_::update(void) {
    unsigned long currentTime = millis();
    bool stateChanged = false;

    // Scan keypad
    if (keypadEnabled) {
        uint16_t newKeypadState = 0;

        for (int row = 0; row < BUNDLE_KEYPAD_ROWS; row++) {
            pinMode(rowPins[row], OUTPUT);
            digitalWrite(rowPins[row], LOW);
            delayMicroseconds(5);  // Reduced from 10 to 5 microseconds

            for (int col = 0; col < BUNDLE_KEYPAD_COLS; col++) {
                if (digitalRead(colPins[col]) == LOW) {
                    int buttonIndex = row * BUNDLE_KEYPAD_COLS + col;
                    newKeypadState |= (1 << buttonIndex);
                }
            }

            digitalWrite(rowPins[row], HIGH);
            pinMode(rowPins[row], INPUT_PULLUP);
        }

        // Only send on state change to reduce USB traffic
        if (newKeypadState != lastKeypadState) {
            if (currentTime - lastKeypadChange > BUNDLE_DEBOUNCE_MS) {
                sendKeypadState(newKeypadState);
                lastKeypadState = newKeypadState;
                lastKeypadChange = currentTime;
                stateChanged = true;
            }
        }
    }

    // Scan navigation buttons
    if (navEnabled) {
        uint8_t newNavState = 0;

        for (int i = 0; i < BUNDLE_NAV_BUTTONS; i++) {
            if (digitalRead(navPins[i]) == LOW) {
                newNavState |= (1 << i);
            }
        }

        if (newNavState != lastNavState) {
            if (currentTime - lastNavChange > BUNDLE_DEBOUNCE_MS) {
                sendNavState(newNavState);
                lastNavState = newNavState;
                lastNavChange = currentTime;
                stateChanged = true;
            }
        }
    }

    return stateChanged ? 1 : 0;
}

int BUNDLEHID_::sendKeypadState(uint16_t button_state) {
    Serial.print("Keypad state: 0x");
    Serial.println(button_state, HEX);

    uint8_t data[2];
    data[0] = button_state & 0xFF;
    data[1] = (button_state >> 8) & 0xFF;

    return DynamicHID().SendReport(BUNDLE_KEYPAD_REPORT_ID, data, 2);
}

int BUNDLEHID_::sendNavState(uint8_t button_state) {
    Serial.print("Nav state: 0x");
    Serial.println(button_state, HEX);

    return DynamicHID().SendReport(BUNDLE_NAV_REPORT_ID, &button_state, 1);
}

BUNDLEHID_ BUNDLEHID;