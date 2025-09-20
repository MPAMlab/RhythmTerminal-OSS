#include "NAVHID.h"

static const uint8_t PROGMEM hid_report[] = {
    0x05, 0x01,                      // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                      // USAGE (Joystick)
    0xa1, 0x01,                      // COLLECTION (Application)

    0x85, NAV_REPORT_ID,             //   REPORT_ID (NAV_REPORT_ID)

    /* Navigation buttons */
    0x05, 0x09,                      //   USAGE_PAGE (Button)
    0x19, 0x01,                      //   USAGE_MINIMUM (Button 1)
    0x29, NAV_BUTTONS,               //   USAGE_MAXIMUM (Button 8)
    0x15, 0x00,                      //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                      //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                      //   REPORT_SIZE (1)
    0x95, NAV_BUTTONS,               //   REPORT_COUNT (8)
    0x81, 0x02,                      //   INPUT (Data,Var,Abs)

    /* Button padding */
    0x75, 0x01,                      //   REPORT_SIZE (1)
    0x95, NAV_BUTTON_PADDING,        //   REPORT_COUNT (0)
    0x81, 0x03,                      //   INPUT (Cnst,Var,Abs)

    0xc0                             // END_COLLECTION
};

NAVHID_::NAVHID_(void) : PluggableUSBModule(1, 1, epType) {
    epType[0] = EP_TYPE_INTERRUPT_IN;
    PluggableUSB().plug(this);

    // Initialize state
    lastState = 0;
    for (int i = 0; i < NAV_BUTTONS; i++) {
        currentState[i] = false;
        lastUpdateTime[i] = 0;
    }
}

int NAVHID_::begin(uint8_t button_pins[NAV_BUTTONS]) {
    // Store button pin configuration
    for (int i = 0; i < NAV_BUTTONS; i++) {
        buttonPins[i] = button_pins[i];
        pinMode(buttonPins[i], INPUT_PULLUP);
    }

    return 0;
}

int NAVHID_::update(void) {
    unsigned long currentTime = millis();
    uint8_t newState = 0;

    // Read each button
    for (int i = 0; i < NAV_BUTTONS; i++) {
        bool pressed = (digitalRead(buttonPins[i]) == LOW);

        if (pressed) {
            newState |= (1 << i);
        }
    }

    // Simple debounce: only update if state is stable
    static uint8_t stableState = 0;
    static unsigned long lastChangeTime = 0;

    if (newState != stableState) {
        if (currentTime - lastChangeTime > NAV_DEBOUNCE_MS) {
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

int NAVHID_::send_state(uint8_t button_state) {
    uint8_t data[2];

    data[0] = NAV_REPORT_ID;
    data[1] = button_state;

    return USB_Send(pluggedEndpoint | TRANSFER_RELEASE, data, 2);
}

int NAVHID_::getInterface(byte* interface_count) {
    *interface_count += 1;
    HIDDescriptor hid_interface = {
        D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
        D_HIDREPORT(sizeof(hid_report)),
        D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x04)
    };
    return USB_SendControl(0, &hid_interface, sizeof(hid_interface));
}

int NAVHID_::getDescriptor(USBSetup& setup) {
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

bool NAVHID_::setup(USBSetup& setup) {
    if (pluggedInterface != setup.wIndex) {
        return false;
    }

    // No special setup required for navigation buttons
    return false;
}

NAVHID_ NAVHID;