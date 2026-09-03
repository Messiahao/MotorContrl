#ifndef MOTOR_APP_TYPES_H
#define MOTOR_APP_TYPES_H

#include <stdint.h>
#include "config.h"

/* Types only: no business-variable definitions or extern declarations. */
typedef struct
{
  uint8_t serial_test_rx_frame[SERIAL_TEST_FRAME_SIZE];
  uint8_t serial_test_rx_index;
} AppProtocolState;

typedef struct
{
  volatile uint32_t ioin;
  volatile uint8_t spi_ok;
  volatile uint8_t spi_test_done;
  volatile uint32_t gconf;
  volatile uint8_t gconf_ok;
  volatile uint8_t gconf_test_done;
  volatile uint32_t gstat;
  volatile uint8_t gstat_test_done;
  volatile uint32_t gstat_after_clear;
  volatile uint8_t gstat_uv_cp_clear_ok;
  volatile uint8_t gstat_clear_test_done;
  volatile uint32_t chopconf;
  volatile uint8_t static_read_test_done;
  volatile uint32_t chopconf_configured;
  volatile uint8_t low_current_config_ok;
  volatile uint8_t low_current_config_test_done;
  volatile uint32_t gstat_enabled;
  volatile uint32_t drv_status_enabled;
  volatile uint8_t enable_ok;
  volatile uint8_t enable_test_done;
  volatile uint8_t enable_active;
} AppMotionAxisState;

typedef struct
{
  volatile uint32_t serial_motion_command_count;
  volatile uint8_t serial_motion_last_axis;
  volatile uint8_t serial_motion_last_direction;
  volatile uint16_t serial_motion_last_speed_hz;
  volatile uint32_t serial_motion_last_distance_steps;
  volatile uint8_t serial_motion_last_frame_ok;
  volatile uint8_t serial_motion_last_response_ok;
  volatile uint8_t serial_motion_error_code;
  volatile uint8_t serial_motion_busy;
  volatile uint8_t serial_motion_command_pending;
  volatile uint8_t serial_motion_state;
  volatile uint8_t serial_motion_start_ok;
  volatile uint8_t serial_motion_stop_pending;
  volatile uint8_t serial_motion_limit_pending;
  volatile uint32_t serial_motion_stop_command_count;
  volatile uint32_t serial_motion_limit_stop_count;
  volatile uint32_t serial_motion_status_query_count;
  volatile uint8_t serial_motion_last_status_response_ok;
  volatile uint16_t serial_motion_last_limit_mask;
  volatile uint8_t serial_motion_last_limit_response_ok;
  volatile uint8_t serial_motion_target_axis;
  volatile uint8_t serial_motion_target_direction;
  volatile uint32_t serial_motion_target_distance_steps;
  volatile uint8_t serial_motion_last_completion_status;
  volatile uint8_t serial_motion_last_completion_error_code;
  volatile uint8_t serial_motion_last_completion_response_ok;
  volatile uint32_t serial_motion_mscnt_before;
  volatile uint32_t serial_motion_mscnt_after;
  volatile uint16_t serial_motion_mscnt_delta;
  volatile uint8_t serial_motion_mscnt_ok;
  AppMotionAxisState axis_state[SERIAL_MOTION_AXIS_COUNT];
} AppMotionState;

typedef struct
{
  volatile uint32_t serial_relay_command_count;
  volatile uint8_t serial_relay_state;
  volatile uint8_t serial_relay_last_frame_ok;
  volatile uint8_t serial_relay_last_response_ok;
  volatile uint8_t serial_relay_error_code;
  volatile uint32_t serial_brake_command_count;
  volatile uint8_t serial_brake_state;
  volatile uint8_t serial_brake_last_frame_ok;
  volatile uint8_t serial_brake_last_response_ok;
  volatile uint8_t serial_brake_error_code;
} AppAuxState;

typedef struct
{
  volatile uint8_t limit_gpio_sample_valid;
  volatile uint8_t limit_pc6_level;
  volatile uint8_t limit_pb15_level;
  volatile uint8_t limit_pb14_level;
  volatile uint8_t limit_pc9_level;
  volatile uint8_t limit_pc8_level;
  volatile uint8_t limit_pc7_level;
  volatile uint8_t limit_pa12_level;
  volatile uint8_t limit_pa11_level;
  volatile uint8_t limit_pa10_level;
  volatile uint16_t limit_active_mask;
} AppLimitState;

/* References to the original volatile ISR objects; never copy/snapshot these values. */
typedef struct
{
  volatile uint8_t *serial_motion_active;
  volatile uint8_t *serial_motion_done;
  volatile uint8_t *serial_motion_axis;
  volatile uint8_t *serial_motion_continuous;
  volatile uint32_t *serial_motion_pulses_done;
  volatile uint32_t *serial_motion_target_steps;
  volatile uint16_t *serial_motion_target_speed_hz;
  volatile uint16_t *serial_motion_current_speed_hz;
  volatile uint16_t *serial_motion_peak_speed_hz;
  volatile uint8_t *serial_motion_profile_phase;
  volatile uint32_t *serial_motion_profile_accel_steps;
  volatile uint32_t *serial_motion_profile_cruise_steps;
  volatile uint32_t *serial_motion_profile_decel_steps;
  volatile uint32_t *serial_motion_profile_update_count;
  volatile uint32_t *serial_motion_last_period_ticks;
  volatile uint8_t *serial_motion_profile_error;
  uint8_t (*write_initial_speed)(void);
} AppMotionIrq;

#endif /* MOTOR_APP_TYPES_H */
