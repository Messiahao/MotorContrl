#include "app_scheduler.h"
#include "app_protocol.h"
#include "app_motion.h"
#include "app_aux_output.h"
#include "app_limit.h"
#include "app_led.h"
#include "app_light.h"
#include "gpio.h"
#include "usart.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"

typedef struct
{
  AppProtocolState protocol;
  AppMotionState motion;
  AppAuxState aux;
  AppLimitState limits;
  const AppMotionIrq *irq;
} AppRuntime;

/* Owned here. Other modules receive explicit pointers, never extern objects. */
static AppRuntime runtime;
static const AppMotionAxisConfig motion_axes[SERIAL_MOTION_AXIS_COUNT] = {
  {
    .protocol_axis = SERIAL_MOTION_AXIS_X,
    .tmc = {X_CS_GPIO_Port, X_CS_Pin, X_EN_GPIO_Port, X_EN_Pin},
    .direction_port = X_DIR_GPIO_Port,
    .direction_pin = X_DIR_Pin,
    .step_port = X_STEP_GPIO_Port,
    .step_pin = X_STEP_Pin,
    .limit_mask = X_LIMIT_ACTIVE_MASK,
    .ihold = X_MOTION_IHOLD,
    .irun = X_MOTION_IRUN,
    .toff = X_MOTION_TOFF
  },
  {
    .protocol_axis = SERIAL_MOTION_AXIS_Y,
    .tmc = {Y_CS_GPIO_Port, Y_CS_Pin, Y_EN_GPIO_Port, Y_EN_Pin},
    .direction_port = Y_DIR_GPIO_Port,
    .direction_pin = Y_DIR_Pin,
    .step_port = Y_STEP_GPIO_Port,
    .step_pin = Y_STEP_Pin,
    .limit_mask = Y_LIMIT_ACTIVE_MASK,
    .ihold = Y_MOTION_IHOLD,
    .irun = Y_MOTION_IRUN,
    .toff = Y_MOTION_TOFF
  },
  {
    .protocol_axis = SERIAL_MOTION_AXIS_Z,
    .tmc = {Z_CS_GPIO_Port, Z_CS_Pin, Z_EN_GPIO_Port, Z_EN_Pin},
    .direction_port = Z_DIR_GPIO_Port,
    .direction_pin = Z_DIR_Pin,
    .step_port = Z_STEP_GPIO_Port,
    .step_pin = Z_STEP_Pin,
    .limit_mask = Z_LIMIT_CHECK_MASK,
    .ihold = Z_MOTION_IHOLD,
    .irun = Z_MOTION_IRUN,
    .toff = Z_MOTION_TOFF
  }
};

static void Scheduler_ProcessFrame(void *context, const uint8_t *frame)
{
  AppRuntime *app = context;
  uint8_t command = frame[SERIAL_FRAME_COMMAND_INDEX];
  uint8_t subcommand = frame[SERIAL_FRAME_SUBCOMMAND_INDEX];

  if (command == SERIAL_COMMAND_MOTION)
  {
    if ((subcommand == SERIAL_SUBCOMMAND_DEFAULT) ||
        (subcommand == SERIAL_SUBCOMMAND_CONTINUOUS))
    {
      AppMotion_ProcessStart(&app->motion, app->irq, &app->protocol, frame,
                             motion_axes, SERIAL_MOTION_AXIS_COUNT);
    }
    else if (subcommand == SERIAL_SUBCOMMAND_STOP)
    {
      AppMotion_ProcessStop(&app->motion, app->irq, &app->protocol, frame,
                            motion_axes, SERIAL_MOTION_AXIS_COUNT);
    }
    else if (subcommand == SERIAL_SUBCOMMAND_STATUS)
    {
      AppMotion_ProcessStatus(&app->motion, app->irq, &app->protocol, frame);
    }
  }
  else if (((command == SERIAL_COMMAND_RELAY) ||
            (command == SERIAL_COMMAND_BRAKE)) &&
           (subcommand == SERIAL_SUBCOMMAND_DEFAULT))
  {
    AppAuxOutput_Process(&app->aux, &app->protocol, frame);
  }
  else if ((command == SERIAL_COMMAND_LIGHT) &&
           (subcommand == SERIAL_SUBCOMMAND_DEFAULT))
  {
    AppLight_Process(&app->protocol, frame);
  }
}

void AppScheduler_Init(const AppMotionIrq *irq)
{
  runtime.irq = irq;
  /* Initialize the active host UART before other peripherals. */
  BspGpio_Init();
  BspUsart2_Init();
  BspI2c_Init();
  BspSpi_Init();
  BspTim2_Init();
  BspTim3_Init();
  BspTim4_Init();
  AppLed_Init();
  AppMotion_Init(motion_axes, SERIAL_MOTION_AXIS_COUNT);
  AppLimit_Init(&runtime.limits);
  AppProtocol_Init(&runtime.protocol);
  /* Do not call BspTmc5160_Init: baseline never called the enabling sequence. */
}

void AppScheduler_Process(void)
{
  /* Fixed polling order. No new tick, delay, interrupt mask, or task queue. */
  AppLed_Process();
  AppProtocol_Process(&runtime.protocol, Scheduler_ProcessFrame, &runtime);
  AppLimit_Process(&runtime.limits);
  AppMotion_Process(&runtime.motion, runtime.irq, &runtime.protocol,
                    motion_axes, SERIAL_MOTION_AXIS_COUNT,
                    AppLimit_ConsumeInterruptMask());
}

void AppScheduler_Run(void)
{
  while (1)
  {
    AppScheduler_Process();
  }
}
