// Copyright 2026 Daniel
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H
#include "os_detection.h"

// Tap: KC_EQUAL, Hold: KC_ESCAPE (uses LT for tap/hold detection)
#define DUAL_FUNC_0 LT(5, KC_5)

enum layers {
    _BASE,
    _NAV,
    _EVE,
};

// Workspace switching — macOS: Ctrl+Arrow, Linux: GUI+HJKL
enum custom_keycodes {
    WS_LEFT = SAFE_RANGE,
    WS_DOWN,
    WS_UP,
    WS_RIGHT,
};

enum td_keycodes {
    TD_OH_HIGH,  // tap: Ctrl+3 (overheat high rack), hold: MO(_NAV)
    TD_OH_MID,   // tap: Ctrl+2 (overheat mid rack),  hold: Shift
    TD_OH_LOW,   // tap: Ctrl+1 (overheat low rack),  hold: CTRL
};

// Overheat high rack: tap = Ctrl+3, hold = MO(_NAV)
static void td_oh_high_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        if (state->pressed) {
            layer_on(_NAV);
        } else {
            tap_code16(LCTL(KC_3));
        }
    }
}
static void td_oh_high_reset(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        layer_off(_NAV);
    }
}

// Overheat mid rack: tap = Ctrl+2, hold = Shift
static void td_oh_mid_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        if (state->pressed) {
            register_code(KC_LEFT_SHIFT);
        } else {
            tap_code16(LCTL(KC_2));
        }
    }
}
static void td_oh_mid_reset(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        unregister_code(KC_LEFT_SHIFT);
        clear_mods();
        send_keyboard_report();
    }
}

// Overheat low rack: tap = Ctrl+1, hold = CTRL
static void td_oh_low_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        if (state->pressed) {
            register_code(KC_LEFT_CTRL);
        } else {
            tap_code16(LCTL(KC_1));
        }
    }
}
static void td_oh_low_reset(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        unregister_code(KC_LEFT_CTRL);
        clear_mods();
        send_keyboard_report();
    }
}

tap_dance_action_t tap_dance_actions[] = {
    [TD_OH_HIGH]      = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_oh_high_finished, td_oh_high_reset),
    [TD_OH_MID]       = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_oh_mid_finished,  td_oh_mid_reset),
    [TD_OH_LOW]       = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_oh_low_finished,  td_oh_low_reset),
};


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
        DUAL_FUNC_0,    KC_1,           KC_2,           KC_3,           KC_4,           KC_5,                                           KC_6,           KC_7,           KC_8,           KC_9,           KC_0,           KC_MINUS,
        MO(_NAV),       KC_Q,           KC_W,           KC_E,           KC_R,           KC_T,                                           KC_Y,           KC_U,           KC_I,           KC_O,           KC_P,           KC_BSLS,
        KC_LEFT_GUI,    KC_A,           KC_S,           KC_D,           KC_F,           KC_G,                                           KC_H,           KC_J,           KC_K,           KC_L,           KC_SCLN,        KC_QUOTE,
        KC_LEFT_CTRL,   KC_Z,           KC_X,           KC_C,           KC_V,           KC_B,                                           KC_N,           KC_M,           KC_COMMA,       KC_DOT,         KC_SLASH,       TG(_EVE),
                                                        MT(MOD_LALT, KC_BSPC), KC_LSFT,                                        MT(MOD_RSFT, KC_ENTER), MT(MOD_RALT, KC_SPACE)
    ),

    [_NAV] = LAYOUT(
        _______,        KC_F1,          KC_F2,          KC_F3,          KC_F4,          KC_F5,                                          KC_F6,          KC_F7,          KC_F8,          KC_F9,          KC_F10,         KC_F11,
        _______,        _______,        _______,        _______,        _______,        _______,                                        _______,        _______,        _______,        _______,        _______,        _______,
        _______,        _______,        _______,        _______,        _______,        _______,                                        KC_LEFT,        KC_DOWN,        KC_UP,          KC_RIGHT,       _______,        _______,
        _______,        _______,        _______,        _______,        _______,        _______,                                        _______,        _______,        _______,        _______,        _______,        _______,
                                                        _______,        KC_TAB,                                                         _______,        _______
    ),

    // EVE Online module layer
    // QWERT = high slots 1-5 (F1-F5), ASDFG = mid slots 1-5 (Alt+F1-F5), ZXCVB = low slots 1-5 (Cmd+F1-F5)
    // Number row: dock/jump (D), lock target (F16), approach (Q), keep at range (E), align to (A), warp to (S)
    [_EVE] = LAYOUT(
        KC_D,           KC_F16,         KC_Q,           KC_E,           KC_A,           KC_S,                                           _______,        _______,        _______,        _______,        _______,        _______,
        TD(TD_OH_HIGH), KC_F1,          KC_F2,          KC_F3,          KC_F4,          KC_F5,                                          KC_F13,         LCTL(KC_F),     LCTL(KC_I),     LCTL(KC_L),     KC_LBRC,        KC_RBRC,
        TD(TD_OH_MID),  LALT(KC_F1),    LALT(KC_F2),    LALT(KC_F3),    LALT(KC_F4),    LALT(KC_F5),                                    WS_LEFT,        WS_DOWN,        WS_UP,          WS_RIGHT,       _______,        _______,
        TD(TD_OH_LOW),  LGUI(KC_F1),    LGUI(KC_F2),    LGUI(KC_F3),    LGUI(KC_F4),    LGUI(KC_F5),                                    LCTL(LALT(KC_SPC)), KC_M,      LSFT(KC_M),     _______,        _______,        TG(_EVE),
                                                        _______,        _______,                                                        _______,        _______
    ),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    case WS_LEFT:
    case WS_DOWN:
    case WS_UP:
    case WS_RIGHT: {
        if (!record->event.pressed) return false;
        bool is_macos = detected_host_os() != OS_LINUX;
        uint16_t key = (keycode == WS_LEFT)  ? (is_macos ? LCTL(KC_LEFT)  : LGUI(KC_H)) :
                       (keycode == WS_DOWN)  ? (is_macos ? LCTL(KC_DOWN)  : LGUI(KC_J)) :
                       (keycode == WS_UP)    ? (is_macos ? LCTL(KC_UP)    : LGUI(KC_K)) :
                                               (is_macos ? LCTL(KC_RIGHT) : LGUI(KC_L));
        tap_code16(key);
        clear_mods();
        send_keyboard_report();
        return false;
    }
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
#define CLR_GREEN       0, 200, 0
#define CLR_CYAN        0, 180, 180
#define CLR_YELLOW      200, 180, 0
#define CLR_PINK        255, 50, 130
#define CLR_RED         200, 0, 0
#define CLR_BLUE        0, 0, 200
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

    case _EVE:
        set_all(led_min, led_max, CLR_OFF);
        // Navigation row (0-5): dock, lock target, approach, keep at range, align to, warp to — cyan
        set_range(0, 6, led_min, led_max, CLR_CYAN);
        // Lock target (index 1) — red to distinguish from nav keys
        set_led(1, led_min, led_max, CLR_RED);
        // Set full speed (N position, index 44) — cyan
        set_led(44, led_min, led_max, CLR_CYAN);
        // Overheat rack keys (6, 12, 18) — match rack colors
        set_led(6,  led_min, led_max, CLR_RED);
        set_led(12, led_min, led_max, CLR_BLUE);
        set_led(18, led_min, led_max, CLR_ORANGE);
        // High slots: QWERT (left row 1, indices 7-11) — red
        set_range(7, 12, led_min, led_max, CLR_RED);
        // Mid slots: ASDFG (left row 2, indices 13-17) — blue
        set_range(13, 18, led_min, led_max, CLR_BLUE);
        // Low slots: ZXCVB (left row 3, indices 19-23) — orange
        set_range(19, 24, led_min, led_max, CLR_ORANGE);
        // PTT key: Y position (32) — pink
        set_led(32, led_min, led_max, CLR_PINK);
        // Window shortcuts: U/I/O (33-35) fleet/inventory/local — green
        // Scan tools: P/\ (36-37) dscan/probe — green
        set_range(33, 38, led_min, led_max, CLR_GREEN);
        // Map keys: M/comma (45-46) system/galaxy map — green
        set_led(45, led_min, led_max, CLR_GREEN);
        set_led(46, led_min, led_max, CLR_GREEN);
        // Workspace switch: HJKL (38-41) — rainbow pulse
        set_led_rainbow(38, led_min, led_max, 0);
        set_led_rainbow(39, led_min, led_max, 1);
        set_led_rainbow(40, led_min, led_max, 2);
        set_led_rainbow(41, led_min, led_max, 3);
        // Toggle key (bottom-right, index 49) — white
        set_led(49, led_min, led_max, CLR_WHITE);
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

void housekeeping_task_user(void) {
    // These change outside of layer_state_set, so poll them
    STATUS_LED_2(is_caps_word_on());
    STATUS_LED_3(leader_sequence_active());
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
