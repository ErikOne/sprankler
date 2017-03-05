/*
 * platform_intf.c
 *
 *  Created on: Mar 4, 2017
 *      Author: erik
 */

#include <platform/platformIntf.h>

#include "platform.h"

static IPlatform_t localIntf =
{
#ifndef UNITTESTS
  .init           = platform_init,
  .addPollHandler = platform_addPollHandler,
#endif
};

IPlatform_t * getPlatformIntf(void)
{
  return &localIntf;
}
