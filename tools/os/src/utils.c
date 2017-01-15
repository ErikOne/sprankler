/*
 * utils.c
 *
 *  Created on: Jan 15, 2017
 *      Author: erik
 */

#include "utils_impl.h"

#include <unistd.h>

K_Status_e utils_sleep(uint32_t sec)
{
  K_Status_e rc = K_Status_Invalid_Param;
  rc = (sleep(sec) == 0)?K_Status_OK:K_Status_General_Error;

  return rc;
}

K_Status_e utils_usleep(uint32_t microseconds)
{
  K_Status_e rc = K_Status_Invalid_Param;
  if (microseconds < 1000000)
  {
    rc = (usleep(microseconds) == 0)?K_Status_OK:K_Status_General_Error;
  }

  return rc;
}
