/*
 * k_types.h
 *
 *  Created on: Jan 4, 2017
 *      Author: erik
 */

#ifndef COMMON_EXPORT_K_TYPES_H_
#define COMMON_EXPORT_K_TYPES_H_

#include <stdint.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include <stdbool.h>

typedef enum
{
  K_True = 1,
  K_False = 2
} K_Boolean_e;

typedef enum
{
  K_Status_unknown = 0,
  K_Status_OK = 1,
  K_Status_General_Error = 2,
  K_Status_Invalid_Param = 3,
  K_Status_Locked = 4,
  K_Status_Unexpected_State = 5,
  K_Status_NoResult = 6

} K_Status_e;

typedef unsigned char uchar_t;
typedef char char_t;

#endif /* COMMON_EXPORT_K_TYPES_H_ */
