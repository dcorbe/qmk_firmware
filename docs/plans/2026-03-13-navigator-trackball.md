# Navigator Trackball Port — Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Port ZSA's Navigator trackball support from their community module into the upstream Voyager keyboard definition, enabling the PAW3805EK trackball sensor (over I2C→SPI bridge) with smooth scrolling, turbo/aim modes, and automouse layer switching.

**Architecture:** The Navigator trackball connects via TRRS using I2C (already available on the Voyager). An SCI18IS606 chip bridges I2C to SPI, communicating with a PAW3805EK motion sensor. We add four files to `keyboards/zsa/voyager/`: the sensor driver (`navigator_trackball.c/.h`) and the scroll/speed overlay (`navigator.c/.h`). Keycodes are added to `voyager.h`, and `voyager.c` gets `pointing_device_task_kb` for the scroll overlay and navigator keycode handling in `process_record_kb`. Keymaps opt in via `POINTING_DEVICE_ENABLE = yes` + `POINTING_DEVICE_DRIVER = custom` in their `rules.mk`.

**Tech Stack:** QMK firmware (C), STM32F303, I2C, custom pointing device driver API

**Source reference:** All navigator code is ported from `zsa/qmk_modules` repo, `navigator_trackball/` directory, GPL-2.0-or-later.

---

### Task 1: Add navigator trackball sensor driver

**Files:**
- Create: `keyboards/zsa/voyager/navigator_trackball.h`
- Create: `keyboards/zsa/voyager/navigator_trackball.c`

**Step 1: Create `navigator_trackball.h`**

```c
// Copyright 2025 ZSA Technology Labs, Inc <contact@zsa.io>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "pointing_device.h"

#ifndef NAVIGATOR_TRACKBALL_ADDRESS
#    define NAVIGATOR_TRACKBALL_ADDRESS 0x50
#endif

#ifndef NAVIGATOR_TRACKBALL_CPI
#    define NAVIGATOR_TRACKBALL_CPI 40
#endif

#ifndef NAVIGATOR_TRACKBALL_CPI_TICK
#    define NAVIGATOR_TRACKBALL_CPI_TICK 5
#endif

#define NAVIGATOR_TRACKBALL_CPI_MAX 125

#ifndef NAVIGATOR_TRACKBALL_TIMEOUT
#    define NAVIGATOR_TRACKBALL_TIMEOUT 100
#endif

#define NAVIGATOR_TRACKBALL_READ 7
#define NAVIGATOR_TRACKBALL_PROBE 1000

#define NCS_PIN 0x01
#define PAW3805EK_ID 0x31

#define SCI18IS606_CONF 0xDC

#define SCI18IS606_RW_SPI 0x00
#define SCI18IS606_CONF_SPI 0xF0
#define SCI18IS606_CLR_INT 0xF1
#define SCI18IS606_GET_ID 0xFE

#define WRITE_REG_BIT 0x80

typedef struct {
    uint8_t reg;
    uint8_t data;
} paw3805ek_reg_seq_t;

void restore_cpi(uint8_t cpi);
```

**Step 2: Create `navigator_trackball.c`**

This is the custom pointing device driver. It implements the four `pointing_device_driver_*` functions that QMK's custom driver interface requires. Note: upstream QMK expects `bool pointing_device_driver_init(void)` (not `void`).

```c
// Copyright 2025 ZSA Technology Labs, Inc <contact@zsa.io>
// SPDX-License-Identifier: GPL-2.0-or-later

// QMK driver for the ZSA Navigator Trackball.
// Hardware: SCI18IS606 (I2C→SPI bridge) + PAW3805EK (motion sensor).
// The bridge converts TRRS I2C into SPI for the sensor.

#include "i2c_master.h"
#include "navigator_trackball.h"
#include <stdint.h>
#include <stdio.h>
#include "quantum.h"

uint8_t current_cpi = NAVIGATOR_TRACKBALL_CPI;

uint8_t has_motion = 0;

uint8_t trackball_init = 0;

deferred_token callback_token = 0;

// Boot sequence for the PAW3805EK sensor.
paw3805ek_reg_seq_t paw3805ek_configure_seq[] = {
    {0x06, 0x80},                 // Software reset
    {0x00, 0x00},                 // Request the sensor ID
    {0x09 | WRITE_REG_BIT, 0x5A}, // Disable write protection
#ifdef MOUSE_EXTENDED_REPORT
    {0x19 | WRITE_REG_BIT, 0x30}, // 16-bit motion data
#else
    {0x19 | WRITE_REG_BIT, 0x34}, // 8-bit motion data
#endif
    {0x09 | WRITE_REG_BIT, 0x00}, // Enable write protection
};

i2c_status_t sci18is606_write(uint8_t *data, uint8_t length) {
    return i2c_transmit(NAVIGATOR_TRACKBALL_ADDRESS, data, length, NAVIGATOR_TRACKBALL_TIMEOUT);
}

i2c_status_t sci18is606_read(uint8_t *data, uint8_t length) {
    return i2c_receive(NAVIGATOR_TRACKBALL_ADDRESS, data, length, NAVIGATOR_TRACKBALL_TIMEOUT);
}

i2c_status_t sci18is606_spi_tx(uint8_t *data, uint8_t length, bool read) {
    i2c_status_t status = sci18is606_write(data, length);
    wait_us(length * 15);
    if (read) {
        status = sci18is606_read(data, length);
    }
    if (status != I2C_STATUS_SUCCESS) {
        trackball_init = 0;
    }
    return status;
}

i2c_status_t sci18is606_configure(void) {
    uint8_t      spi_conf[2] = {SCI18IS606_CONF_SPI, SCI18IS606_CONF};
    i2c_status_t status      = sci18is606_write(spi_conf, 2);
    wait_ms(10);
    if (status != I2C_STATUS_SUCCESS) {
        trackball_init = 0;
    }
    return status;
}

bool paw3805ek_set_cpi(void) {
    paw3805ek_reg_seq_t cpi_reg_seq[] = {
        {0x09 | WRITE_REG_BIT, 0x5A},
        {0x0D | WRITE_REG_BIT, current_cpi},
        {0x0E | WRITE_REG_BIT, current_cpi},
        {0x09 | WRITE_REG_BIT, 0x00},
    };

    for (uint8_t i = 0; i < sizeof(cpi_reg_seq) / sizeof(paw3805ek_reg_seq_t); i++) {
        uint8_t buf[3];
        buf[0] = NCS_PIN;
        buf[1] = cpi_reg_seq[i].reg;
        buf[2] = cpi_reg_seq[i].data;
        if (sci18is606_spi_tx(buf, 3, true) != I2C_STATUS_SUCCESS) {
            return false;
        }
    }
    return true;
}

bool paw3805ek_configure(void) {
    for (uint8_t i = 0; i < sizeof(paw3805ek_configure_seq) / sizeof(paw3805ek_reg_seq_t); i++) {
        uint8_t buf[3];
        buf[0] = NCS_PIN;
        buf[1] = paw3805ek_configure_seq[i].reg;
        buf[2] = paw3805ek_configure_seq[i].data;
        if (sci18is606_spi_tx(buf, 3, true) != I2C_STATUS_SUCCESS) {
            return false;
        }
        wait_ms(1);

        if (i == 1 && buf[1] != PAW3805EK_ID) {
            return false;
        }
    }
    return true;
}

bool paw3805ek_has_motion(void) {
    uint8_t motion[3] = {0x01, 0x02, 0x00};
    if (sci18is606_spi_tx(motion, 3, true) != I2C_STATUS_SUCCESS) {
        return false;
    }
    return motion[1] & 0x80;
}

void paw3805ek_read_motion(report_mouse_t *mouse_report) {
#ifdef MOUSE_EXTENDED_REPORT
    uint8_t delta_x_l[2] = {0x01, 0x03};
    if (sci18is606_spi_tx(delta_x_l, 3, true) != I2C_STATUS_SUCCESS) return;

    uint8_t delta_y_l[2] = {0x01, 0x04};
    if (sci18is606_spi_tx(delta_y_l, 3, true) != I2C_STATUS_SUCCESS) return;

    uint8_t delta_x_h[2] = {0x01, 0x11};
    if (sci18is606_spi_tx(delta_x_h, 3, true) != I2C_STATUS_SUCCESS) return;

    uint8_t delta_y_h[2] = {0x01, 0x12};
    if (sci18is606_spi_tx(delta_y_h, 3, true) != I2C_STATUS_SUCCESS) return;

    mouse_report->x = (int16_t)((delta_x_h[1] << 8) | delta_x_l[1]);
    mouse_report->y = (int16_t)((delta_y_h[1] << 8) | delta_y_l[1]);
#else
    uint8_t delta_x[2] = {0x01, 0x03};
    if (sci18is606_spi_tx(delta_x, 3, true) != I2C_STATUS_SUCCESS) return;

    uint8_t delta_y[2] = {0x01, 0x04};
    if (sci18is606_spi_tx(delta_y, 3, true) != I2C_STATUS_SUCCESS) return;

    mouse_report->x = delta_x[1];
    mouse_report->y = delta_y[1];
#endif
}

uint32_t sci18is606_read_callback(uint32_t trigger_time, void *cb_arg) {
    if (!trackball_init) {
        pointing_device_driver_init();
        return NAVIGATOR_TRACKBALL_PROBE;
    }
    if (paw3805ek_has_motion()) {
        has_motion = 1;
    }
    return NAVIGATOR_TRACKBALL_READ;
}

bool pointing_device_driver_init(void) {
    i2c_init();
    if (sci18is606_configure() == I2C_STATUS_SUCCESS) {
        paw3805ek_configure();
    } else {
        return false;
    }

    trackball_init = 1;
    restore_cpi(current_cpi);
    if (!callback_token) {
        callback_token = defer_exec(NAVIGATOR_TRACKBALL_READ, sci18is606_read_callback, NULL);
    }
    return true;
}

report_mouse_t pointing_device_driver_get_report(report_mouse_t mouse_report) {
    if (!trackball_init) {
        return mouse_report;
    }

    if (has_motion) {
        has_motion = 0;
        paw3805ek_read_motion(&mouse_report);
    }
    return mouse_report;
}

uint16_t pointing_device_driver_get_cpi(void) {
    return current_cpi;
}

void restore_cpi(uint8_t cpi) {
    current_cpi = cpi;
    paw3805ek_set_cpi();
}

void pointing_device_driver_set_cpi(uint16_t cpi) {
    if (cpi == 0) {
        if (current_cpi > NAVIGATOR_TRACKBALL_CPI_TICK) {
            current_cpi -= NAVIGATOR_TRACKBALL_CPI_TICK;
            paw3805ek_set_cpi();
        }
    } else {
        if (current_cpi <= NAVIGATOR_TRACKBALL_CPI_MAX - NAVIGATOR_TRACKBALL_CPI_TICK) {
            current_cpi += NAVIGATOR_TRACKBALL_CPI_TICK;
            paw3805ek_set_cpi();
        }
    }
}
```

**Step 3: Verify files compile in isolation**

No compile check yet — these need to be wired into the build in Task 3.

**Step 4: Commit**

```bash
git add keyboards/zsa/voyager/navigator_trackball.c keyboards/zsa/voyager/navigator_trackball.h
git commit -m "feat: add Navigator trackball sensor driver for Voyager

Port PAW3805EK + SCI18IS606 I2C-to-SPI bridge driver from
zsa/qmk_modules navigator_trackball community module."
```

---

### Task 2: Add navigator scroll/turbo/aim overlay

**Files:**
- Create: `keyboards/zsa/voyager/navigator.h`
- Create: `keyboards/zsa/voyager/navigator.c`

**Step 1: Create `navigator.h`**

```c
// Copyright 2025 ZSA Technology Labs, Inc <contact@zsa.io>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifndef NAVIGATOR_SCROLL_DIVIDER
#    define NAVIGATOR_SCROLL_DIVIDER 10
#endif

#ifndef NAVIGATOR_SCROLL_THRESHOLD
#    define NAVIGATOR_SCROLL_THRESHOLD 0f
#endif

#ifndef NAVIGATOR_SCROLL_ACCELERATION
#    define NAVIGATOR_SCROLL_ACCELERATION 1.5f
#endif

#ifndef NAVIGATOR_SCROLL_MAX_SPEED
#    define NAVIGATOR_SCROLL_MAX_SPEED 8.0f
#endif

#ifndef NAVIGATOR_TRACKBALL_ROTATION
#    define NAVIGATOR_TRACKBALL_ROTATION 0
#endif

#define _NAVIGATOR_ROTATION (NAVIGATOR_TRACKBALL_ROTATION % 360)

#ifndef NAVIGATOR_TURBO_MULTIPLIER
#    define NAVIGATOR_TURBO_MULTIPLIER 3
#endif

#ifndef NAVIGATOR_AIM_DIVIDER
#    define NAVIGATOR_AIM_DIVIDER 3
#endif

extern bool set_scrolling;
extern bool navigator_turbo;
extern bool navigator_aim;

report_mouse_t pointing_device_task_navigator(report_mouse_t mouse_report);
```

**Step 2: Create `navigator.c`**

```c
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
```

**Step 3: Commit**

```bash
git add keyboards/zsa/voyager/navigator.c keyboards/zsa/voyager/navigator.h
git commit -m "feat: add Navigator scroll/turbo/aim overlay for Voyager

Port smooth scrolling algorithm with acceleration, turbo mode,
aim mode, and configurable rotation from zsa/qmk_modules."
```

---

### Task 3: Wire navigator into Voyager keyboard definition

**Files:**
- Modify: `keyboards/zsa/voyager/voyager.h:18-21` — add navigator keycodes
- Modify: `keyboards/zsa/voyager/voyager.c:224-268` — add navigator keycode handling + pointing_device_task_kb

**Step 1: Add keycodes to `voyager.h`**

Replace the existing `enum voyager_keycodes` (lines 18-21) with:

```c
enum voyager_keycodes {
    TOGGLE_LAYER_COLOR = QK_KB,
    LED_LEVEL,
#ifdef POINTING_DEVICE_ENABLE
    NAVIGATOR_INC_CPI,
    NAVIGATOR_DEC_CPI,
    NAVIGATOR_TURBO,
    NAVIGATOR_AIM,
    DRAG_SCROLL,
    TOGGLE_SCROLL,
#endif
};
```

Also add a conditional include after line 7 (`#include "quantum.h"`):

```c
#ifdef POINTING_DEVICE_ENABLE
#    include "navigator.h"
#    include "navigator_trackball.h"
#endif
```

**Step 2: Add navigator handling to `voyager.c`**

Add at the top of `voyager.c`, after line 5 (`#include "voyager.h"`):

```c
#ifdef POINTING_DEVICE_ENABLE
#    include "navigator.h"
#endif
```

Add `pointing_device_task_kb` before the existing `process_record_kb` (before line 224):

```c
#ifdef POINTING_DEVICE_ENABLE
report_mouse_t pointing_device_task_kb(report_mouse_t mouse_report) {
    mouse_report = pointing_device_task_navigator(mouse_report);
    return pointing_device_task_user(mouse_report);
}
#endif
```

Add navigator keycode cases inside `process_record_kb`'s switch statement (after the `RGB_MATRIX_ENABLE` block, before the closing `}` on line 266):

```c
#ifdef POINTING_DEVICE_ENABLE
        case NAVIGATOR_INC_CPI:
            if (record->event.pressed) pointing_device_set_cpi(1);
            return false;
        case NAVIGATOR_DEC_CPI:
            if (record->event.pressed) pointing_device_set_cpi(0);
            return false;
        case NAVIGATOR_TURBO:
            navigator_turbo = record->event.pressed;
            break;
        case NAVIGATOR_AIM:
            navigator_aim = record->event.pressed;
            break;
        case DRAG_SCROLL:
            set_scrolling = record->event.pressed;
            break;
        case TOGGLE_SCROLL:
            if (record->event.pressed) set_scrolling = !set_scrolling;
            break;
#endif
```

**Step 3: Commit**

```bash
git add keyboards/zsa/voyager/voyager.h keyboards/zsa/voyager/voyager.c
git commit -m "feat: wire Navigator trackball into Voyager keyboard definition

Add navigator keycodes (CPI, turbo, aim, scroll) to voyager_keycodes
enum. Add pointing_device_task_kb for scroll overlay. Handle navigator
keycodes in process_record_kb. All behind POINTING_DEVICE_ENABLE guard."
```

---

### Task 4: Update dan keymap for Navigator support

**Files:**
- Modify: `keyboards/zsa/voyager/keymaps/dan/rules.mk`
- Modify: `keyboards/zsa/voyager/keymaps/dan/config.h`
- Modify: `keyboards/zsa/voyager/keymaps/dan/keymap.c`

**Step 1: Update `rules.mk`**

Replace entire contents with:

```makefile
LEADER_ENABLE = yes
LAYER_LOCK_ENABLE = yes
POINTING_DEVICE_ENABLE = yes
POINTING_DEVICE_DRIVER = custom
SRC += keyboards/zsa/voyager/navigator_trackball.c
SRC += keyboards/zsa/voyager/navigator.c
```

**Step 2: Update `config.h`**

Add after the existing `#define LAYER_STATE_8BIT` line:

```c

// Navigator trackball
#define MOUSE_EXTENDED_REPORT
#define POINTING_DEVICE_AUTO_MOUSE_ENABLE
#define AUTO_MOUSE_DEFAULT_LAYER 2
#define AUTO_MOUSE_TIME 650
#define AUTO_MOUSE_THRESHOLD 10
#define NAVIGATOR_SCROLL_DIVIDER 50
#define NAVIGATOR_SCROLL_INVERT_Y
```

Also remove `#define LAYER_STATE_8BIT` — automouse needs more than 8 layer states if we grow beyond 8 layers, and it's safer without. (Actually, with only 3 layers this is fine to keep. Keep it.)

**Step 3: Update `keymap.c` layer 2**

Replace the `_UTIL` layer (lines 32-39) with the Navigator automouse layer matching the Oryx export:

```c
    // Navigator automouse layer — activates automatically when trackball moves
    [_UTIL] = LAYOUT(
        NAVIGATOR_DEC_CPI, NAVIGATOR_INC_CPI, _______,     _______,        _______,        QK_LLCK,                                        _______,        _______,        _______,        _______,        _______,        _______,
        _______,        _______,        _______,        _______,        KC_MS_BTN3,     TOGGLE_SCROLL,                                  _______,        _______,        _______,        _______,        _______,        _______,
        _______,        _______,        _______,        KC_MS_BTN2,     KC_MS_BTN1,     DRAG_SCROLL,                                    _______,        _______,        _______,        _______,        _______,        _______,
        _______,        _______,        _______,        _______,        _______,        _______,                                        _______,        _______,        _______,        _______,        _______,        _______,
                                                        _______,        _______,                                                        _______,        _______
    ),
```

Also update the layer 2 comment — remove the TODO.

**Step 4: Compile**

Run: `qmk compile -kb zsa/voyager -km dan`
Expected: Clean build with no errors or warnings.

**Step 5: Commit**

```bash
git add keyboards/zsa/voyager/keymaps/dan/rules.mk keyboards/zsa/voyager/keymaps/dan/config.h keyboards/zsa/voyager/keymaps/dan/keymap.c
git commit -m "feat: enable Navigator trackball in dan keymap

Enable pointing device with custom driver, configure automouse on
layer 2 with scroll/CPI/mouse button controls matching Oryx layout."
```

---

### Task 5: Verify build and flash

**Step 1: Full clean build**

Run: `qmk compile -kb zsa/voyager -km dan`
Expected: `[OK]` for all compilation units, successful link, `.bin` output.

**Step 2: Check for warnings**

Run: `qmk compile -kb zsa/voyager -km dan 2>&1 | grep -iE 'warn|error'`
Expected: No output.

**Step 3: Verify default keymap still compiles (no regressions)**

Run: `qmk compile -kb zsa/voyager -km default`
Expected: Clean build — navigator code should not affect keymaps that don't enable `POINTING_DEVICE_ENABLE`.

**Step 4: Flash (user-driven)**

Run: `qmk flash -kb zsa/voyager -km dan`
Expected: Firmware flashed. Trackball should move the cursor. Layer 2 activates automatically on trackball movement (automouse). DRAG_SCROLL held = scroll mode. TOGGLE_SCROLL tapped = toggle scroll. CPI keys adjust sensitivity.
