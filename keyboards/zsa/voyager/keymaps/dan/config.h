// Copyright 2026 Daniel
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define LEADER_TIMEOUT 400
#define LEADER_PER_KEY_TIMING

// Caps Word — activate by holding Left + Right Shift together.
// (Not double-tap-shift, which is reserved for games.)
#define BOTH_SHIFTS_TURNS_ON_CAPS_WORD

// From Oryx config
#define ONESHOT_TAP_TOGGLE 2
#undef ONESHOT_TIMEOUT
#define ONESHOT_TIMEOUT 4000
#define USB_SUSPEND_WAKEUP_DELAY 0
#define LAYER_STATE_8BIT

// Take control of status LEDs from voyager.c
#define VOYAGER_USER_LEDS

// Mousekeys
#undef MOUSEKEY_DELAY
#define MOUSEKEY_DELAY       50
#undef MOUSEKEY_INTERVAL
#define MOUSEKEY_INTERVAL    20
#undef MOUSEKEY_MAX_SPEED
#define MOUSEKEY_MAX_SPEED   6
#undef MOUSEKEY_TIME_TO_MAX
#define MOUSEKEY_TIME_TO_MAX 25

// Navigator trackball
#define MOUSE_EXTENDED_REPORT
#define POINTING_DEVICE_AUTO_MOUSE_ENABLE
#define AUTO_MOUSE_DEFAULT_LAYER 2
#define AUTO_MOUSE_TIME 650
#define AUTO_MOUSE_THRESHOLD 10
#define NAVIGATOR_SCROLL_DIVIDER 50
#define NAVIGATOR_SCROLL_INVERT_Y
