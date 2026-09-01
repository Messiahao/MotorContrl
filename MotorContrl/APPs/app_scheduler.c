#include "app_scheduler.h"
#include "app_protocol.h"
#include "app_motion.h"
#include "app_aux_output.h"
#include "app_limit.h"
#include "app_self_test.h"
#include "app_led.h"
#include "app_light.h"
#include "gpio.h"
#include "usart.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "mcp4728.h"

typedef struct
{
  AppProtocolState protocol;
  AppMotionState motion;
  AppAuxState aux;
  AppLimitState limits;
  AppTmcState tmc;
  const AppMotionIrq *irq;
} AppRuntime;

/* Owned here. Other modules receive explicit pointers, never extern objects. */
static AppRuntime runtime;
static const TMC5160_HandleTypeDef x_tmc5160 = {
  X_CS_GPIO_Port, X_CS_Pin, X_EN_GPIO_Port, X_EN_Pin
};

static void Scheduler_ProcessFrame(void *context, const uint8_t *frame)
{
  AppRuntime *app = context;

  if (((frame[SERIAL_FRAME_COMMAND_INDEX] == SERIAL_COMMAND_MOTION) &&
       (frame[SERIAL_FRAME_SUBCOMMAND_INDEX] == SERIAL_SUBCOMMAND_DEFAULT)) ||
      ((frame[SERIAL_FRAME_COMMAND_INDEX] == SERIAL_COMMAND_MOTION) &&
       (frame[SERIAL_FRAME_SUBCOMMAND_INDEX] == SERIAL_SUBCOMMAND_CONTINUOUS)))
  {
    AppMotion_ProcessStart(&app->motion, app->irq, &app->protocol, frame);
  }
  else if ((frame[SERIAL_FRAME_COMMAND_INDEX] == SERIAL_COMMAND_MOTION) &&
           (frame[SERIAL_FRAME_SUBCOMMAND_INDEX] == SERIAL_SUBCOMMAND_STOP))
  {
    AppMotion_ProcessStop(&app->motion, app->irq, &app->protocol, frame);
  }
  else if ((frame[SERIAL_FRAME_COMMAND_INDEX] == SERIAL_COMMAND_MOTION) &&
           (frame[SERIAL_FRAME_SUBCOMMAND_INDEX] == SERIAL_SUBCOMMAND_STATUS))
  {
    AppMotion_ProcessStatus(&app->motion, app->irq, &app->protocol, frame);
  }
  else if (((frame[SERIAL_FRAME_COMMAND_INDEX] == SERIAL_COMMAND_RELAY) &&
            (frame[SERIAL_FRAME_SUBCOMMAND_INDEX] == SERIAL_SUBCOMMAND_DEFAULT)) ||
           ((frame[SERIAL_FRAME_COMMAND_INDEX] == SERIAL_COMMAND_BRAKE) &&
            (frame[SERIAL_FRAME_SUBCOMMAND_INDEX] == SERIAL_SUBCOMMAND_DEFAULT)))
  {
    AppAuxOutput_Process(&app->aux, &app->protocol, frame);
  }
  else if ((frame[SERIAL_FRAME_COMMAND_INDEX] == SERIAL_COMMAND_LIGHT) &&
           (frame[SERIAL_FRAME_SUBCOMMAND_INDEX] == SERIAL_SUBCOMMAND_DEFAULT))
  {
    AppLight_Process(&app->protocol, frame);
  }
  else
  {
    app->protocol.serial_test_last_frame_ok = 0U;
    app->protocol.serial_test_frame_error_count++;
    app->protocol.serial_test_last_response_ok = 0U;
  }
}

void AppScheduler_Init(const AppMotionIrq *irq)
{
  runtime.irq = irq;
  /* Do not group UART initializations: this is the original startup order. */
  BspGpio_Init();
  BspUsart2_Init();
  BspI2c_Init();
  BspSpi_Init();
  BspTim2_Init();
  BspTim3_Init();
  BspTim4_Init();
  BspUsart3_Init();
  AppLed_Init();
  AppMotion_Init();
  AppLimit_Init(&runtime.limits);
  AppSelfTest_Init(&runtime.tmc);
  AppProtocol_Init(&runtime.protocol);
  AppAuxOutput_Init();
  BspMcp4728_Init();
  AppLight_Init();
  /* Do not call BspTmc5160_Init: baseline never called the enabling sequence. */
}

void AppScheduler_Process(void)
{
  /* Fixed polling order. No new tick, delay, interrupt mask, or task queue. */
  AppLed_Process();
  AppProtocol_Process(&runtime.protocol, Scheduler_ProcessFrame, &runtime);
  AppLimit_Process(&runtime.limits);
  AppMotion_Process(&runtime.motion, runtime.irq, &runtime.protocol,
                    &runtime.tmc, &x_tmc5160);
#if !LIMIT_GPIO_STATIC_TEST && !SERIAL_PROTOCOL_STAGE1_TEST
  AppSelfTest_Process(&runtime.tmc, &x_tmc5160);
#endif
}

void AppScheduler_Run(void)
{
  while (1)
  {
    AppScheduler_Process();
  }
}
