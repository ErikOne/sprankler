/*
 * platform_init.c
 *
 *  Created on: Mar 4, 2017
 *      Author: erik
 */

#include "platform.h"

#include <logging/logging.h>

K_Status_e platform_init(void)
{
  K_Status_e rc = K_Status_General_Error;

  if (platform_initSysPoller() == K_Status_OK)
  {
    if (platform_setupPumpSwitch() != K_Status_OK)
    {
      ERROR("Could not set up input button\n");
    }
  }
  else
  {
    WARNING("Could not start system poller\n");
  }

  return rc;
}
