// Copyright 2026 Daniel
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#define LEADER_TIMEOUT 400
#define LEADER_PER_KEY_TIMING

// From Oryx config
#define ONESHOT_TAP_TOGGLE 2
#undef ONESHOT_TIMEOUT
#define ONESHOT_TIMEOUT 4000
#define USB_SUSPEND_WAKEUP_DELAY 0
#define LAYER_STATE_8BIT

// Take control of status LEDs from voyager.c
#define VOYAGER_USER_LEDS
