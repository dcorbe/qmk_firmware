// Copyright 2026 Daniel
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

enum layers {
    _BASE,
    _NAV,
    _SYM,
    _FUNC,
};

// Tap: KC_1, Hold: KC_ESC
#define TD_1_ESC LT(0, KC_1)

// Bottom row corner mod-taps
#define CT_Z    LCTL_T(KC_Z)
#define GUI_SL  RGUI_T(KC_SLSH)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    // Base: QWERTY — Voyager-style thumb mods, corner mods for Ctrl/Cmd
    //
    // ┌─────┬─────┬─────┬─────┬─────┐   ┌─────┬─────┬─────┬─────┬─────┐
    // │  Q  │  W  │  E  │  R  │  T  │   │  Y  │  U  │  I  │  O  │  P  │
    // ├─────┼─────┼─────┼─────┼─────┤   ├─────┼─────┼─────┼─────┼─────┤
    // │  A  │  S  │  D  │  F  │  G  │   │  H  │  J  │  K  │  L  │  ;  │
    // ├─────┼─────┼─────┼─────┼─────┤   ├─────┼─────┼─────┼─────┼─────┤
    // │Z/⌃  │  X  │  C  │  V  │  B  │   │  N  │  M  │  ,  │  .  │ //⌘ │
    // └─────┴─────┴──┬──┴──┬──┴──┬──┘   └──┬──┴──┬──┴──┬──┴─────┴─────┘
    //                │ NAV │⌥/Bsp│⇧/Tab│   │⇧/Ent│⌥/Spc│ SYM │
    //                └─────┴─────┴─────┘   └─────┴─────┴─────┘
    [_BASE] = LAYOUT_split_3x5_3(
        KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,         KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,
        KC_A,    KC_S,    KC_D,    KC_F,    KC_G,         KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN,
        CT_Z,    KC_X,    KC_C,    KC_V,    KC_B,         KC_N,    KC_M,    KC_COMM, KC_DOT,  GUI_SL,
              MO(_NAV), MT(MOD_LALT, KC_BSPC), MT(MOD_LSFT, KC_TAB),  MT(MOD_RSFT, KC_ENT), MT(MOD_RALT, KC_SPC), MO(_SYM)
    ),

    // Nav: numbers, arrows, navigation
    //
    // ┌─────┬─────┬─────┬─────┬─────┐   ┌─────┬─────┬─────┬─────┬─────┐
    // │ 1/⎋ │  2  │  3  │  4  │  5  │   │  6  │  7  │  8  │  9  │  0  │
    // ├─────┼─────┼─────┼─────┼─────┤   ├─────┼─────┼─────┼─────┼─────┤
    // │     │     │     │     │     │   │  ←  │  ↓  │  ↑  │  →  │  '  │
    // ├─────┼─────┼─────┼─────┼─────┤   ├─────┼─────┼─────┼─────┼─────┤
    // │     │     │     │     │     │   │Home │PgDn │PgUp │ End │     │
    // └─────┴─────┴──┬──┴──┬──┴──┬──┘   └──┬──┴──┬──┴──┬──┴─────┴─────┘
    //                │ [*] │     │     │   │     │     │FUNC │
    //                └─────┴─────┴─────┘   └─────┴─────┴─────┘
    [_NAV] = LAYOUT_split_3x5_3(
        TD_1_ESC, KC_2,    KC_3,    KC_4,    KC_5,         KC_6,    KC_7,    KC_8,    KC_9,    KC_0,
        XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,     KC_LEFT, KC_DOWN, KC_UP,   KC_RGHT, KC_QUOT,
        XXXXXXX,  XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,     KC_HOME, KC_PGDN, KC_PGUP, KC_END,  XXXXXXX,
                            _______, _______, _______,      _______, _______, MO(_FUNC)
    ),

    // Sym: symbols and punctuation
    //
    // ┌─────┬─────┬─────┬─────┬─────┐   ┌─────┬─────┬─────┬─────┬─────┐
    // │  !  │  @  │  #  │  $  │  %  │   │  ^  │  &  │  *  │  -  │  =  │
    // ├─────┼─────┼─────┼─────┼─────┤   ├─────┼─────┼─────┼─────┼─────┤
    // │  `  │  ~  │  (  │  )  │  \  │   │  |  │  [  │  ]  │  {  │  }  │
    // ├─────┼─────┼─────┼─────┼─────┤   ├─────┼─────┼─────┼─────┼─────┤
    // │     │     │  _  │  +  │     │   │     │     │     │     │     │
    // └─────┴─────┴──┬──┴──┬──┴──┬──┘   └──┬──┴──┬──┴──┬──┴─────┴─────┘
    //                │FUNC │     │     │   │     │     │ [*] │
    //                └─────┴─────┴─────┘   └─────┴─────┴─────┘
    [_SYM] = LAYOUT_split_3x5_3(
        KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC,     KC_CIRC, KC_AMPR, KC_ASTR, KC_MINS, KC_EQL,
        KC_GRV,  KC_TILD, KC_LPRN, KC_RPRN, KC_BSLS,     KC_PIPE, KC_LBRC, KC_RBRC, KC_LCBR, KC_RCBR,
        XXXXXXX, XXXXXXX, KC_UNDS, KC_PLUS, XXXXXXX,     XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,
                          MO(_FUNC), _______, _______,    _______, _______, _______
    ),

    // Func: F-keys, media, boot (Nav + Sym outer thumbs together)
    //
    // ┌─────┬─────┬─────┬─────┬─────┐   ┌─────┬─────┬─────┬─────┬─────┐
    // │ F1  │ F2  │ F3  │ F4  │ F5  │   │ F6  │ F7  │ F8  │ F9  │ F10 │
    // ├─────┼─────┼─────┼─────┼─────┤   ├─────┼─────┼─────┼─────┼─────┤
    // │ F11 │ F12 │     │     │     │   │     │ Vol-│ Vol+│Mute │     │
    // ├─────┼─────┼─────┼─────┼─────┤   ├─────┼─────┼─────┼─────┼─────┤
    // │BOOT │     │     │     │     │   │     │Prev │Next │Play │     │
    // └─────┴─────┴──┬──┴──┬──┴──┬──┘   └──┬──┴──┬──┴──┬──┴─────┴─────┘
    //                │     │     │     │   │     │     │     │
    //                └─────┴─────┴─────┘   └─────┴─────┴─────┘
    [_FUNC] = LAYOUT_split_3x5_3(
        KC_F1,   KC_F2,   KC_F3,   KC_F4,   KC_F5,       KC_F6,   KC_F7,   KC_F8,   KC_F9,   KC_F10,
        KC_F11,  KC_F12,  XXXXXXX, XXXXXXX, XXXXXXX,     XXXXXXX, KC_VOLD, KC_VOLU, KC_MUTE, XXXXXXX,
        QK_BOOT, XXXXXXX, XXXXXXX, XXXXXXX, XXXXXXX,     XXXXXXX, KC_MPRV, KC_MNXT, KC_MPLY, XXXXXXX,
                          _______, _______, _______,      _______, _______, _______
    ),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
    case TD_1_ESC:
        if (record->tap.count > 0) {
            if (record->event.pressed) {
                register_code16(KC_1);
            } else {
                unregister_code16(KC_1);
            }
        } else {
            if (record->event.pressed) {
                register_code16(KC_ESC);
            } else {
                unregister_code16(KC_ESC);
            }
        }
        return false;
    }
    return true;
}
