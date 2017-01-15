/*
 * utils_impl.h
 *
 *  Created on: Jan 15, 2017
 *      Author: erik
 */

#ifndef OS_HEADER_UTILS_IMPL_H_
#define OS_HEADER_UTILS_IMPL_H_

#include <os/utilsIntf.h>

K_Status_e utils_sleep(uint32_t seconds);
K_Status_e utils_usleep(uint32_t microseconds);

#endif /* OS_HEADER_UTILS_IMPL_H_ */
