ifneq ($(ARM_ATSAM),)
  PLATFORM_KEY = arm_atsam

  MCU_FAMILY = SAMD
  MCU_SERIES = SAMD51

  # ARM Cortex-M4 core
  ARMV = 7
endif

