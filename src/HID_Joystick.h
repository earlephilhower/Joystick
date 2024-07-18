/*
    Joystick.h

    Copyright (c) 2022, Benjamin Aigner <beni@asterics-foundation.org>

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

#pragma once

#include <Arduino.h>
#include "class/hid/hid.h"

/// HID Gamepad Protocol Report.
typedef struct TU_ATTR_PACKED {
  int16_t x;         ///< Delta x  movement of left analog-stick
  int16_t y;         ///< Delta y  movement of left analog-stick
  int16_t z;         ///< Delta z  movement of right analog-joystick
  int16_t rz;        ///< Delta Rz movement of right analog-joystick
  int16_t rx;        ///< Delta Rx movement of analog left trigger
  int16_t ry;        ///< Delta Ry movement of analog right trigger
  uint8_t hat;       ///< Buttons mask for currently pressed buttons in the DPad/hat
  uint32_t buttons;  ///< Buttons mask for currently pressed buttons
} hid_gamepad16_report_t;


//======================================================================
class HID_Joystick {
protected:
    bool _autosend;
    bool _use8bit;
    bool _use10bit;
    bool _use16bit;
    hid_gamepad16_report_t data;
    int mapBits(int const value);
public:
    HID_Joystick(void);
    void begin(void);
    void end(void);

    //set a selected button to pressed/released
    void button(uint8_t button, bool val);
    //same call as button, but "button" starts with 0
    void setButton(uint8_t btn, bool val);
    //set axis values
    void X(int val);
    void Y(int val);
    void position(int X, int Y);
    void Z(int val);
    void Zrotate(int val);
    void sliderLeft(int val);
    void sliderRight(int val);
    //note: not implemented in TinyUSB gamepad, is mapped to sliderLeft.
    void slider(int val);

    //additional hat function to use the hat position instead of the angle
    enum HatPosition { IDLE = 0, UP = 1, UP_RIGHT = 2, RIGHT = 3, DOWN_RIGHT = 4, DOWN = 5, DOWN_LEFT = 6, LEFT = 7, UP_LEFT = 8 };
    void hat(HatPosition position);

    //set the hat value, from 0-360. -1 is rest position
    void hat(int angle);
    //compatibility: there is only one hat implemented, num parameter is ignored
    void hat(unsigned int num, int angle);


    //if set, the gamepad report is not automatically sent after an update of axes/buttons; use send_now to update
    void useManualSend(bool mode);
    //immediately send an HID report
    virtual void send_now(void) = 0;
    //define the mapping of axes values
    //default: axes methods are accepting values from 0-1023 (compatibility to other Joystick libraries)
    //if use8bit(true) is called, -127 to 127 values are used.
    void use8bit(bool mode = true);

    // 10-bit, unsigned 0...1023
    void use10bit();

    // 16-bit signed, -32767...32767
    void use16bit();

    //get the gamepad report which is sent last.
    void getReport(hid_gamepad16_report_t *report);
};
