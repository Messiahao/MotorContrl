#ifndef __TMC5160_H
#define __TMC5160_H

#include "main.h"

#define TMC5160_FRAME_SIZE 5
#define TMC5160_WRITE_BIT 0x80
#define TMC5160_INIT_DELAY_MS 10
#define TMC5160_VERSION_SHIFT 24
#define TMC5160_VERSION_VALUE 0x30U
#define TMC5160_MSCNT_MASK 0x03FFU
#define TMC5160_MSCNT_MODULUS 0x0400U
#define TMC5160_BYTE1_SHIFT 8
#define TMC5160_BYTE2_SHIFT 16
#define TMC5160_BYTE3_SHIFT 24
#define TMC5160_COOLCONF_INIT 0


/*
==================================================
TMC5160 device structure
==================================================
*/

typedef struct
{

    GPIO_TypeDef *CS_GPIO_Port;
    uint16_t CS_Pin;

    GPIO_TypeDef *EN_GPIO_Port;
    uint16_t EN_Pin;

}TMC5160_HandleTypeDef;

void BspTmc5160_WriteEnable(const TMC5160_HandleTypeDef *dev, uint8_t enabled);

/*
==================================================
SPI handle
==================================================
*/


/*
==================================================
CS control
==================================================
*/



/*
==================================================
ENN control

TMC5160:
ENN = Low enable
ENN = High disable
==================================================
*/



/*
==================================================
Register address
==================================================
*/

#define TMC5160_GCONF          0x00
#define TMC5160_GSTAT          0x01
#define TMC5160_IOIN           0x04
#define TMC5160_SHORT_CONF     0x09
#define TMC5160_DRV_CONF       0x0A
#define TMC5160_GLOBALSCALER   0x0B

#define TMC5160_IHOLD_IRUN     0x10
#define TMC5160_TPOWERDOWN     0x11

#define TMC5160_CHOPCONF       0x6C
#define TMC5160_COOLCONF       0x6D
#define TMC5160_DRV_STATUS     0x6F

#define TMC5160_PWMCONF        0x70

#define TMC5160_MSCNT          0x6A

/*
==================================================
GCONF 0x00
==================================================
*/

#define TMC5160_GCONF_EN_PWM_MODE			(1<<2)

#define TMC5160_GCONF_MULTISTEP_FILT	(1<<3)

/*
STEP/DIR mode:

disable StealthChop
enable step filter
*/

#define TMC5160_GCONF_INIT		( TMC5160_GCONF_MULTISTEP_FILT )

/*
==================================================
GSTAT 0x01
==================================================
*/

#define TMC5160_GSTAT_RESET						(1<<0)

#define TMC5160_GSTAT_DRV_ERR					(1<<1)

#define TMC5160_GSTAT_UV_CP						(1<<2)

#define TMC5160_GSTAT_INIT		( TMC5160_GSTAT_RESET | TMC5160_GSTAT_DRV_ERR | TMC5160_GSTAT_UV_CP )

/*
==================================================
IHOLD_IRUN 0x10
==================================================
*/

#define TMC5160_IHOLD(x)						((x)&0x1F)

#define TMC5160_IRUN(x)							(((x)&0x1F)<<8)

#define TMC5160_IHOLDDELAY(x)				(((x)&0x0F)<<16)

/*
4.2A motor
*/

#define TMC5160_CURRENT_4A2		( TMC5160_IHOLD(7) | TMC5160_IRUN(14) | TMC5160_IHOLDDELAY(6) )

/*
2.8A motor
*/

#define TMC5160_CURRENT_2A8		( TMC5160_IHOLD(5) | TMC5160_IRUN(9) | TMC5160_IHOLDDELAY(6) )

/*
==================================================
CHOPCONF 0x6C
==================================================
*/

#define TMC5160_TOFF(x)							((x)&0x0F)

#define TMC5160_TOFF_MASK					0x0F
				
#define TMC5160_HSTRT(x)						(((x)&0x07)<<4)
				
#define TMC5160_HEND(x)							(((x)&0x0F)<<7)
				
#define TMC5160_TBL(x)							(((x)&0x03)<<15)

/*
MRES:

0 = 256
1 = 128
2 = 64
*/

#define TMC5160_MRES(x)							(((x)&0x0F)<<24)

#define TMC5160_INTPOL							(1<<28)

/*
1/64 microstep

TOFF=3
HSTRT=5
HEND=2
TBL=2
MRES=2
INTPOL=1
*/

#define TMC5160_CHOPCONF_INIT			( TMC5160_INTPOL | TMC5160_MRES(2) | TMC5160_TBL(2) | TMC5160_HEND(2) | TMC5160_HSTRT(5) | TMC5160_TOFF(3) )

#define TMC5160_GLOBALSCALER_INIT 	0

#define TMC5160_TPOWERDOWN_INIT 		10

/*
==================================================
Function
==================================================
*/

void BspTmc5160_WriteRegister(const TMC5160_HandleTypeDef *dev, uint8_t address, uint32_t data);

uint32_t BspTmc5160_ReadRegister(const TMC5160_HandleTypeDef *dev, uint8_t address);

void BspTmc5160_Init(const TMC5160_HandleTypeDef *dev, uint32_t current);

#endif
