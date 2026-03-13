// Copyright 2025 ZSA Technology Labs, Inc <contact@zsa.io>
// SPDX-License-Identifier: GPL-2.0-or-later

// QMK driver for the ZSA Navigator Trackball.
// Hardware: SCI18IS606 (I2C→SPI bridge) + PAW3805EK (motion sensor).
// The bridge converts TRRS I2C into SPI for the sensor.

#include "i2c_master.h"
#include "navigator_trackball.h"
#include <stdint.h>
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
    uint8_t delta_x_l[3] = {0x01, 0x03, 0x00};
    if (sci18is606_spi_tx(delta_x_l, 3, true) != I2C_STATUS_SUCCESS) return;

    uint8_t delta_y_l[3] = {0x01, 0x04, 0x00};
    if (sci18is606_spi_tx(delta_y_l, 3, true) != I2C_STATUS_SUCCESS) return;

    uint8_t delta_x_h[3] = {0x01, 0x11, 0x00};
    if (sci18is606_spi_tx(delta_x_h, 3, true) != I2C_STATUS_SUCCESS) return;

    uint8_t delta_y_h[3] = {0x01, 0x12, 0x00};
    if (sci18is606_spi_tx(delta_y_h, 3, true) != I2C_STATUS_SUCCESS) return;

    mouse_report->x = (int16_t)((delta_x_h[1] << 8) | delta_x_l[1]);
    mouse_report->y = (int16_t)((delta_y_h[1] << 8) | delta_y_l[1]);
#else
    uint8_t delta_x[3] = {0x01, 0x03, 0x00};
    if (sci18is606_spi_tx(delta_x, 3, true) != I2C_STATUS_SUCCESS) return;

    uint8_t delta_y[3] = {0x01, 0x04, 0x00};
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
