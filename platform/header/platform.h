/*
 * platform.h
 *
 *  Created on: Mar 4, 2017
 *      Author: erik
 */

#ifndef HEADER_PLATFORM_H_
#define HEADER_PLATFORM_H_

#include <common/k_types.h>
#include <platform/platformIntf.h>

#define PLATFORM_GPIO_DIR           "/sys/class/gpio"
#define PLATFORM_GPIO_DIR_TEMPLATE  "/sys/class/gpio/gpio%u"

#define EXPORT_FILE                       (const char_t *) "export"
#define DIRECTION_FILE                    (const char_t *) "direction"
#define EDGE_FILE                         (const char_t *) "edge"
#define VALUE_FILE                        (const char_t *) "value"
#define ACTIVE_LOW_FILE                   (const char_t *) "active_low"

#define DIRECTION_IN                      (const char_t *) "in"
#define DIRECTION_OUT                     (const char_t *) "out"
#define EDGE_RISING                       (const char_t *) "rising"
#define EDGE_FALLING                      (const char_t *) "falling"
#define EDGE_BOTH                         (const char_t *) "both"
#define EDGE_NONE                         (const char_t *) "none"
#define VALUE_ACTIVE                      (const char_t *) "1"
#define VALUE_INACTIVE                    (const char_t *) "0"

K_Status_e platform_init(void);
K_Status_e platform_setupPumpSwitch(void);
K_Status_e platform_setupWallPlug(void);
K_Status_e platform_initSysPoller(void);

K_Status_e platform_sysWrite(const char_t * dir, const char_t * file, const char_t * data);
K_Status_e platform_getDirForGPIO(PlatformGPIO_e gpio, char_t * buffer, size_t size);

K_Status_e platform_addPollHandler(int32_t fd, POLLHANDLER handler);
K_Status_e platform_setGPIOActive(PlatformGPIO_e pin, K_Boolean_e active);

K_Status_e platform_openGPIO(PlatformGPIO_e gpio);
K_Boolean_e platform_isGPIOActive(PlatformGPIO_e gpio);
K_Status_e platform_toggleGPIO(PlatformGPIO_e gpio);

#endif /* HEADER_PLATFORM_H_ */
