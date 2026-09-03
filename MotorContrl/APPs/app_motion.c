#include "app_motion.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"

static uint8_t Serial_Motion_SendResponse(AppMotionState *motion, const AppMotionIrq *irq,
                                           uint8_t status, uint8_t error_code)
{
  uint8_t i;
  uint8_t response[SERIAL_TEST_FRAME_SIZE];

  response[SERIAL_FRAME_HEAD0_INDEX] = SERIAL_RESPONSE_HEAD;
  response[SERIAL_FRAME_HEAD1_INDEX] = SERIAL_RESPONSE_HEAD;
  response[SERIAL_FRAME_COMMAND_INDEX] = SERIAL_COMMAND_MOTION;
  response[SERIAL_FRAME_SUBCOMMAND_INDEX] = status;
  if ((status == SERIAL_STATUS_ACCEPTED) || (status == SERIAL_STATUS_DONE) || (status == SERIAL_STATUS_STOPPED))
  {
    response[SERIAL_FRAME_DATA0_INDEX] = motion->serial_motion_target_axis;
    response[SERIAL_FRAME_DATA1_INDEX] = motion->serial_motion_target_direction;
    response[SERIAL_FRAME_DATA2_INDEX] = (uint8_t)((*irq->serial_motion_target_speed_hz) >> SERIAL_BYTE1_SHIFT);
    response[SERIAL_FRAME_DATA3_INDEX] = (uint8_t)(*irq->serial_motion_target_speed_hz);
    response[SERIAL_FRAME_DATA4_INDEX] = (uint8_t)(motion->serial_motion_target_distance_steps >> SERIAL_BYTE3_SHIFT);
    response[SERIAL_FRAME_DATA5_INDEX] = (uint8_t)(motion->serial_motion_target_distance_steps >> SERIAL_BYTE2_SHIFT);
    response[SERIAL_FRAME_DATA6_INDEX] = (uint8_t)(motion->serial_motion_target_distance_steps >> SERIAL_BYTE1_SHIFT);
    response[SERIAL_FRAME_DATA7_INDEX] = (uint8_t)motion->serial_motion_target_distance_steps;
  }
  else
  {
    response[SERIAL_FRAME_DATA0_INDEX] = error_code;
    for (i = SERIAL_DATA_SECOND; i < SERIAL_TAIL_FIRST; i++)
    {
      response[i] = 0U;
    }
  }
  response[SERIAL_FRAME_TAIL0_INDEX] = SERIAL_RESPONSE_TAIL;
  response[SERIAL_FRAME_TAIL1_INDEX] = SERIAL_RESPONSE_TAIL;

  return (BspUsart_Write(response,
                            SERIAL_TEST_FRAME_SIZE, SERIAL_TX_TIMEOUT_MS) == HAL_OK);
}

static uint8_t Serial_Motion_SendStatusResponse(AppMotionState *motion, const AppMotionIrq *irq)
{
  uint8_t response[SERIAL_TEST_FRAME_SIZE];

  response[SERIAL_FRAME_HEAD0_INDEX] = SERIAL_RESPONSE_HEAD;
  response[SERIAL_FRAME_HEAD1_INDEX] = SERIAL_RESPONSE_HEAD;
  response[SERIAL_FRAME_COMMAND_INDEX] = SERIAL_COMMAND_MOTION;
  response[SERIAL_FRAME_SUBCOMMAND_INDEX] = SERIAL_SUBCOMMAND_STATUS;
  response[SERIAL_FRAME_DATA0_INDEX] = motion->serial_motion_target_axis;
  response[SERIAL_FRAME_DATA1_INDEX] = motion->serial_motion_state;
  response[SERIAL_FRAME_DATA2_INDEX] = (uint8_t)((*irq->serial_motion_target_speed_hz) >> SERIAL_BYTE1_SHIFT);
  response[SERIAL_FRAME_DATA3_INDEX] = (uint8_t)(*irq->serial_motion_target_speed_hz);
  response[SERIAL_FRAME_DATA4_INDEX] = (uint8_t)((*irq->serial_motion_pulses_done) >> SERIAL_BYTE3_SHIFT);
  response[SERIAL_FRAME_DATA5_INDEX] = (uint8_t)((*irq->serial_motion_pulses_done) >> SERIAL_BYTE2_SHIFT);
  response[SERIAL_FRAME_DATA6_INDEX] = (uint8_t)((*irq->serial_motion_pulses_done) >> SERIAL_BYTE1_SHIFT);
  response[SERIAL_FRAME_DATA7_INDEX] = (uint8_t)(*irq->serial_motion_pulses_done);
  response[SERIAL_FRAME_TAIL0_INDEX] = SERIAL_RESPONSE_TAIL;
  response[SERIAL_FRAME_TAIL1_INDEX] = SERIAL_RESPONSE_TAIL;

  return (BspUsart_Write(response,
                            SERIAL_TEST_FRAME_SIZE, SERIAL_TX_TIMEOUT_MS) == HAL_OK);
}

static const AppMotionAxisConfig *Serial_Motion_FindAxis(
    const AppMotionAxisConfig *axes, uint8_t axis_count, uint8_t protocol_axis)
{
  uint8_t i;

  for (i = 0U; i < axis_count; i++)
  {
    if (axes[i].protocol_axis == protocol_axis)
    {
      return &axes[i];
    }
  }
  return 0;
}

static AppMotionAxisState *Serial_Motion_GetAxisState(
    AppMotionState *motion, const AppMotionAxisConfig *axis)
{
  return &motion->axis_state[axis->protocol_axis - SERIAL_MOTION_AXIS_X];
}

static uint8_t Serial_Motion_PrepareAndStart(
    AppMotionState *motion, const AppMotionIrq *irq,
    const AppMotionAxisConfig *axis)
{
  AppMotionAxisState *axis_state;
  const TMC5160_HandleTypeDef *dev;
  uint32_t expected_chopconf;
  uint16_t limit_mask;

  axis_state = Serial_Motion_GetAxisState(motion, axis);
  dev = &axis->tmc;

  limit_mask = BspGpio_ReadLimitActiveMask();
  if ((limit_mask & axis->limit_mask) != 0U)
  {
    motion->serial_motion_last_limit_mask = limit_mask & axis->limit_mask;
    return SERIAL_MOTION_ERROR_LIMIT;
  }

  BspTmc5160_WriteEnable(dev, 0U);
  axis_state->enable_active = 0U;
  axis_state->ioin = BspTmc5160_ReadRegister(dev, TMC5160_IOIN);
  axis_state->spi_ok =
      ((axis_state->ioin >> TMC5160_VERSION_SHIFT) == TMC5160_VERSION_VALUE);
  axis_state->spi_test_done = 1U;
  if (axis_state->spi_ok == 0U)
  {
    return SERIAL_MOTION_ERROR_TMC;
  }

  BspTmc5160_WriteRegister(dev, TMC5160_GCONF, TMC5160_GCONF_INIT);
  axis_state->gconf = BspTmc5160_ReadRegister(dev, TMC5160_GCONF);
  axis_state->gconf_ok = (axis_state->gconf == TMC5160_GCONF_INIT);
  axis_state->gconf_test_done = 1U;
  if (axis_state->gconf_ok == 0U)
  {
    return SERIAL_MOTION_ERROR_TMC;
  }

  axis_state->gstat = BspTmc5160_ReadRegister(dev, TMC5160_GSTAT);
  axis_state->gstat_test_done = 1U;
  BspTmc5160_WriteRegister(dev, TMC5160_GSTAT, TMC5160_GSTAT_INIT);
  /* 阻塞延时，建议后续改为状态机定时器替代 */
  HAL_Delay(SERIAL_MOTION_GSTAT_CLEAR_DELAY_MS);
  axis_state->gstat_after_clear =
      BspTmc5160_ReadRegister(dev, TMC5160_GSTAT);
  axis_state->gstat_uv_cp_clear_ok =
      ((axis_state->gstat_after_clear & TMC5160_GSTAT_UV_CP) == 0U);
  axis_state->gstat_clear_test_done = 1U;
  if (axis_state->gstat_uv_cp_clear_ok == 0U)
  {
    return SERIAL_MOTION_ERROR_TMC;
  }

  axis_state->chopconf =
      BspTmc5160_ReadRegister(dev, TMC5160_CHOPCONF);
  axis_state->static_read_test_done = 1U;
  BspTmc5160_WriteRegister(dev, TMC5160_IHOLD_IRUN,
                           TMC5160_IHOLD(axis->ihold) |
                           TMC5160_IRUN(axis->irun));
  expected_chopconf = (axis_state->chopconf & ~TMC5160_TOFF_MASK) |
                      TMC5160_TOFF(axis->toff);
  BspTmc5160_WriteRegister(dev, TMC5160_CHOPCONF, expected_chopconf);
  axis_state->chopconf_configured =
      BspTmc5160_ReadRegister(dev, TMC5160_CHOPCONF);
  axis_state->low_current_config_ok =
      (axis_state->chopconf_configured == expected_chopconf);
  axis_state->low_current_config_test_done = 1U;
  if (axis_state->low_current_config_ok == 0U)
  {
    return SERIAL_MOTION_ERROR_TMC;
  }

  limit_mask = BspGpio_ReadLimitActiveMask();
  if ((limit_mask & axis->limit_mask) != 0U)
  {
    motion->serial_motion_last_limit_mask = limit_mask & axis->limit_mask;
    return SERIAL_MOTION_ERROR_LIMIT;
  }
  BspTmc5160_WriteEnable(dev, 1U);
  axis_state->enable_active = 1U;
  /* 阻塞延时，建议后续改为状态机定时器替代 */
  HAL_Delay(SERIAL_MOTION_ENABLE_DELAY_MS);
  axis_state->gstat_enabled =
      BspTmc5160_ReadRegister(dev, TMC5160_GSTAT);
  axis_state->drv_status_enabled =
      BspTmc5160_ReadRegister(dev, TMC5160_DRV_STATUS);
  axis_state->enable_ok =
      ((axis_state->gstat_enabled &
        (TMC5160_GSTAT_DRV_ERR | TMC5160_GSTAT_UV_CP)) == 0U);
  axis_state->enable_test_done = 1U;
  if (axis_state->enable_ok == 0U)
  {
    BspTmc5160_WriteEnable(dev, 0U);
    axis_state->enable_active = 0U;
    return SERIAL_MOTION_ERROR_TMC;
  }

  limit_mask = BspGpio_ReadLimitActiveMask();
  if ((limit_mask & axis->limit_mask) != 0U)
  {
    motion->serial_motion_last_limit_mask = limit_mask & axis->limit_mask;
    BspTmc5160_WriteEnable(dev, 0U);
    axis_state->enable_active = 0U;
    return SERIAL_MOTION_ERROR_LIMIT;
  }

  BspGpio_Write(axis->direction_port, axis->direction_pin,
                (motion->serial_motion_target_direction != 0U) ?
                GPIO_PIN_SET : GPIO_PIN_RESET);
  BspGpio_Write(axis->step_port, axis->step_pin, GPIO_PIN_RESET);
  /* 阻塞延时，建议后续改为状态机定时器替代 */
  HAL_Delay(SERIAL_MOTION_DIRECTION_DELAY_MS);
  BspGpio_WriteStepMode(axis->step_port, axis->step_pin, GPIO_MODE_AF_PP);
  BspGpio_Write(axis->step_port, axis->step_pin, GPIO_PIN_RESET);
  motion->serial_motion_mscnt_before =
      BspTmc5160_ReadRegister(dev, TMC5160_MSCNT);
  motion->serial_motion_mscnt_ok = 0U;
  (*irq->serial_motion_current_speed_hz) = 0U;
  (*irq->serial_motion_peak_speed_hz) = 0U;
  (*irq->serial_motion_profile_phase) = 0U;
  (*irq->serial_motion_profile_accel_steps) = 0U;
  (*irq->serial_motion_profile_cruise_steps) = 0U;
  (*irq->serial_motion_profile_decel_steps) = 0U;
  (*irq->serial_motion_profile_update_count) = 0U;
  (*irq->serial_motion_last_period_ticks) = 0U;
  (*irq->serial_motion_profile_error) = SERIAL_MOTION_ERROR_NONE;
  (*irq->serial_motion_pulses_done) = 0U;
  (*irq->serial_motion_axis) = axis->protocol_axis;
  if (irq->write_initial_speed() == 0U)
  {
    BspTmc5160_WriteEnable(dev, 0U);
    axis_state->enable_active = 0U;
    return SERIAL_MOTION_ERROR_TIMER;
  }
  if (BspTim_WriteAxisSetupInterrupt(axis->protocol_axis) == 0U)
  {
    BspTmc5160_WriteEnable(dev, 0U);
    axis_state->enable_active = 0U;
    return SERIAL_MOTION_ERROR_TIMER;
  }
  (*irq->serial_motion_active) = 1U;
  if (BspTim_WriteAxisStart(axis->protocol_axis) != HAL_OK)
  {
    (*irq->serial_motion_active) = 0U;
    (void)BspTim_WriteAxisStop(axis->protocol_axis);
    BspTim_WriteAxisDisableUpdate(axis->protocol_axis);
    BspTmc5160_WriteEnable(dev, 0U);
    axis_state->enable_active = 0U;
    return SERIAL_MOTION_ERROR_TIMER;
  }
  motion->serial_motion_state = SERIAL_MOTION_STATE_ACTIVE;
  motion->serial_motion_start_ok = 1U;
  return SERIAL_MOTION_ERROR_NONE;
}

void AppMotion_Init(const AppMotionAxisConfig *axes, uint8_t axis_count)
{
  uint8_t i;

  for (i = 0U; i < axis_count; i++)
  {
    BspTmc5160_WriteEnable(&axes[i].tmc, 0U);
    BspGpio_Write(axes[i].direction_port, axes[i].direction_pin,
                  GPIO_PIN_RESET);
  }
}

void AppMotion_ProcessStart(AppMotionState *motion, const AppMotionIrq *irq,
                            AppProtocolState *protocol, const uint8_t *frame,
                            const AppMotionAxisConfig *axes, uint8_t axis_count)
{
  const AppMotionAxisConfig *axis;
  uint8_t i;
  uint8_t motion_frame_ok;
  uint8_t motion_continuous;
  uint8_t motion_error_code;
  uint16_t motion_speed_hz;
  uint32_t motion_distance_steps;
  uint8_t motion_response[SERIAL_TEST_FRAME_SIZE];

  (void)protocol;
  motion->serial_motion_command_count++;
  motion->serial_motion_last_axis = frame[SERIAL_FRAME_DATA0_INDEX];
  motion->serial_motion_last_direction = frame[SERIAL_FRAME_DATA1_INDEX];
  motion_speed_hz = (uint16_t)(((uint16_t)frame[SERIAL_FRAME_DATA2_INDEX] << SERIAL_BYTE1_SHIFT) |
                               frame[SERIAL_FRAME_DATA3_INDEX]);
  motion_continuous = (frame[SERIAL_FRAME_SUBCOMMAND_INDEX] == SERIAL_SUBCOMMAND_CONTINUOUS) &&
                      (SERIAL_MOTION_VALIDATION_COMMAND_ENABLED != 0U);
  if (motion_continuous != 0U)
  {
    motion_distance_steps = 0U;
  }
  else
  {
    motion_distance_steps = ((uint32_t)frame[SERIAL_FRAME_DATA4_INDEX] << SERIAL_BYTE3_SHIFT) |
                            ((uint32_t)frame[SERIAL_FRAME_DATA5_INDEX] << SERIAL_BYTE2_SHIFT) |
                            ((uint32_t)frame[SERIAL_FRAME_DATA6_INDEX] << SERIAL_BYTE1_SHIFT) |
                            (uint32_t)frame[SERIAL_FRAME_DATA7_INDEX];
  }
  motion->serial_motion_last_speed_hz = motion_speed_hz;
  motion->serial_motion_last_distance_steps = motion_distance_steps;
  axis = Serial_Motion_FindAxis(axes, axis_count,
                                motion->serial_motion_last_axis);

  motion_frame_ok =
      (frame[SERIAL_FRAME_TAIL0_INDEX] == SERIAL_REQUEST_TAIL) &&
      (frame[SERIAL_FRAME_TAIL1_INDEX] == SERIAL_REQUEST_TAIL);
  motion_error_code = SERIAL_MOTION_ERROR_NONE;
  if (motion_frame_ok == 0U)
  {
    motion_error_code = SERIAL_MOTION_ERROR_FRAME;
  }
  else if (axis == 0)
  {
    motion_error_code = SERIAL_MOTION_ERROR_AXIS;
  }
  else if (motion->serial_motion_last_direction > SERIAL_MOTION_DIRECTION_MAX)
  {
    motion_error_code = SERIAL_MOTION_ERROR_DIRECTION;
  }
  else if ((motion_speed_hz < SERIAL_MOTION_MIN_SPEED_HZ) ||
           (motion_speed_hz > SERIAL_MOTION_MAX_SPEED_HZ))
  {
    motion_error_code = SERIAL_MOTION_ERROR_SPEED;
  }
  else if ((motion_continuous == 0U) &&
           ((motion_distance_steps == 0U) ||
           (motion_distance_steps > SERIAL_MOTION_MAX_DISTANCE_STEPS))
          )
  {
    motion_error_code = SERIAL_MOTION_ERROR_DISTANCE;
  }
  else if ((motion_continuous == 0U) &&
           (motion_distance_steps >
           ((uint32_t)motion_speed_hz * SERIAL_MOTION_MAX_TIME_SECONDS))
          )
  {
    motion_error_code = SERIAL_MOTION_ERROR_TIME;
  }
  else if (motion->serial_motion_busy != 0U)
  {
    motion_error_code = SERIAL_MOTION_ERROR_BUSY;
  }

  motion->serial_motion_error_code = motion_error_code;
  motion->serial_motion_last_frame_ok = (motion_error_code ==
                                 SERIAL_MOTION_ERROR_NONE);
  if (motion->serial_motion_last_frame_ok != 0U)
  {
    motion->serial_motion_target_axis = motion->serial_motion_last_axis;
    motion->serial_motion_target_direction = motion->serial_motion_last_direction;
    (*irq->serial_motion_target_speed_hz) = motion_speed_hz;
    motion->serial_motion_target_distance_steps = motion_distance_steps;
    (*irq->serial_motion_target_steps) = (motion_continuous != 0U) ?
                                 0U : motion_distance_steps;
    (*irq->serial_motion_continuous) = motion_continuous;
    (*irq->serial_motion_pulses_done) = 0U;
    motion->serial_motion_command_pending = 1U;
    motion->serial_motion_busy = 1U;
    motion->serial_motion_state = SERIAL_MOTION_STATE_PENDING;
    motion->serial_motion_stop_pending = 0U;
    motion->serial_motion_limit_pending = 0U;
  }
  motion_response[SERIAL_FRAME_HEAD0_INDEX] = SERIAL_RESPONSE_HEAD;
  motion_response[SERIAL_FRAME_HEAD1_INDEX] = SERIAL_RESPONSE_HEAD;
  motion_response[SERIAL_FRAME_COMMAND_INDEX] = SERIAL_COMMAND_MOTION;
  motion_response[SERIAL_FRAME_SUBCOMMAND_INDEX] = (motion->serial_motion_last_frame_ok != 0U) ?
                       SERIAL_STATUS_ACCEPTED : SERIAL_STATUS_ERROR;
  if (motion->serial_motion_last_frame_ok != 0U)
  {
    for (i = 0U; i < SERIAL_DATA_SIZE; i++)
    {
      motion_response[SERIAL_DATA_FIRST + i] = frame[SERIAL_DATA_FIRST + i];
    }
  }
  else
  {
    motion_response[SERIAL_FRAME_DATA0_INDEX] = motion_error_code;
    for (i = SERIAL_DATA_SECOND; i < SERIAL_TAIL_FIRST; i++)
    {
      motion_response[i] = 0U;
    }
  }
  motion_response[SERIAL_FRAME_TAIL0_INDEX] = SERIAL_RESPONSE_TAIL;
  motion_response[SERIAL_FRAME_TAIL1_INDEX] = SERIAL_RESPONSE_TAIL;
  motion->serial_motion_last_response_ok =
      (BspUsart_Write(motion_response,
                         SERIAL_TEST_FRAME_SIZE, SERIAL_TX_TIMEOUT_MS) == HAL_OK);
}

void AppMotion_ProcessStop(AppMotionState *motion, const AppMotionIrq *irq,
                           AppProtocolState *protocol, const uint8_t *frame,
                           const AppMotionAxisConfig *axes, uint8_t axis_count)
{
  const AppMotionAxisConfig *axis;

  (void)protocol;
  motion->serial_motion_stop_command_count++;
  axis = Serial_Motion_FindAxis(axes, axis_count,
                                frame[SERIAL_FRAME_DATA0_INDEX]);
  if ((frame[SERIAL_FRAME_TAIL0_INDEX] != SERIAL_REQUEST_TAIL) ||
      (frame[SERIAL_FRAME_TAIL1_INDEX] != SERIAL_REQUEST_TAIL))
  {
    motion->serial_motion_error_code = SERIAL_MOTION_ERROR_FRAME;
    motion->serial_motion_last_response_ok =
        Serial_Motion_SendResponse(motion, irq, SERIAL_STATUS_ERROR, SERIAL_MOTION_ERROR_FRAME);
  }
  else if ((axis == 0) ||
           ((motion->serial_motion_busy != 0U) &&
            (axis->protocol_axis != motion->serial_motion_target_axis)))
  {
    motion->serial_motion_error_code = SERIAL_MOTION_ERROR_AXIS;
    motion->serial_motion_last_response_ok =
        Serial_Motion_SendResponse(motion, irq, SERIAL_STATUS_ERROR, SERIAL_MOTION_ERROR_AXIS);
  }
  else if (motion->serial_motion_busy == 0U)
  {
    motion->serial_motion_error_code = SERIAL_MOTION_ERROR_NOT_ACTIVE;
    motion->serial_motion_last_response_ok =
        Serial_Motion_SendResponse(motion, irq, SERIAL_STATUS_ERROR,
                                    SERIAL_MOTION_ERROR_NOT_ACTIVE);
  }
  else
  {
    motion->serial_motion_stop_pending = 1U;
    motion->serial_motion_last_response_ok = 0U;
  }
}

void AppMotion_ProcessStatus(AppMotionState *motion, const AppMotionIrq *irq, AppProtocolState *protocol,
                              const uint8_t *frame)
{
  (void)protocol;
  motion->serial_motion_status_query_count++;
  if ((frame[SERIAL_FRAME_TAIL0_INDEX] != SERIAL_REQUEST_TAIL) ||
      (frame[SERIAL_FRAME_TAIL1_INDEX] != SERIAL_REQUEST_TAIL))
  {
    motion->serial_motion_error_code = SERIAL_MOTION_ERROR_FRAME;
    motion->serial_motion_last_status_response_ok =
        Serial_Motion_SendResponse(motion, irq, SERIAL_STATUS_ERROR, SERIAL_MOTION_ERROR_FRAME);
  }
  else
  {
    motion->serial_motion_last_status_response_ok =
        Serial_Motion_SendStatusResponse(motion, irq);
  }
}

void AppMotion_Process(AppMotionState *motion, const AppMotionIrq *irq,
                       AppProtocolState *protocol,
                       const AppMotionAxisConfig *axes, uint8_t axis_count,
                       uint16_t limit_event_mask)
{
  AppMotionAxisState *axis_state;
  const AppMotionAxisConfig *axis;
  const TMC5160_HandleTypeDef *dev;
  uint8_t start_error_code;
  uint8_t stop_status;
  uint8_t stop_error_code;
  uint8_t stop_state;
  (void)protocol;
  axis = Serial_Motion_FindAxis(axes, axis_count,
                                motion->serial_motion_target_axis);
  axis_state = (axis != 0) ? Serial_Motion_GetAxisState(motion, axis) : 0;
  dev = (axis != 0) ? &axis->tmc : 0;
  if (axis != 0)
  {
    limit_event_mask &= axis->limit_mask;
  }
  else
  {
    limit_event_mask = 0U;
  }
  if (limit_event_mask != 0U)
  {
    motion->serial_motion_last_limit_mask = limit_event_mask;
    if (motion->serial_motion_busy != 0U)
    {
      motion->serial_motion_limit_pending = 1U;
      motion->serial_motion_stop_pending = 1U;
    }
  }

  if (motion->serial_motion_stop_pending != 0U)
  {
    if (motion->serial_motion_limit_pending != 0U)
    {
      stop_status = SERIAL_STATUS_ERROR;
      stop_error_code = SERIAL_MOTION_ERROR_LIMIT;
      stop_state = SERIAL_MOTION_STATE_LIMIT_STOPPED;
      motion->serial_motion_limit_stop_count++;
    }
    else
    {
      stop_status = SERIAL_STATUS_STOPPED;
      stop_error_code = SERIAL_MOTION_ERROR_NONE;
      stop_state = SERIAL_MOTION_STATE_STOPPED;
    }
    motion->serial_motion_stop_pending = 0U;
    motion->serial_motion_limit_pending = 0U;
    motion->serial_motion_command_pending = 0U;
    if ((*irq->serial_motion_active) != 0U)
    {
      (void)BspTim_WriteAxisStop(axis->protocol_axis);
      BspTim_WriteAxisDisableUpdate(axis->protocol_axis);
      (*irq->serial_motion_active) = 0U;
    }
    (*irq->serial_motion_done) = 0U;
    BspGpio_WriteStepMode(axis->step_port, axis->step_pin,
                          GPIO_MODE_OUTPUT_PP);
    BspGpio_Write(axis->step_port, axis->step_pin, GPIO_PIN_RESET);
    if (motion->serial_motion_start_ok != 0U)
    {
      motion->serial_motion_mscnt_after =
          BspTmc5160_ReadRegister(dev, TMC5160_MSCNT);
      motion->serial_motion_mscnt_delta =
          (uint16_t)((motion->serial_motion_mscnt_after -
                      motion->serial_motion_mscnt_before) & TMC5160_MSCNT_MASK);
    }
    axis_state->enable_active = 0U;
    motion->serial_motion_start_ok = 0U;
    (*irq->serial_motion_continuous) = 0U;
    motion->serial_motion_busy = 0U;
    motion->serial_motion_state = stop_state;
    motion->serial_motion_error_code = stop_error_code;
    motion->serial_motion_last_completion_status = stop_status;
    motion->serial_motion_last_completion_error_code = stop_error_code;
    motion->serial_motion_last_completion_response_ok =
        Serial_Motion_SendResponse(motion, irq, stop_status, stop_error_code);
    if (stop_status == SERIAL_STATUS_ERROR)
    {
      motion->serial_motion_last_limit_response_ok =
          motion->serial_motion_last_completion_response_ok;
    }
  }

  if ((motion->serial_motion_command_pending != 0U) &&
      ((*irq->serial_motion_active) == 0U))
  {
    motion->serial_motion_command_pending = 0U;
    motion->serial_motion_start_ok = 0U;
    start_error_code = Serial_Motion_PrepareAndStart(motion, irq, axis);
    if (start_error_code != SERIAL_MOTION_ERROR_NONE)
    {
      motion->serial_motion_error_code = start_error_code;
      motion->serial_motion_busy = 0U;
      motion->serial_motion_state = SERIAL_MOTION_STATE_ERROR;
      motion->serial_motion_last_completion_status = SERIAL_STATUS_ERROR;
      motion->serial_motion_last_completion_error_code = start_error_code;
      motion->serial_motion_last_completion_response_ok =
          Serial_Motion_SendResponse(motion, irq, SERIAL_STATUS_ERROR, start_error_code);
    }
  }

  if ((*irq->serial_motion_done) != 0U)
  {
    (*irq->serial_motion_done) = 0U;
    BspGpio_WriteStepMode(axis->step_port, axis->step_pin,
                          GPIO_MODE_OUTPUT_PP);
    BspGpio_Write(axis->step_port, axis->step_pin, GPIO_PIN_RESET);
    motion->serial_motion_mscnt_after =
        BspTmc5160_ReadRegister(dev, TMC5160_MSCNT);
    motion->serial_motion_mscnt_delta =
        (uint16_t)((motion->serial_motion_mscnt_after -
                    motion->serial_motion_mscnt_before) & TMC5160_MSCNT_MASK);
    motion->serial_motion_mscnt_ok =
        ((motion->serial_motion_mscnt_delta ==
          ((*irq->serial_motion_target_steps) & TMC5160_MSCNT_MASK)) ||
         (motion->serial_motion_mscnt_delta ==
          ((TMC5160_MSCNT_MODULUS - ((*irq->serial_motion_target_steps) & TMC5160_MSCNT_MASK)) &
           TMC5160_MSCNT_MASK)));
    axis_state->enable_active = 0U;
    motion->serial_motion_start_ok = 0U;
    (*irq->serial_motion_continuous) = 0U;
    motion->serial_motion_limit_pending = 0U;
    motion->serial_motion_busy = 0U;
    if ((*irq->serial_motion_profile_error) != SERIAL_MOTION_ERROR_NONE)
    {
      motion->serial_motion_state = SERIAL_MOTION_STATE_ERROR;
      motion->serial_motion_error_code = (*irq->serial_motion_profile_error);
      motion->serial_motion_last_completion_status = SERIAL_STATUS_ERROR;
      motion->serial_motion_last_completion_error_code = (*irq->serial_motion_profile_error);
      motion->serial_motion_last_completion_response_ok =
          Serial_Motion_SendResponse(motion, irq, SERIAL_STATUS_ERROR, (*irq->serial_motion_profile_error));
    }
    else
    {
      motion->serial_motion_state = SERIAL_MOTION_STATE_DONE;
      motion->serial_motion_error_code = SERIAL_MOTION_ERROR_NONE;
      motion->serial_motion_last_completion_status = SERIAL_STATUS_DONE;
      motion->serial_motion_last_completion_error_code = SERIAL_MOTION_ERROR_NONE;
      motion->serial_motion_last_completion_response_ok =
          Serial_Motion_SendResponse(motion, irq, SERIAL_STATUS_DONE, SERIAL_MOTION_ERROR_NONE);
    }
  }
}
