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
  Pin_Relais_Unknown = 0,
  PIN_Relais_1 = GPIO(27),
  PIN_Relais_2 = GPIO(22),
  PIN_Input_Switch = GPIO(4),
} PlatformPins_e;

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

} IPlatform_t;

IPlatform_t * getPlatformIntf(void);

#endif /* EXPORT_PLATFORMINTF_H_ */
