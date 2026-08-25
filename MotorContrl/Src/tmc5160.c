#include "tmc5160.h"

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

void TMC5160_WriteRegister(const TMC5160_HandleTypeDef *dev, uint8_t address, uint32_t data)
{

uint8_t tx[5];

tx[0]=address | 0x80;
tx[1]=(uint8_t)(data>>24);
tx[2]=(uint8_t)(data>>16);
tx[3]=(uint8_t)(data>>8);
tx[4]=(uint8_t)data;

TMC5160_CS_LOW(dev);

HAL_SPI_Transmit(&hspi1, tx, 5, HAL_MAX_DELAY);

TMC5160_CS_HIGH(dev);

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

uint32_t TMC5160_ReadRegister(const TMC5160_HandleTypeDef *dev, uint8_t address)
{

uint8_t tx[5];
uint8_t rx[5];
uint32_t value;

/*
First request
*/

tx[0]=address;
tx[1]=0;
tx[2]=0;
tx[3]=0;
tx[4]=0;

TMC5160_CS_LOW(dev);
	
HAL_SPI_TransmitReceive(&hspi1, tx, rx, 5, HAL_MAX_DELAY);

TMC5160_CS_HIGH(dev);

/*
Second dummy transfer

get requested register
*/

tx[0]=0;
tx[1]=0;
tx[2]=0;
tx[3]=0;
tx[4]=0;

TMC5160_CS_LOW(dev);

HAL_SPI_TransmitReceive(&hspi1, tx, rx, 5, HAL_MAX_DELAY);

TMC5160_CS_HIGH(dev);

value =
((uint32_t)rx[1]<<24) |
((uint32_t)rx[2]<<16) |
((uint32_t)rx[3]<<8) |
((uint32_t)rx[4]);

return value;

}

/*
==================================================
Initialization

STEP/DIR mode
==================================================
*/

void TMC5160_Init(const TMC5160_HandleTypeDef *dev, uint32_t current)
{

/*
Keep driver disabled
*/

TMC5160_DISABLE(dev);

HAL_Delay(10);

/*
Clear error flag
*/

TMC5160_WriteRegister(dev, TMC5160_GSTAT, TMC5160_GSTAT_INIT);

/*
General configuration
*/

TMC5160_WriteRegister(dev,TMC5160_GCONF,TMC5160_GCONF_INIT);

/*
Current scaling

RSENSE=25mR

0 means 256
*/

TMC5160_WriteRegister(dev, TMC5160_GLOBALSCALER, TMC5160_GLOBALSCALER_INIT);

/*
Motor current
*/

TMC5160_WriteRegister(dev, TMC5160_IHOLD_IRUN, current);

/*
Standstill power down delay
*/

TMC5160_WriteRegister(dev, TMC5160_TPOWERDOWN, TMC5160_TPOWERDOWN_INIT);

/*
Chopper

1/64 microstep
*/

TMC5160_WriteRegister(dev, TMC5160_CHOPCONF, TMC5160_CHOPCONF_INIT);

/*
Disable CoolStep
*/

TMC5160_WriteRegister(dev, TMC5160_COOLCONF, 0);

/*
Enable driver

ENN LOW

*/

TMC5160_ENABLE(dev);

}
