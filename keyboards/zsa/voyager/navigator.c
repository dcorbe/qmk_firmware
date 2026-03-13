// Copyright 2025 ZSA Technology Labs, Inc <contact@zsa.io>
// SPDX-License-Identifier: GPL-2.0-or-later

// Navigator scroll/turbo/aim overlay.
// Processes mouse reports from the sensor driver, applying rotation,
// speed modes, and smooth scroll conversion.

#include "quantum.h"
#include "navigator.h"

#if _NAVIGATOR_ROTATION != 0 && _NAVIGATOR_ROTATION != 90 && \
    _NAVIGATOR_ROTATION != 180 && _NAVIGATOR_ROTATION != 270
#    define _NT_ROT_RAD (_NAVIGATOR_ROTATION * 3.14159265358979f / 180.0f)
static const float rotation_cos = __builtin_cosf(_NT_ROT_RAD);
static const float rotation_sin = __builtin_sinf(_NT_ROT_RAD);
#endif

float scroll_accumulated_h = 0;
float scroll_accumulated_v = 0;

bool set_scrolling    = false;
bool navigator_turbo  = false;
bool navigator_aim    = false;

report_mouse_t pointing_device_task_navigator(report_mouse_t mouse_report) {
    // Apply rotation transform
#if _NAVIGATOR_ROTATION == 90
    mouse_xy_report_t tmp_x = mouse_report.x;
    mouse_report.x = -mouse_report.y;
    mouse_report.y = tmp_x;
#elif _NAVIGATOR_ROTATION == 180
    mouse_report.x = -mouse_report.x;
    mouse_report.y = -mouse_report.y;
#elif _NAVIGATOR_ROTATION == 270
    mouse_xy_report_t tmp_x = mouse_report.x;
    mouse_report.x = mouse_report.y;
    mouse_report.y = -tmp_x;
#elif _NAVIGATOR_ROTATION != 0
    mouse_xy_report_t tmp_x = mouse_report.x;
    mouse_report.x = (mouse_xy_report_t)(tmp_x * rotation_cos - mouse_report.y * rotation_sin);
    mouse_report.y = (mouse_xy_report_t)(tmp_x * rotation_sin + mouse_report.y * rotation_cos);
#endif

    if (navigator_turbo) {
        mouse_report.x *= NAVIGATOR_TURBO_MULTIPLIER;
        mouse_report.y *= NAVIGATOR_TURBO_MULTIPLIER;
    }

    if (navigator_aim) {
        mouse_report.x /= NAVIGATOR_AIM_DIVIDER;
        mouse_report.y /= NAVIGATOR_AIM_DIVIDER;
    }

    if (set_scrolling) {
        scroll_accumulated_h += (float)mouse_report.x / NAVIGATOR_SCROLL_DIVIDER;
        scroll_accumulated_v += (float)mouse_report.y / NAVIGATOR_SCROLL_DIVIDER;

        float abs_h = (scroll_accumulated_h < 0) ? -scroll_accumulated_h : scroll_accumulated_h;
        float abs_v = (scroll_accumulated_v < 0) ? -scroll_accumulated_v : scroll_accumulated_v;

        float scroll_h = 0.0f;
        float scroll_v = 0.0f;

        if (abs_h >= 1.0f) {
            float speed_h = 1.0f + ((abs_h - 1.0f) * NAVIGATOR_SCROLL_ACCELERATION);
            if (speed_h > NAVIGATOR_SCROLL_MAX_SPEED) {
                speed_h = NAVIGATOR_SCROLL_MAX_SPEED;
            }
            scroll_h = (scroll_accumulated_h > 0) ? speed_h : -speed_h;
        }

        if (abs_v >= 1.0f) {
            float speed_v = 1.0f + ((abs_v - 1.0f) * NAVIGATOR_SCROLL_ACCELERATION);
            if (speed_v > NAVIGATOR_SCROLL_MAX_SPEED) {
                speed_v = NAVIGATOR_SCROLL_MAX_SPEED;
            }
            scroll_v = (scroll_accumulated_v > 0) ? speed_v : -speed_v;
        }

#ifdef NAVIGATOR_SCROLL_INVERT_X
        mouse_report.h = (int8_t)scroll_h;
#else
        mouse_report.h = (int8_t)-scroll_h;
#endif

#ifdef NAVIGATOR_SCROLL_INVERT_Y
        mouse_report.v = (int8_t)-scroll_v;
#else
        mouse_report.v = (int8_t)scroll_v;
#endif

        if (abs_h >= 1.0f) {
            scroll_accumulated_h -= (scroll_accumulated_h > 0) ? 1.0f : -1.0f;
        }
        if (abs_v >= 1.0f) {
            scroll_accumulated_v -= (scroll_accumulated_v > 0) ? 1.0f : -1.0f;
        }

        // Gentle decay after idle
        static uint8_t idle_counter_h = 0, idle_counter_v = 0;

        if (mouse_report.x == 0 && mouse_report.h == 0) {
            idle_counter_h++;
            if (idle_counter_h > 20) {
                scroll_accumulated_h *= 0.98f;
            }
        } else {
            idle_counter_h = 0;
        }

        if (mouse_report.y == 0 && mouse_report.v == 0) {
            idle_counter_v++;
            if (idle_counter_v > 20) {
                scroll_accumulated_v *= 0.98f;
            }
        } else {
            idle_counter_v = 0;
        }

        mouse_report.x = 0;
        mouse_report.y = 0;
    }
    return mouse_report;
}
