#include "matrix.h"
#include "md_rgb_matrix.h"
#include "suspend.h"

// Declared in tmk_core/protocol/arm_atsam/i2c_master.h; forward-declared here to
// avoid pulling in drivers/i2c_master.h which shadows the arm_atsam-specific header.
void I2C3733_Control_Set(uint8_t state);

/** \brief Suspend power down
 *
 * FIXME: needs doc
 */
void suspend_power_down(void) {
#ifdef RGB_MATRIX_ENABLE
    I2C3733_Control_Set(0); // Disable LED driver
#endif

    suspend_power_down_kb();
}

/** \brief run immediately after wakeup
 *
 * FIXME: needs doc
 */
void suspend_wakeup_init(void) {
#ifdef RGB_MATRIX_ENABLE
#    ifdef USE_MASSDROP_CONFIGURATOR
    if (led_enabled) {
        I2C3733_Control_Set(1);
    }
#    else
    I2C3733_Control_Set(1);
#    endif
#endif

    suspend_wakeup_init_kb();
}
