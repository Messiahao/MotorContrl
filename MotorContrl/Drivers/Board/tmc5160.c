#include "tmc5160.h"
#include "spi.h"

static void Tmc5160_WriteChipSelect(const TMC5160_HandleTypeDef *dev, GPIO_PinState level)
{
  HAL_GPIO_WritePin(dev->CS_GPIO_Port, dev->CS_Pin, level);
}

void BspTmc5160_WriteEnable(const TMC5160_HandleTypeDef *dev, uint8_t enabled)
{
  HAL_GPIO_WritePin(dev->EN_GPIO_Port, dev->EN_Pin,
                    (enabled != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

/*
==================================================
SPI write

40bit frame:
Byte0:
Address + write bit

Byte1~4:
DATA[31:0]

==================================================
*/

void BspTmc5160_WriteRegister(const TMC5160_HandleTypeDef *dev, uint8_t address, uint32_t data)
{

uint8_t tx[TMC5160_FRAME_SIZE];

tx[0]=address | TMC5160_WRITE_BIT;
tx[1]=(uint8_t)(data>>TMC5160_BYTE3_SHIFT);
tx[2]=(uint8_t)(data>>TMC5160_BYTE2_SHIFT);
tx[3]=(uint8_t)(data>>TMC5160_BYTE1_SHIFT);
tx[4]=(uint8_t)data;

Tmc5160_WriteChipSelect(dev, GPIO_PIN_RESET);

BspSpi_Write(tx, TMC5160_FRAME_SIZE, HAL_MAX_DELAY);

Tmc5160_WriteChipSelect(dev, GPIO_PIN_SET);

}

/*
==================================================
SPI read

TMC5160 pipeline read

First frame:
send address

Second frame:
receive data

==================================================
*/

uint32_t BspTmc5160_ReadRegister(const TMC5160_HandleTypeDef *dev, uint8_t address)
{

uint8_t tx[TMC5160_FRAME_SIZE];
uint8_t rx[TMC5160_FRAME_SIZE];
uint32_t value;

/*
First request
*/

tx[0]=address;
tx[1]=0;
tx[2]=0;
tx[3]=0;
tx[4]=0;

Tmc5160_WriteChipSelect(dev, GPIO_PIN_RESET);

BspSpi_ReadWrite(tx, rx, TMC5160_FRAME_SIZE, HAL_MAX_DELAY);

Tmc5160_WriteChipSelect(dev, GPIO_PIN_SET);

/*
Second dummy transfer

get requested register
*/

tx[0]=0;
tx[1]=0;
tx[2]=0;
tx[3]=0;
tx[4]=0;

Tmc5160_WriteChipSelect(dev, GPIO_PIN_RESET);

BspSpi_ReadWrite(tx, rx, TMC5160_FRAME_SIZE, HAL_MAX_DELAY);

Tmc5160_WriteChipSelect(dev, GPIO_PIN_SET);

value =
((uint32_t)rx[1]<<TMC5160_BYTE3_SHIFT) |
((uint32_t)rx[2]<<TMC5160_BYTE2_SHIFT) |
((uint32_t)rx[3]<<TMC5160_BYTE1_SHIFT) |
((uint32_t)rx[4]);

return value;

}

/*
==================================================
Initialization

STEP/DIR mode
==================================================
*/

void BspTmc5160_Init(const TMC5160_HandleTypeDef *dev, uint32_t current)
{

/*
Keep driver disabled
*/

BspTmc5160_WriteEnable(dev, 0U);

/* 阻塞延时，建议后续改为状态机定时器替代 */
HAL_Delay(TMC5160_INIT_DELAY_MS);

/*
Clear error flag
*/

BspTmc5160_WriteRegister(dev, TMC5160_GSTAT, TMC5160_GSTAT_INIT);

/*
General configuration
*/

BspTmc5160_WriteRegister(dev,TMC5160_GCONF,TMC5160_GCONF_INIT);

/*
Current scaling

RSENSE=25mR

0 means 256
*/

BspTmc5160_WriteRegister(dev, TMC5160_GLOBALSCALER, TMC5160_GLOBALSCALER_INIT);

/*
Motor current
*/

BspTmc5160_WriteRegister(dev, TMC5160_IHOLD_IRUN, current);

/*
Standstill power down delay
*/

BspTmc5160_WriteRegister(dev, TMC5160_TPOWERDOWN, TMC5160_TPOWERDOWN_INIT);

/*
Chopper

1/64 microstep
*/

BspTmc5160_WriteRegister(dev, TMC5160_CHOPCONF, TMC5160_CHOPCONF_INIT);

/*
Disable CoolStep
*/

BspTmc5160_WriteRegister(dev, TMC5160_COOLCONF, TMC5160_COOLCONF_INIT);

/*
Enable driver

ENN LOW

*/

BspTmc5160_WriteEnable(dev, 1U);

}
