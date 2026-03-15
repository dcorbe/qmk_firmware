// Copyright 2026 Daniel
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "navigator.h"

// Tap: KC_EQUAL, Hold: KC_ESCAPE (uses LT for tap/hold detection)
#define DUAL_FUNC_0 LT(5, KC_5)

enum layers {
    _BASE,
    _NAV,
    _UTIL,
};


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
        DUAL_FUNC_0,    KC_1,           KC_2,           KC_3,           KC_4,           KC_5,                                           KC_6,           KC_7,           KC_8,           KC_9,           KC_0,           KC_MINUS,
        MO(_NAV),       KC_Q,           KC_W,           KC_E,           KC_R,           KC_T,                                           KC_Y,           KC_U,           KC_I,           KC_O,           KC_P,           KC_BSLS,
        KC_LEFT_GUI,    KC_A,           KC_S,           KC_D,           KC_F,           KC_G,                                           KC_H,           KC_J,           KC_K,           KC_L,           KC_SCLN,        KC_QUOTE,
        KC_LEFT_CTRL,   KC_Z,           KC_X,           KC_C,           KC_V,           KC_B,                                           KC_N,           KC_M,           KC_COMMA,       KC_DOT,         KC_SLASH,       QK_LEAD,
                                                        MT(MOD_LALT, KC_BSPC), MT(MOD_LSFT, KC_TAB),                                   MT(MOD_RSFT, KC_ENTER), MT(MOD_RALT, KC_SPACE)
    ),

    [_NAV] = LAYOUT(
        _______,        KC_F1,          KC_F2,          KC_F3,          KC_F4,          KC_F5,                                          KC_F6,          KC_F7,          KC_F8,          KC_F9,          KC_F10,         KC_F11,
        _______,        _______,        _______,        _______,        _______,        _______,                                        _______,        _______,        _______,        _______,        _______,        _______,
        _______,        _______,        _______,        _______,        _______,        _______,                                        KC_LEFT,        KC_DOWN,        KC_UP,          KC_RIGHT,       _______,        _______,
        _______,        _______,        _______,        _______,        _______,        _______,                                        _______,        _______,        _______,        _______,        _______,        _______,
                                                        _______,        _______,                                                        _______,        _______
    ),

    // Navigator automouse layer — activates automatically when trackball moves
    [_UTIL] = LAYOUT(
        NAVIGATOR_DEC_CPI, NAVIGATOR_INC_CPI, _______,     _______,        _______,        TG(_UTIL),                                        _______,        _______,        _______,        _______,        _______,        _______,
        _______,        _______,        _______,        _______,        MS_BTN3,     TOGGLE_SCROLL,                                  _______,        _______,        _______,        _______,        _______,        _______,
        _______,        _______,        _______,        MS_BTN2,        MS_BTN1,     DRAG_SCROLL,                                    _______,        _______,        _______,        _______,        _______,        _______,
        _______,        _______,        _______,        _______,        _______,        _______,                                        _______,        _______,        _______,        _______,        _______,        _______,
                                                        _______,        _______,                                                        _______,        _______
    ),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    case QK_MODS ... QK_MODS_MAX:
        // Mouse and consumer keys with modifiers work inconsistently across OSes.
        // Ensure modifiers are always applied to the key that was pressed.
        if (IS_MOUSE_KEYCODE(QK_MODS_GET_BASIC_KEYCODE(keycode))) {
            if (record->event.pressed) {
                add_mods(QK_MODS_GET_MODS(keycode));
                send_keyboard_report();
                wait_ms(2);
                register_code(QK_MODS_GET_BASIC_KEYCODE(keycode));
                return false;
            } else {
                wait_ms(2);
                del_mods(QK_MODS_GET_MODS(keycode));
            }
        }
        break;

    case DUAL_FUNC_0:
        if (record->tap.count > 0) {
            if (record->event.pressed) {
                register_code16(KC_EQUAL);
            } else {
                unregister_code16(KC_EQUAL);
            }
        } else {
            if (record->event.pressed) {
                register_code16(KC_ESCAPE);
            } else {
                unregister_code16(KC_ESCAPE);
            }
        }
        return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// RGB Matrix — per-layer lighting
// ---------------------------------------------------------------------------
//
// LED index map (52 keys):
//   Left hand:  0-5 (row 0), 6-11 (row 1), 12-17 (row 2), 18-23 (row 3), 24-25 (thumbs)
//   Right hand: 26-31 (row 0), 32-37 (row 1), 38-43 (row 2), 44-49 (row 3), 50-51 (thumbs)

// Colors (R, G, B)
#define CLR_TEAL        0, 80, 80
#define CLR_TEAL_BRIGHT 0, 160, 160
#define CLR_ORANGE      180, 80, 0
#define CLR_ORANGE_DIM  60, 25, 0
#define CLR_WHITE       200, 200, 200
#define CLR_PURPLE      100, 0, 160
#define CLR_PURPLE_DIM  30, 0, 50
#define CLR_GREEN       0, 200, 0
#define CLR_CYAN        0, 180, 180
#define CLR_YELLOW      200, 180, 0
#define CLR_BLUE_PULSE  0, 0, 120
#define CLR_OFF         0, 0, 0
#define REACTIVE_FADE_MS 300

static void set_all(uint8_t led_min, uint8_t led_max, uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t i = led_min; i < led_max; i++) {
        rgb_matrix_set_color(i, r, g, b);
    }
}

// Set color for a range of LEDs, respecting the batch window
static void set_range(uint8_t first, uint8_t last, uint8_t led_min, uint8_t led_max, uint8_t r, uint8_t g, uint8_t b) {
    uint8_t start = (first > led_min) ? first : led_min;
    uint8_t end   = (last < led_max) ? last : led_max;
    for (uint8_t i = start; i < end; i++) {
        rgb_matrix_set_color(i, r, g, b);
    }
}

// Set color for a single LED if it falls within the batch window
static void set_led(uint8_t idx, uint8_t led_min, uint8_t led_max, uint8_t r, uint8_t g, uint8_t b) {
    if (idx >= led_min && idx < led_max) {
        rgb_matrix_set_color(idx, r, g, b);
    }
}

// Set a single LED to a rainbow color based on timer, offset per key for spread
static void set_led_rainbow(uint8_t idx, uint8_t led_min, uint8_t led_max, uint8_t offset) {
    if (idx < led_min || idx >= led_max) return;
    uint8_t hue = (uint8_t)((timer_read() / 4) + (offset * 60));
    HSV hsv = {hue, 255, 200};
    RGB rgb = hsv_to_rgb(hsv);
    rgb_matrix_set_color(idx, rgb.r, rgb.g, rgb.b);
}

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    uint8_t layer = get_highest_layer(layer_state);
    bool gui_held = get_mods() & MOD_MASK_GUI;

    switch (layer) {
    case _BASE:
        set_all(led_min, led_max, CLR_TEAL);
        // WASD: W (8), A (13), S (14), D (15) — green
        set_led(8, led_min, led_max, CLR_GREEN);
        set_range(13, 16, led_min, led_max, CLR_GREEN);
        // Home row right: H J K L (38-41)
        if (gui_held) {
            // Rainbow pulse when Cmd is held
            set_led_rainbow(38, led_min, led_max, 0);
            set_led_rainbow(39, led_min, led_max, 1);
            set_led_rainbow(40, led_min, led_max, 2);
            set_led_rainbow(41, led_min, led_max, 3);
        } else {
            set_range(38, 42, led_min, led_max, CLR_TEAL_BRIGHT);
        }
        break;

    case _NAV:
        set_all(led_min, led_max, CLR_ORANGE_DIM);
        // F-keys: left F1-F5 (1-5), right F6-F11 (26-31)
        set_range(1, 6, led_min, led_max, CLR_YELLOW);
        set_range(26, 32, led_min, led_max, CLR_YELLOW);
        // Arrow keys: H J K L positions (38-41) — bright white
        set_range(38, 42, led_min, led_max, CLR_WHITE);
        break;

    case _UTIL:
        if (set_scrolling) {
            // Scroll mode active — blue pulse
            set_all(led_min, led_max, CLR_BLUE_PULSE);
        } else {
            set_all(led_min, led_max, CLR_PURPLE_DIM);
        }
        // CPI keys (0, 1) — yellow
        set_led(0, led_min, led_max, CLR_YELLOW);
        set_led(1, led_min, led_max, CLR_YELLOW);
        // TG lock key (5) — white
        set_led(5, led_min, led_max, CLR_WHITE);
        // Mouse buttons: BTN3 (10), BTN2 (15), BTN1 (16) — green
        set_led(10, led_min, led_max, CLR_GREEN);
        set_led(15, led_min, led_max, CLR_GREEN);
        set_led(16, led_min, led_max, CLR_GREEN);
        // Scroll keys: TOGGLE_SCROLL (11), DRAG_SCROLL (17) — cyan
        set_led(11, led_min, led_max, CLR_CYAN);
        set_led(17, led_min, led_max, CLR_CYAN);
        break;
    }

    // Reactive flash: white flash on recently pressed keys, fading out
    for (uint8_t j = 0; j < g_last_hit_tracker.count; j++) {
        uint8_t idx = g_last_hit_tracker.index[j];
        if (idx < led_min || idx >= led_max) continue;

        uint16_t tick = g_last_hit_tracker.tick[j];
        // tick increments at ~1 per ms; use directly as elapsed time
        if (tick < REACTIVE_FADE_MS) {
            uint8_t b = 255 - (uint8_t)((uint32_t)255 * tick / REACTIVE_FADE_MS);
            rgb_matrix_set_color(idx, b, b, b);
        }
    }

    return false;
}

// ---------------------------------------------------------------------------
// Status LEDs — mode indicators
// ---------------------------------------------------------------------------

layer_state_t layer_state_set_user(layer_state_t state) {
    // Clear scroll mode whenever the mouse layer deactivates, so the
    // trackball x/y reports are non-zero and auto-mouse can re-engage.
    if (!layer_state_cmp(state, _UTIL)) {
        set_scrolling = false;
    }
    // LED 4: layer 2 locked (TG, not automouse)
    STATUS_LED_4(get_auto_mouse_toggle());
    return state;
}

void housekeeping_task_user(void) {
    // These change outside of layer_state_set, so poll them
    STATUS_LED_1(set_scrolling);
    STATUS_LED_2(is_caps_word_on());
    STATUS_LED_3(leader_sequence_active());
}

void pointing_device_init_user(void) {
    set_auto_mouse_enable(true);
}

void leader_end_user(void) {
    if (leader_sequence_two_keys(KC_G, KC_P)) {
        SEND_STRING("git push\n");
    } else if (leader_sequence_two_keys(KC_G, KC_S)) {
        SEND_STRING("git status\n");
    } else if (leader_sequence_two_keys(KC_G, KC_L)) {
        SEND_STRING("git log --oneline\n");
    }
}
