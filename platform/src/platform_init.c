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
    if (platform_setupPumpSwitch() == K_Status_OK)
    {
      if (platform_setupWallPlug() == K_Status_OK)
      {
        platform_setGPIOActive(GPIO_Relais_1, K_False);
        platform_setGPIOActive(GPIO_Relais_2, K_False);
        rc = K_Status_OK;
      }
      else
      {
        ERROR("Could not set up Wall Plug\n");
      }
    }
    else
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
