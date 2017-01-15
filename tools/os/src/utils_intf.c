/*
 * utils_intf.c
 *
 *  Created on: Jan 15, 2017
 *      Author: erik
 */

#include "utils_impl.h"

static IUtils_t localIntf =
{
#ifndef UNITTESTS
  .sleep  = utils_sleep,
  .usleep = utils_usleep,
#endif
};

IUtils_t * getUtilsIntf(void)
{
  return &localIntf;
}
