#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/*
 * LED initialization function
 *
 * This function initializes the internal variables
 * used by LED task.
 *
 */
void LED_Init(void);

/*
 * LED periodic task
 *
 * This function must be called repeatedly inside main loop.
 *
 * It is non-blocking.
 *
 * Function:
 * Toggle LED state every 500ms.
 */
void LED_Task(void);

#ifdef __cplusplus
}
#endif

#endif /* __LED_H */