#ifndef MOTOR_APP_TYPES_H
#define MOTOR_APP_TYPES_H

#include <stdint.h>
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Types only: no business-variable definitions or extern declarations. */
typedef struct
{
  uint8_t serial_test_rx_frame[SERIAL_TEST_FRAME_SIZE];
  uint8_t serial_test_rx_index;
  volatile uint32_t serial_test_command_count;
  volatile uint32_t serial_test_frame_error_count;
  volatile uint32_t serial_test_rx_byte_count;
  volatile uint32_t serial_test_uart_error_count;
  volatile uint8_t serial_test_last_rx_byte;
  volatile uint8_t serial_test_last_frame_ok;
  volatile uint8_t serial_test_last_response_ok;
  volatile uint32_t serial_test_build_marker;
} AppProtocolState;

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
  uint32_t limit_gpio_poll_tick;
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

typedef struct
{
  uint32_t x_tmc5160_spi_test_tick;
  uint8_t x_tmc5160_spi_test_pending;
  volatile uint32_t x_tmc5160_ioin;
  volatile uint8_t x_tmc5160_spi_ok;
  volatile uint8_t x_tmc5160_spi_test_done;
  volatile uint32_t x_tmc5160_gconf;
  volatile uint8_t x_tmc5160_gconf_ok;
  volatile uint8_t x_tmc5160_gconf_test_done;
  volatile uint32_t x_tmc5160_gstat;
  volatile uint8_t x_tmc5160_gstat_test_done;
  uint32_t x_tmc5160_gstat_clear_tick;
  uint8_t x_tmc5160_gstat_clear_pending;
  volatile uint32_t x_tmc5160_gstat_after_clear;
  volatile uint8_t x_tmc5160_gstat_uv_cp_clear_ok;
  volatile uint8_t x_tmc5160_gstat_clear_test_done;
  volatile uint32_t x_tmc5160_chopconf;
  volatile uint8_t x_tmc5160_static_read_test_done;
  volatile uint32_t x_tmc5160_chopconf_configured;
  volatile uint8_t x_tmc5160_low_current_config_ok;
  volatile uint8_t x_tmc5160_low_current_config_test_done;
  uint32_t x_tmc5160_enable_test_tick;
  uint8_t x_tmc5160_enable_test_pending;
  volatile uint32_t x_tmc5160_gstat_enabled;
  volatile uint32_t x_tmc5160_drv_status_enabled;
  volatile uint8_t x_tmc5160_enable_test_ok;
  volatile uint8_t x_tmc5160_enable_test_done;
  volatile uint8_t x_tmc5160_enable_test_active;
  uint32_t x_tmc5160_step_test_tick;
  uint8_t x_tmc5160_step_test_state;
  volatile uint32_t x_tmc5160_mscnt_before_step;
  volatile uint32_t x_tmc5160_mscnt_after_step;
  volatile uint16_t x_tmc5160_mscnt_step_delta;
  volatile uint8_t x_tmc5160_step_test_ok;
  volatile uint8_t x_tmc5160_step_test_done;
  uint32_t x_tmc5160_multi_step_test_tick;
  uint8_t x_tmc5160_multi_step_test_state;
  uint8_t x_tmc5160_multi_step_test_count;
  volatile uint32_t x_tmc5160_mscnt_before_multi_step;
  volatile uint32_t x_tmc5160_mscnt_after_multi_step;
  volatile uint16_t x_tmc5160_mscnt_multi_step_delta;
  volatile uint8_t x_tmc5160_multi_step_test_ok;
  volatile uint8_t x_tmc5160_multi_step_test_done;
  uint32_t x_tmc5160_motion_test_start_tick;
  uint8_t x_tmc5160_motion_test_state;
  volatile uint8_t x_tmc5160_motion_test_active;
  volatile uint8_t x_tmc5160_motion_test_done;
} AppTmcState;

/* References to the original volatile ISR objects; never copy/snapshot these values. */
typedef struct
{
  volatile uint8_t *serial_motion_active;
  volatile uint8_t *serial_motion_done;
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

#ifdef __cplusplus
}
#endif

#endif /* MOTOR_APP_TYPES_H */
