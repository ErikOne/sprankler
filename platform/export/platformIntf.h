/*
 * platformIntf.h
 *
 *  Created on: Jan 4, 2017
 *      Author: erik
 */

#ifndef EXPORT_PLATFORMINTF_H_
#define EXPORT_PLATFORMINTF_H_

#include <common/k_types.h>

#define GPIO(x) x

typedef void (* POLLHANDLER)(int32_t fd, uint8_t * data, uint32_t nbrOfBytes);

typedef enum
{
  GPIO_Unknown = 0,
  GPIO_Relais_1 = GPIO(27),
  GPIO_Relais_2 = GPIO(22),
  GPIO_Input_Switch = GPIO(4),
} PlatformGPIO_e;

typedef enum
{
  Direction_Unknown = 0,
  Direction_Input = 1,
  Direction_Output = 2,
} PinDirection_e;

typedef struct _platform_interface
{
  K_Status_e (* init)(void);

  K_Status_e (* addPollHandler)(int32_t fd, POLLHANDLER handler);

  K_Status_e (* setGPIOActive)(PlatformGPIO_e gpio, K_Boolean_e active);

  K_Boolean_e (* isGPIOActive)(PlatformGPIO_e gpio);

  K_Status_e (* toggleGPIO)(PlatformGPIO_e gpio);

} IPlatform_t;

IPlatform_t * getPlatformIntf(void);

#endif /* EXPORT_PLATFORMINTF_H_ */
