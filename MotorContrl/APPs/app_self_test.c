#include "app_self_test.h"
#include "gpio.h"
#include "tim.h"

void AppSelfTest_Init(AppTmcState *tmc)
{
  tmc->x_tmc5160_spi_test_tick = HAL_GetTick();
  tmc->x_tmc5160_spi_test_pending = 1U;
}

void AppSelfTest_Process(AppTmcState *tmc, const TMC5160_HandleTypeDef *dev)
{
#if !LIMIT_GPIO_STATIC_TEST && !SERIAL_PROTOCOL_STAGE1_TEST
    if ((tmc->x_tmc5160_spi_test_pending != 0U) &&
        ((HAL_GetTick() - tmc->x_tmc5160_spi_test_tick) >= X_SELF_TEST_SPI_DELAY_MS))
    {
      tmc->x_tmc5160_ioin = BspTmc5160_ReadRegister(dev, TMC5160_IOIN);
      tmc->x_tmc5160_spi_ok = ((tmc->x_tmc5160_ioin >> TMC5160_VERSION_SHIFT) == TMC5160_VERSION_VALUE);
      tmc->x_tmc5160_spi_test_done = 1U;
      tmc->x_tmc5160_spi_test_pending = 0U;

      if (tmc->x_tmc5160_spi_ok != 0U)
      {
        BspTmc5160_WriteRegister(dev, TMC5160_GCONF,
                              TMC5160_GCONF_INIT);
        tmc->x_tmc5160_gconf = BspTmc5160_ReadRegister(dev, TMC5160_GCONF);
        tmc->x_tmc5160_gconf_ok = (tmc->x_tmc5160_gconf == TMC5160_GCONF_INIT);
        tmc->x_tmc5160_gconf_test_done = 1U;

        if (tmc->x_tmc5160_gconf_ok != 0U)
        {
          tmc->x_tmc5160_gstat = BspTmc5160_ReadRegister(dev, TMC5160_GSTAT);
          tmc->x_tmc5160_gstat_test_done = 1U;
          BspTmc5160_WriteRegister(dev, TMC5160_GSTAT,
                                TMC5160_GSTAT_INIT);
          tmc->x_tmc5160_gstat_clear_tick = HAL_GetTick();
          tmc->x_tmc5160_gstat_clear_pending = 1U;
        }
      }
    }

    if ((tmc->x_tmc5160_gstat_clear_pending != 0U) &&
        ((HAL_GetTick() - tmc->x_tmc5160_gstat_clear_tick) >= X_SELF_TEST_CLEAR_DELAY_MS))
    {
      tmc->x_tmc5160_gstat_after_clear =
          BspTmc5160_ReadRegister(dev, TMC5160_GSTAT);
      tmc->x_tmc5160_gstat_uv_cp_clear_ok =
          ((tmc->x_tmc5160_gstat_after_clear & TMC5160_GSTAT_UV_CP) == 0U);
      tmc->x_tmc5160_gstat_clear_test_done = 1U;
      tmc->x_tmc5160_gstat_clear_pending = 0U;
    }

    if ((tmc->x_tmc5160_gstat_clear_test_done != 0U) &&
        (tmc->x_tmc5160_static_read_test_done == 0U))
    {
      tmc->x_tmc5160_chopconf =
          BspTmc5160_ReadRegister(dev, TMC5160_CHOPCONF);
      tmc->x_tmc5160_static_read_test_done = 1U;
    }

    if ((tmc->x_tmc5160_static_read_test_done != 0U) &&
        (tmc->x_tmc5160_low_current_config_test_done == 0U))
    {
      BspTmc5160_WriteRegister(dev, TMC5160_IHOLD_IRUN,
                            TMC5160_IHOLD(X_MOTION_IHOLD) |
                            TMC5160_IRUN(X_MOTION_IRUN));
      BspTmc5160_WriteRegister(dev, TMC5160_CHOPCONF,
                            (tmc->x_tmc5160_chopconf & ~TMC5160_TOFF_MASK) |
                            TMC5160_TOFF(X_MOTION_TOFF));
      tmc->x_tmc5160_chopconf_configured =
          BspTmc5160_ReadRegister(dev, TMC5160_CHOPCONF);
      tmc->x_tmc5160_low_current_config_ok =
          (tmc->x_tmc5160_chopconf_configured ==
           ((tmc->x_tmc5160_chopconf & ~TMC5160_TOFF_MASK) |
            TMC5160_TOFF(X_MOTION_TOFF)));
      tmc->x_tmc5160_low_current_config_test_done = 1U;
    }

    if ((tmc->x_tmc5160_low_current_config_ok != 0U) &&
        (tmc->x_tmc5160_enable_test_done == 0U) &&
        (tmc->x_tmc5160_enable_test_pending == 0U))
    {
      BspTmc5160_WriteEnable(dev, 1U);
      tmc->x_tmc5160_enable_test_tick = HAL_GetTick();
      tmc->x_tmc5160_enable_test_pending = 1U;
      tmc->x_tmc5160_enable_test_active = 1U;
    }

    if ((tmc->x_tmc5160_enable_test_pending != 0U) &&
        ((HAL_GetTick() - tmc->x_tmc5160_enable_test_tick) >= X_SELF_TEST_ENABLE_DELAY_MS))
    {
      tmc->x_tmc5160_gstat_enabled =
          BspTmc5160_ReadRegister(dev, TMC5160_GSTAT);
      tmc->x_tmc5160_drv_status_enabled =
          BspTmc5160_ReadRegister(dev, TMC5160_DRV_STATUS);
      tmc->x_tmc5160_enable_test_ok =
          ((tmc->x_tmc5160_gstat_enabled &
            (TMC5160_GSTAT_DRV_ERR | TMC5160_GSTAT_UV_CP)) == 0U);
      if (tmc->x_tmc5160_enable_test_ok == 0U)
      {
        BspTmc5160_WriteEnable(dev, 0U);
        tmc->x_tmc5160_enable_test_active = 0U;
      }
      tmc->x_tmc5160_enable_test_done = 1U;
      tmc->x_tmc5160_enable_test_pending = 0U;
    }

    if ((tmc->x_tmc5160_enable_test_active != 0U) &&
        (tmc->x_tmc5160_enable_test_ok != 0U) &&
        (tmc->x_tmc5160_step_test_done == 0U))
    {
      if (tmc->x_tmc5160_step_test_state == X_SELF_TEST_PHASE_IDLE)
      {


        BspGpio_WriteXStepMode(GPIO_MODE_OUTPUT_PP);
        BspGpio_Write(X_DIR_GPIO_Port, X_DIR_Pin, GPIO_PIN_RESET);
        BspGpio_Write(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
        tmc->x_tmc5160_step_test_tick = HAL_GetTick();
        tmc->x_tmc5160_step_test_state = X_SELF_TEST_PHASE_WAIT_RISE;
      }
      else if ((tmc->x_tmc5160_step_test_state == X_SELF_TEST_PHASE_WAIT_RISE) &&
               ((HAL_GetTick() - tmc->x_tmc5160_step_test_tick) >= X_SELF_TEST_STEP_DELAY_MS))
      {
        tmc->x_tmc5160_mscnt_before_step =
            BspTmc5160_ReadRegister(dev, TMC5160_MSCNT);
        BspGpio_Write(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_SET);
        tmc->x_tmc5160_step_test_tick = HAL_GetTick();
        tmc->x_tmc5160_step_test_state = X_SELF_TEST_PHASE_HIGH;
      }
      else if ((tmc->x_tmc5160_step_test_state == X_SELF_TEST_PHASE_HIGH) &&
               ((HAL_GetTick() - tmc->x_tmc5160_step_test_tick) >= X_SELF_TEST_STEP_DELAY_MS))
      {
        BspGpio_Write(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
        tmc->x_tmc5160_step_test_tick = HAL_GetTick();
        tmc->x_tmc5160_step_test_state = X_SELF_TEST_PHASE_LOW;
      }
      else if ((tmc->x_tmc5160_step_test_state == X_SELF_TEST_PHASE_LOW) &&
               ((HAL_GetTick() - tmc->x_tmc5160_step_test_tick) >= X_SELF_TEST_STEP_DELAY_MS))
      {


        tmc->x_tmc5160_mscnt_after_step =
            BspTmc5160_ReadRegister(dev, TMC5160_MSCNT);
        tmc->x_tmc5160_mscnt_step_delta =
            (uint16_t)((tmc->x_tmc5160_mscnt_after_step -
                        tmc->x_tmc5160_mscnt_before_step) & TMC5160_MSCNT_MASK);
        tmc->x_tmc5160_step_test_ok =
            ((tmc->x_tmc5160_mscnt_step_delta == 1U) ||
             (tmc->x_tmc5160_mscnt_step_delta == TMC5160_MSCNT_MASK));
        tmc->x_tmc5160_step_test_done = 1U;

        BspGpio_WriteXStepMode(GPIO_MODE_AF_PP);
      }
    }

    if ((tmc->x_tmc5160_enable_test_active != 0U) &&
        (tmc->x_tmc5160_step_test_ok != 0U) &&
        (tmc->x_tmc5160_multi_step_test_done == 0U))
    {
      if (tmc->x_tmc5160_multi_step_test_state == X_SELF_TEST_PHASE_IDLE)
      {


        BspGpio_WriteXStepMode(GPIO_MODE_OUTPUT_PP);
        BspGpio_Write(X_DIR_GPIO_Port, X_DIR_Pin, GPIO_PIN_RESET);
        BspGpio_Write(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
        tmc->x_tmc5160_mscnt_before_multi_step =
            BspTmc5160_ReadRegister(dev, TMC5160_MSCNT);
        tmc->x_tmc5160_multi_step_test_tick = HAL_GetTick();
        tmc->x_tmc5160_multi_step_test_state = X_SELF_TEST_PHASE_WAIT_RISE;
      }
      else if ((tmc->x_tmc5160_multi_step_test_state == X_SELF_TEST_PHASE_WAIT_RISE) &&
               ((HAL_GetTick() - tmc->x_tmc5160_multi_step_test_tick) >= X_SELF_TEST_STEP_DELAY_MS))
      {
        BspGpio_Write(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_SET);
        tmc->x_tmc5160_multi_step_test_tick = HAL_GetTick();
        tmc->x_tmc5160_multi_step_test_state = X_SELF_TEST_PHASE_HIGH;
      }
      else if ((tmc->x_tmc5160_multi_step_test_state == X_SELF_TEST_PHASE_HIGH) &&
               ((HAL_GetTick() - tmc->x_tmc5160_multi_step_test_tick) >= X_SELF_TEST_STEP_DELAY_MS))
      {
        BspGpio_Write(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
        tmc->x_tmc5160_multi_step_test_tick = HAL_GetTick();
        tmc->x_tmc5160_multi_step_test_state = X_SELF_TEST_PHASE_LOW;
      }
      else if ((tmc->x_tmc5160_multi_step_test_state == X_SELF_TEST_PHASE_LOW) &&
               ((HAL_GetTick() - tmc->x_tmc5160_multi_step_test_tick) >= X_SELF_TEST_STEP_GAP_MS))
      {
        tmc->x_tmc5160_multi_step_test_count++;
        if (tmc->x_tmc5160_multi_step_test_count >= X_SELF_TEST_STEP_COUNT)
        {


          tmc->x_tmc5160_mscnt_after_multi_step =
              BspTmc5160_ReadRegister(dev, TMC5160_MSCNT);
          tmc->x_tmc5160_mscnt_multi_step_delta =
              (uint16_t)((tmc->x_tmc5160_mscnt_after_multi_step -
                          tmc->x_tmc5160_mscnt_before_multi_step) & TMC5160_MSCNT_MASK);
          tmc->x_tmc5160_multi_step_test_ok =
              ((tmc->x_tmc5160_mscnt_multi_step_delta == X_SELF_TEST_STEP_COUNT) ||
               (tmc->x_tmc5160_mscnt_multi_step_delta == X_SELF_TEST_REVERSE_DELTA));
          tmc->x_tmc5160_multi_step_test_done = 1U;

          BspGpio_WriteXStepMode(GPIO_MODE_AF_PP);
        }
        else
        {
          tmc->x_tmc5160_multi_step_test_state = X_SELF_TEST_PHASE_WAIT_RISE;
        }
      }
    }

    if ((tmc->x_tmc5160_enable_test_active != 0U) &&
        (tmc->x_tmc5160_multi_step_test_ok != 0U) &&
        (tmc->x_tmc5160_motion_test_done == 0U))
    {
      if (tmc->x_tmc5160_motion_test_state == X_SELF_TEST_MOTION_IDLE)
      {
        BspGpio_Write(X_DIR_GPIO_Port, X_DIR_Pin, GPIO_PIN_RESET);
        BspGpio_Write(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
        tmc->x_tmc5160_motion_test_start_tick = HAL_GetTick();
        if (BspTim_WriteXStart() == HAL_OK)
        {
          tmc->x_tmc5160_motion_test_active = 1U;
          tmc->x_tmc5160_motion_test_state = X_SELF_TEST_MOTION_RUNNING;
        }
        else
        {
          tmc->x_tmc5160_motion_test_done = 1U;
        }
      }
      else if ((HAL_GetTick() - tmc->x_tmc5160_motion_test_start_tick) >=
               X_MOTION_TEST_DURATION_MS)
      {


        BspTim_WriteXStop();
        BspGpio_WriteXStepMode(GPIO_MODE_OUTPUT_PP);
        BspGpio_Write(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
        tmc->x_tmc5160_motion_test_active = 0U;
        tmc->x_tmc5160_motion_test_done = 1U;
      }
    }

    if ((tmc->x_tmc5160_enable_test_active != 0U) &&
        ((HAL_GetTick() - tmc->x_tmc5160_enable_test_tick) >=
         X_MOTION_TEST_DISABLE_DELAY_MS))
    {
      BspTmc5160_WriteEnable(dev, 0U);
      tmc->x_tmc5160_enable_test_active = 0U;
    }
#else
  (void)tmc;
  (void)dev;
#endif
    /*
    if ((auxiliary_output_test_state == 0U) &&
        ((HAL_GetTick() - auxiliary_output_test_tick) >= 5000U))
    {
      HAL_GPIO_WritePin(GPIOB, VOUT_5_Pin | VOUT_24_Pin, GPIO_PIN_SET);
      auxiliary_output_test_tick = HAL_GetTick();
      auxiliary_output_test_state = 1U;
    }
    else if ((auxiliary_output_test_state == 1U) &&
             ((HAL_GetTick() - auxiliary_output_test_tick) >= 5000U))
    {
      HAL_GPIO_WritePin(GPIOB, VOUT_5_Pin | VOUT_24_Pin, GPIO_PIN_RESET);
      auxiliary_output_test_state = 2U;
    }
    */
}
