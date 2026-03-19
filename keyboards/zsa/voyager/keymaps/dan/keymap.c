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
    _EVE,
};

enum td_keycodes {
    TD_LAYER_TOGGLE,
};

// Single tap: toggle mouse layer (_UTIL), double tap: toggle EVE layer (_EVE)
// When exiting mouse mode, auto_mouse_reset_trigger() must be used instead of
// layer_invert() so the trackball cooldown fires and the layer doesn't
// immediately re-engage. layer_state_set_user() clears scroll state on deactivation.
static void td_layer_toggle_finished(tap_dance_state_t *state, void *user_data) {
    if (state->count == 1) {
        if (layer_state_is(_UTIL)) {
            auto_mouse_reset_trigger(true);
        } else {
            layer_on(_UTIL);
        }
    } else if (state->count == 2) {
        layer_invert(_EVE);
    }
}

tap_dance_action_t tap_dance_actions[] = {
    [TD_LAYER_TOGGLE] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, td_layer_toggle_finished, NULL),
};

enum custom_keycodes {
    EXIT_MOUSE = SAFE_RANGE,
};


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [_BASE] = LAYOUT(
        DUAL_FUNC_0,    KC_1,           KC_2,           KC_3,           KC_4,           KC_5,                                           KC_6,           KC_7,           KC_8,           KC_9,           KC_0,           KC_MINUS,
        MO(_NAV),       KC_Q,           KC_W,           KC_E,           KC_R,           KC_T,                                           KC_Y,           KC_U,           KC_I,           KC_O,           KC_P,           KC_BSLS,
        KC_LEFT_GUI,    KC_A,           KC_S,           KC_D,           KC_F,           KC_G,                                           KC_H,           KC_J,           KC_K,           KC_L,           KC_SCLN,        KC_QUOTE,
        KC_LEFT_CTRL,   KC_Z,           KC_X,           KC_C,           KC_V,           KC_B,                                           KC_N,           KC_M,           KC_COMMA,       KC_DOT,         KC_SLASH,       TD(TD_LAYER_TOGGLE),
                                                        MT(MOD_LALT, KC_BSPC), MT(MOD_LSFT, KC_TAB),                                   MT(MOD_RSFT, KC_ENTER), MT(MOD_RALT, KC_SPACE)
    ),

    [_NAV] = LAYOUT(
        _______,        KC_F1,          KC_F2,          KC_F3,          KC_F4,          KC_F5,                                          KC_F6,          KC_F7,          KC_F8,          KC_F9,          KC_F10,         KC_F11,
        _______,        _______,        _______,        _______,        _______,        _______,                                        _______,        _______,        _______,        _______,        _______,        _______,
        _______,        _______,        _______,        _______,        _______,        _______,                                        KC_LEFT,        KC_DOWN,        KC_UP,          KC_RIGHT,       _______,        _______,
        _______,        _______,        _______,        _______,        _______,        _______,                                        _______,        _______,        _______,        _______,        _______,        _______,
                                                        _______,        _______,                                                        _______,        _______
    ),

    // EVE Online module layer
    // QWERT = high slots 1-5 (F1-F5), ASDFG = mid slots 1-5 (Alt+F1-F5), ZXCVB = low slots 1-5 (Cmd+F1-F5)
    [_EVE] = LAYOUT(
        _______,        _______,        _______,        _______,        _______,        _______,                                        _______,        _______,        _______,        _______,        _______,        _______,
        _______,        KC_F1,          KC_F2,          KC_F3,          KC_F4,          KC_F5,                                          _______,        _______,        _______,        _______,        _______,        _______,
        _______,        LALT(KC_F1),    LALT(KC_F2),    LALT(KC_F3),    LALT(KC_F4),    LALT(KC_F5),                                    _______,        _______,        _______,        _______,        _______,        _______,
        _______,        LGUI(KC_F1),    LGUI(KC_F2),    LGUI(KC_F3),    LGUI(KC_F4),    LGUI(KC_F5),                                    _______,        _______,        _______,        _______,        _______,        TD(TD_LAYER_TOGGLE),
                                                        _______,        _______,                                                        _______,        _______
    ),

    // Navigator automouse layer — activates automatically when trackball moves
    [_UTIL] = LAYOUT(
        NAVIGATOR_DEC_CPI, NAVIGATOR_INC_CPI, _______,     _______,        _______,        _______,                                        _______,        _______,        _______,        _______,        _______,        _______,
        _______,        _______,        _______,        _______,        _______,        _______,                                        _______,        _______,        _______,        _______,        _______,        _______,
        _______,        _______,        _______,        _______,        _______,        _______,                                        MS_LEFT,        MS_DOWN,        MS_UP,          MS_RGHT,       _______,        _______,
        _______,        _______,        _______,        _______,        _______,        _______,                                        _______,        _______,        _______,        _______,        _______,        EXIT_MOUSE,
                                                        TOGGLE_SCROLL,  MS_BTN1,                                                        MS_BTN2,        MS_BTN3
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

    case EXIT_MOUSE:
        // Turn off the mouse layer and start the re-activation delay so
        // the trackball can re-engage it on the next deliberate movement.
        auto_mouse_reset_trigger(record->event.pressed);
        return false;
    }
    return true;
}

// Keep the mouse layer active as long as it is on — only EXIT_MOUSE turns it
// off. Without this, the layer would time out after AUTO_MOUSE_TIME ms of
// inactivity.
bool auto_mouse_activation(report_mouse_t mouse_report) {
    if (layer_state_is(_UTIL)) return true;
    return abs(mouse_report.x) > AUTO_MOUSE_THRESHOLD ||
           abs(mouse_report.y) > AUTO_MOUSE_THRESHOLD ||
           mouse_report.buttons;
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
#define CLR_PINK        255, 50, 130
#define CLR_RED         200, 0, 0
#define CLR_BLUE        0, 0, 200
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

    case _EVE:
        set_all(led_min, led_max, CLR_OFF);
        // High slots: QWERT (left row 1, indices 7-11) — red
        set_range(7, 12, led_min, led_max, CLR_RED);
        // Mid slots: ASDFG (left row 2, indices 13-17) — blue
        set_range(13, 18, led_min, led_max, CLR_BLUE);
        // Low slots: ZXCVB (left row 3, indices 19-23) — orange
        set_range(19, 24, led_min, led_max, CLR_ORANGE);
        // Toggle key (bottom-right, index 49) — white
        set_led(49, led_min, led_max, CLR_WHITE);
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

// Mouse buttons + scroll toggle on thumbs: (24) (25) (50) (51) — green
        set_led(24, led_min, led_max, CLR_GREEN);
        set_led(25, led_min, led_max, CLR_GREEN);
        set_led(50, led_min, led_max, CLR_GREEN);
        set_led(51, led_min, led_max, CLR_GREEN);
        // Mouse movement: HJKL (38-41) — pink
        set_range(38, 42, led_min, led_max, CLR_PINK);
        // Layer toggle key (49) — white
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

layer_state_t layer_state_set_user(layer_state_t state) {
    // Clear scroll mode whenever the mouse layer deactivates, so the
    // trackball x/y reports are non-zero and auto-mouse can re-engage.
    if (!layer_state_cmp(state, _UTIL)) {
        set_scrolling = false;
    }
    // LED 4: mouse layer active
    STATUS_LED_4(layer_state_cmp(state, _UTIL));
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
