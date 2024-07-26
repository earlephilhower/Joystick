/*
    Joystick.cpp

    Copyright (c) 2022, Benjamin Aigner <beni@asterics-foundation.org>
    Implementation loosely based on:
    Mouse library from https://github.com/earlephilhower/arduino-pico
    Joystick functions from Teensyduino https://github.com/PaulStoffregen

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "HID_Joystick.h"
#include <Arduino.h>
#include <RP2040USB.h>

#include <tusb.h>
#include <class/hid/hid_device.h>

// Weak function override to add our descriptor to the TinyUSB list
//void __USBInstallJoystick() { /* noop */ }

//================================================================================
//================================================================================
//	Joystick/Gamepad

HID_Joystick::HID_Joystick(void) {
    _use8bit = false;
    _use10bit = true;
    _use16bit = false;
    _autosend = true;
    memset(&data, 0, sizeof(data));
    //_X_axis = _Y_axis = _Z_axis = _Zrotate = _sliderLeft = _sliderRight = _hat = data.buttons = 0;
}

/** define the mapping of axes values
    default: axes methods are accepting values from 0-1023 (compatibility to other Joystick libraries)
		and are mapped internally to int8_t
    if use8bit(true) is called, -127 to 127 values are used.*/
void HID_Joystick::use8bit(bool mode) {
    _use8bit = mode;
}

void HID_Joystick::use10bit() {
    _use8bit = false;
    _use10bit = true;
    _use16bit = false;
}

void HID_Joystick::use16bit() {
    _use8bit = false;
    _use10bit = false;
    _use16bit = true;
}

//if set, the gamepad report is not automatically sent after an update of axes/buttons; use send_now to update
void HID_Joystick::useManualSend(bool mode) {
    _autosend = !mode;
}

/** Maps values from 8bit signed or 10bit unsigned to report values

    Depending on the setting via use8bit(), either values from 0-1023 or -127 - 127
    are mapped.
*/
int HID_Joystick::mapBits(int const value) {
    if (_use8bit) {
        if (value < -127) {
            return -32767;
        } else if (value >  127) {
            return 32767;
        } else {
            return value << 8;
        }
    } else if (_use10bit) {
        if (value < 0) {
            return 0;
        }
        if (value > 1023) {
            return 32767;
        }
        return map(value, 0, 1023, -32767, 32767);
    } else {
        if (value < -32767) {
            return -32767;
        } else if (value >  32767) {
            return 32767;
        } else {
            return value;
        }
    }
}

void HID_Joystick::begin(void) {
}

void HID_Joystick::end(void) {
}

void HID_Joystick::button(uint8_t button, bool val) {
    //I've no idea why, but without a second dword, it is not possible.
    //maybe something with the alignment when using bit set/clear?!?
    static uint32_t buttons_local = 0;

    if (button >= 1 && button <= 32) {
        if (val) {
            buttons_local |= (1UL << (button - 1));
        } else {
            buttons_local &= ~(1UL << (button - 1));
        }

        data.buttons = buttons_local;
        if (_autosend) {
            send_now();
        }
    }
}

void HID_Joystick::setButton(uint8_t btn, bool val) {
    //simply call button, but we setButton uses 0-31; button 1-32
    button(btn + 1, val);
}

void HID_Joystick::X(int val) {
    data.x = mapBits(val);
    if (_autosend) {
        send_now();
    }
}
void HID_Joystick::Y(int val) {
    data.y = mapBits(val);
    if (_autosend) {
        send_now();
    }
}
void HID_Joystick::Z(int val) {
    data.z = mapBits(val);
    if (_autosend) {
        send_now();
    }
}
void HID_Joystick::Zrotate(int val) {
    data.rz = mapBits(val);
    if (_autosend) {
        send_now();
    }
}
void HID_Joystick::sliderLeft(int val) {
    data.rx = mapBits(val);
    if (_autosend) {
        send_now();
    }
}
void HID_Joystick::sliderRight(int val) {
    data.ry = mapBits(val);
    if (_autosend) {
        send_now();
    }
}

void HID_Joystick::slider(int val) {
    data.rx = mapBits(val);
    if (_autosend) {
        send_now();
    }
}

void HID_Joystick::position(int X, int Y) {
    data.x = mapBits(X);
    data.y = mapBits(Y);
    if (_autosend) {
        send_now();
    }
}

//additional hat function to use the hat position instead of the angle
void HID_Joystick::hat(HatPosition position) {
    data.hat = (uint8_t) position;
    if (_autosend) {
        send_now();
    }
}

//compatibility: there is only one hat implemented, num parameter is ignored
void HID_Joystick::hat(unsigned int num, int angle) {
    (void) num;
    hat(angle);
}

//set the hat value, from 0-360. -1 is rest position
void HID_Joystick::hat(int angle) {
    if (angle < 0) {
        data.hat = 0;
    }
    if (angle >= 0 && angle <= 360) {
        data.hat = map(angle, 0, 360, 1, 9);
    }
    if (data.hat == 9) {
        data.hat = 0;
    }
    if (_autosend) {
        send_now();
    }
}

//send back the Joystick report
void HID_Joystick::getReport(hid_gamepad16_report_t *report) {
    memcpy(report, &data, sizeof(data));
}
