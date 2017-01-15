/*
 * utilsIntf.h
 *
 *  Created on: Jan 15, 2017
 *      Author: erik
 */

#ifndef OS_EXPORT_UTILSINTF_H_
#define OS_EXPORT_UTILSINTF_H_

#include <common/k_types.h>

typedef struct _utils_interface
{
  K_Status_e (* sleep)(uint32_t seconds);

  K_Status_e (* usleep)(uint32_t microseconds);

} IUtils_t;

IUtils_t * getUtilsIntf(void);

#endif /* OS_EXPORT_UTILSINTF_H_ */
