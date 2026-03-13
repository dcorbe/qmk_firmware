// Copyright 2026 Daniel
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

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
        NAVIGATOR_DEC_CPI, NAVIGATOR_INC_CPI, _______,     _______,        _______,        QK_LLCK,                                        _______,        _______,        _______,        _______,        _______,        _______,
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

void leader_end_user(void) {
    if (leader_sequence_two_keys(KC_G, KC_P)) {
        SEND_STRING("git push\n");
    } else if (leader_sequence_two_keys(KC_G, KC_S)) {
        SEND_STRING("git status\n");
    } else if (leader_sequence_two_keys(KC_G, KC_L)) {
        SEND_STRING("git log --oneline\n");
    }
}
