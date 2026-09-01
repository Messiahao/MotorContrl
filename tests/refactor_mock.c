/* Host-only HAL boundary mocks, prepended by check_refactor.py.
   Never included in the firmware project; state definitions belong in this .c. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef enum { GPIO_PIN_RESET = 0, GPIO_PIN_SET = 1 } GPIO_PinState;
typedef enum { HAL_OK = 0, HAL_ERROR = 1 } HAL_StatusTypeDef;
typedef struct { unsigned id; uint32_t IDR; } GPIO_TypeDef;
typedef struct { uint32_t Pin, Mode, Pull, Speed; } GPIO_InitTypeDef;
typedef struct { uint32_t DR; } USART_TypeDef;
typedef struct { USART_TypeDef *Instance; } UART_HandleTypeDef;
typedef struct { void *Instance; } TIM_HandleTypeDef;
typedef struct { unsigned unused; } SPI_HandleTypeDef;
static GPIO_TypeDef ports[3] = {{0,65535}, {1,65535}, {2,65535}};
static USART_TypeDef uart_instance;
UART_HandleTypeDef huart3 = { &uart_instance };
TIM_HandleTypeDef htim2 = { (void *)0x40000000 };
SPI_HandleTypeDef hspi1;
#define GPIOA (&ports[0])
#define GPIOB (&ports[1])
#define GPIOC (&ports[2])
#define GPIO_MODE_OUTPUT_PP 1
#define GPIO_MODE_AF_PP 2
#define GPIO_NOPULL 0
#define GPIO_SPEED_FREQ_HIGH 3
#define GPIO_PIN_0 (1U << 0)
#define GPIO_PIN_1 (1U << 1)
#define GPIO_PIN_2 (1U << 2)
#define GPIO_PIN_3 (1U << 3)
#define GPIO_PIN_4 (1U << 4)
#define GPIO_PIN_5 (1U << 5)
#define GPIO_PIN_6 (1U << 6)
#define GPIO_PIN_7 (1U << 7)
#define GPIO_PIN_8 (1U << 8)
#define GPIO_PIN_9 (1U << 9)
#define GPIO_PIN_10 (1U << 10)
#define GPIO_PIN_11 (1U << 11)
#define GPIO_PIN_12 (1U << 12)
#define GPIO_PIN_13 (1U << 13)
#define GPIO_PIN_14 (1U << 14)
#define GPIO_PIN_15 (1U << 15)
#define TIM2 ((void *)0x40000000)
#define TIM2_IRQn 28
#define TIM_CHANNEL_2 4
#define TIM_FLAG_CC2 4
#define TIM_FLAG_UPDATE 1
#define TIM_IT_CC2 4
#define TIM_IT_UPDATE 1
#define UART_FLAG_ORE 8
#define UART_FLAG_RXNE 32
#define RESET 0
#define HAL_MAX_DELAY 0xffffffffU

static uint32_t tick, input_levels[3] = {65535,65535,65535};
static uint32_t output_levels[3], registers_tmc[128];
static uint32_t period, compare_value, counter, interrupt_enable, pwm_running;
static unsigned fault, tx_failure, irq_after_gpio, pulse_direction;
static unsigned ore, rx_read, rx_count;
static uint8_t rx_data[4096];
static uint16_t i2c_address;
static uint8_t i2c_data[4];
static uint16_t i2c_length;
static unsigned i2c_write_count;
static uint64_t trace_hash = 14695981039346656037ULL;
static uint32_t event_count;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
static void Trace(uint32_t tag, uint32_t a, uint32_t b, uint32_t c)
{
    uint32_t words[4] = {tag,a,b,c};
    unsigned i, j;
    ++event_count;
    for (i=0;i<4;i++) for (j=0;j<4;j++) {
        trace_hash ^= (words[i] >> (j*8)) & 255;
        trace_hash *= 1099511628211ULL;
    }
}
static void MockPulse(void)
{
    if (pwm_running) {
        registers_tmc[0x6a] = (registers_tmc[0x6a] + (pulse_direction ? 1023 : 1)) & 1023;
        HAL_TIM_PeriodElapsedCallback(&htim2);
    }
}
static uint32_t HAL_GetTick(void) { Trace(1,tick,0,0); return tick; }
static void HAL_Delay(uint32_t delay)
{
    Trace(2,delay,0,0); tick += delay;
    if ((fault==6 && delay==100) || (fault==7 && delay==200)) input_levels[2] &= ~GPIO_PIN_6;
}
static GPIO_PinState HAL_GPIO_ReadPin(GPIO_TypeDef *port, uint16_t pin)
{
    port->IDR = input_levels[port->id];
    GPIO_PinState level = (input_levels[port->id] & pin) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    Trace(3,port->id,pin,level);
    if (irq_after_gpio && --irq_after_gpio == 0) MockPulse();
    return level;
}
static void HAL_GPIO_WritePin(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState level)
{
    Trace(4,port->id,pin,level);
    if (port==GPIOA && pin==GPIO_PIN_1 && level && !(output_levels[0]&pin))
        registers_tmc[0x6a] = (registers_tmc[0x6a]+1)&1023;
    if (level) output_levels[port->id] |= pin; else output_levels[port->id] &= ~pin;
    if (port==GPIOA && pin==GPIO_PIN_0) pulse_direction = level;
}
static void HAL_GPIO_Init(GPIO_TypeDef *port, GPIO_InitTypeDef *cfg)
{
    Trace(5,port->id,cfg->Pin,cfg->Mode); Trace(6,cfg->Pull,cfg->Speed,0);
}
static void HAL_NVIC_SetPriority(unsigned irq, unsigned preempt, unsigned sub) { Trace(7,irq,preempt,sub); }
static void HAL_NVIC_ClearPendingIRQ(unsigned irq) { Trace(8,irq,0,0); }
static void HAL_NVIC_EnableIRQ(unsigned irq) { Trace(9,irq,0,0); }
static HAL_StatusTypeDef HAL_TIM_PWM_Start(TIM_HandleTypeDef *htim, unsigned channel)
{
    (void)htim; Trace(10,channel,0,0); pwm_running = fault==5 ? 0 : 1; return fault==5 ? HAL_ERROR : HAL_OK;
}
static HAL_StatusTypeDef HAL_TIM_PWM_Stop(TIM_HandleTypeDef *htim, unsigned channel)
{
    (void)htim; Trace(11,channel,0,0); pwm_running=0; return HAL_OK;
}
#define __HAL_TIM_SET_AUTORELOAD(h,v) (Trace(12,(v),0,0),period=(v))
#define __HAL_TIM_SET_COMPARE(h,c,v) (Trace(13,(c),(v),0),compare_value=(v))
#define __HAL_TIM_SET_COUNTER(h,v) (Trace(14,(v),0,0),counter=(v))
#define __HAL_TIM_CLEAR_FLAG(h,v) Trace(15,(v),0,0)
#define __HAL_TIM_DISABLE_IT(h,v) (Trace(16,(v),0,0),interrupt_enable&=~(v))
#define __HAL_TIM_ENABLE_IT(h,v) (Trace(17,(v),0,0),interrupt_enable|=(v))
static unsigned MockUartFlag(unsigned flag)
{
    if(flag==UART_FLAG_ORE) { Trace(18,flag,ore,0); return ore; }
    if(rx_read<rx_count) { uart_instance.DR=rx_data[rx_read++]; Trace(18,flag,1,uart_instance.DR); return 1; }
    Trace(18,flag,0,0); return 0;
}
static void MockClearOre(void) { Trace(19,0,0,0); ore=0; }
#define __HAL_UART_GET_FLAG(h,f) MockUartFlag(f)
#define __HAL_UART_CLEAR_OREFLAG(h) MockClearOre()
static HAL_StatusTypeDef HAL_UART_Transmit(UART_HandleTypeDef *uart, uint8_t *data, uint16_t len, uint32_t timeout)
{
    unsigned i; (void)uart; Trace(20,len,timeout,tx_failure);
    for(i=0;i<len;i++) Trace(21,i,data[i],0);
    return tx_failure ? HAL_ERROR : HAL_OK;
}
static uint32_t MockTmcRead(unsigned address)
{
    uint32_t value=registers_tmc[address];
    if(fault==1 && address==0x04) value=0;
    if(fault==2 && address==0x00) value^=1;
    if(fault==3 && address==0x01) value|=4;
    if(fault==4 && address==0x6c) value^=1;
    if(fault==8 && address==0x01 && !(output_levels[2]&GPIO_PIN_3)) value|=2;
    Trace(22,address,value,0); return value;
}
static void MockTmcWrite(unsigned address, uint32_t value)
{
    Trace(23,address,value,0);
    if(address==0x01) registers_tmc[address]&=~value; else registers_tmc[address]=value;
}
static void HAL_Init(void) { Trace(24,0,0,0); }
void SystemClock_Config(void) { Trace(25,0,0,0); }
static void MockInit(unsigned index) { Trace(26,index,0,0); }
static HAL_StatusTypeDef BspI2c_Write(uint16_t address, uint8_t *data,
                                      uint16_t length, uint32_t timeout)
{
    unsigned i;
    i2c_address = address;
    i2c_length = length;
    i2c_write_count++;
    Trace(27,address,length,timeout);
    for (i=0; i<length && i<sizeof(i2c_data); i++) {
        i2c_data[i] = data[i];
        Trace(28,i,data[i],0);
    }
    return HAL_OK;
}
