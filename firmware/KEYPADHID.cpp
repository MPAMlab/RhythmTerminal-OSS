#include "KEYPADHID.h"

static const uint8_t PROGMEM hid_report[] = {
    0x05, 0x01,                      // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                      // USAGE (Keyboard)
    0xa1, 0x01,                      // COLLECTION (Application)

    0x85, KEYPAD_REPORT_ID,          //   REPORT_ID (KEYPAD_REPORT_ID)

    /* Keypad as individual buttons */
    0x05, 0x09,                      //   USAGE_PAGE (Button)
    0x19, 0x01,                      //   USAGE_MINIMUM (Button 1)
    0x29, KEYPAD_BUTTONS,            //   USAGE_MAXIMUM (Button 12)
    0x15, 0x00,                      //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                      //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                      //   REPORT_SIZE (1)
    0x95, KEYPAD_BUTTONS,            //   REPORT_COUNT (12)
    0x81, 0x02,                      //   INPUT (Data,Var,Abs)

    /* Padding to byte boundary */
    0x75, 0x01,                      //   REPORT_SIZE (1)
    0x95, 4,                         //   REPORT_COUNT (4)
    0x81, 0x03,                      //   INPUT (Cnst,Var,Abs)

    0xc0                             // END_COLLECTION
};

KEYPADHID_::KEYPADHID_(void) : PluggableUSBModule(1, 1, epType) {
    epType[0] = EP_TYPE_INTERRUPT_IN;
    PluggableUSB().plug(this);

    // Initialize state
    lastState = 0;
    for (int i = 0; i < KEYPAD_BUTTONS; i++) {
        currentState[i] = false;
        lastUpdateTime[i] = 0;
    }
}

int KEYPADHID_::begin(uint8_t row_pins[KEYPAD_ROWS], uint8_t col_pins[KEYPAD_COLS], const char keymap[KEYPAD_ROWS][KEYPAD_COLS]) {
    // Store pin configuration
    for (int i = 0; i < KEYPAD_ROWS; i++) {
        rowPins[i] = row_pins[i];
        pinMode(rowPins[i], INPUT_PULLUP);
    }
    for (int i = 0; i < KEYPAD_COLS; i++) {
        colPins[i] = col_pins[i];
        pinMode(colPins[i], INPUT_PULLUP);
    }
    keyMap = (const char*)keymap;

    return 0;
}

int KEYPADHID_::update(void) {
    unsigned long currentTime = millis();
    uint16_t newState = 0;

    // Scan each key
    for (int row = 0; row < KEYPAD_ROWS; row++) {
        // Set current row to LOW
        pinMode(rowPins[row], OUTPUT);
        digitalWrite(rowPins[row], LOW);
        delayMicroseconds(10); // Short delay for signal to stabilize

        for (int col = 0; col < KEYPAD_COLS; col++) {
            // Read column pin
            if (digitalRead(colPins[col]) == LOW) {
                // Key is pressed
                int buttonIndex = row * KEYPAD_COLS + col;
                newState |= (1 << buttonIndex);
            }
        }

        // Set row back to INPUT_PULLUP
        digitalWrite(rowPins[row], HIGH);
        pinMode(rowPins[row], INPUT_PULLUP);
    }

    // Simple debounce: only update if state is stable
    static uint16_t stableState = 0;
    static unsigned long lastChangeTime = 0;

    if (newState != stableState) {
        if (currentTime - lastChangeTime > KEYPAD_DEBOUNCE_MS) {
            stableState = newState;
            lastChangeTime = currentTime;

            // Send update if state changed
            if (stableState != lastState) {
                send_state(stableState);
                lastState = stableState;
            }
        }
    } else {
        lastChangeTime = currentTime;
    }

    return 0;
}

int KEYPADHID_::send_state(uint16_t button_state) {
    uint8_t data[3];

    data[0] = KEYPAD_REPORT_ID;
    data[1] = button_state & 0xFF;
    data[2] = (button_state >> 8) & 0xFF;

    // Debug output for USB_Send
    int result = USB_Send(pluggedEndpoint, data, 3);
    if (result != 3) {
        Serial.print("USB_Send error: ");
        Serial.println(result);
    }
    return result;
}

int KEYPADHID_::getInterface(byte* interface_count) {
    *interface_count += 1;
    HIDDescriptor hid_interface = {
        D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
        D_HIDREPORT(sizeof(hid_report)),
        D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x04)
    };
    return USB_SendControl(0, &hid_interface, sizeof(hid_interface));
}

int KEYPADHID_::getDescriptor(USBSetup& setup) {
    if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) {
        return 0;
    }

    if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) {
        return 0;
    }

    if (setup.wIndex != pluggedInterface) {
        return 0;
    }

    return USB_SendControl(TRANSFER_PGM, hid_report, sizeof(hid_report));
}

bool KEYPADHID_::setup(USBSetup& setup) {
    if (pluggedInterface != setup.wIndex) {
        return false;
    }

    // No special setup required for keypad
    return false;
}

KEYPADHID_ KEYPADHID;