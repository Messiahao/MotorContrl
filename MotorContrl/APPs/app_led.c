#include "app_led.h"
#include "led.h"

/*
 * LED toggle interval
 *
 * Unit:
 * millisecond
 */

/*
 * Store the last toggle timestamp
 */
static uint32_t led_last_tick;

/*
 * LED current state
 *
 * 0 : LED OFF
 * 1 : LED ON
 */
static uint8_t led_state;

/*
 * AppLed_Init()
 *
 * Called once after GPIO initialization.
 */
void AppLed_Init(void)
{

    /*
     * Initialize timestamp
     *
     * HAL_GetTick() returns system uptime in ms.
     */
    led_last_tick = HAL_GetTick();

    /*
     * Because LED is connected:
     *
     * 3.3V
     *  |
     * LED
     *  |
     * MCU GPIO
     *
     * It is active-low:
     *
     * GPIO LOW  -> LED ON
     * GPIO HIGH -> LED OFF
     *
     * Set initial state OFF.
     */
    led_state = 0;

    BspLed_Init();

}

/*
 * AppLed_Process()
 *
 * Non-blocking periodic task.
 *
 * It only checks elapsed time.
 * No delay exists here.
 */
void AppLed_Process(void)
{

    uint32_t current_tick = HAL_GetTick();

    /*
     * Check whether 500ms has elapsed.
     *
     * Using subtraction instead of:
     *
     * if(current_tick > led_last_tick + 500)
     *
     * because subtraction method handles
     * uint32_t overflow correctly.
     */
    if((current_tick - led_last_tick) >= LED_BLINK_PERIOD_MS)
    {
			
        /*
         * Update timestamp
         */
        led_last_tick = current_tick;

        /*
         * Toggle LED state
         */
        led_state ^= 1;

        /*
         * Active-low LED:
         *
         * led_state = 1
         * GPIO LOW
         * LED ON
         *
         * led_state = 0
         * GPIO HIGH
         * LED OFF
         */
        BspLed_Write((led_state != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET);

    }

}
