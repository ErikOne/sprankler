/*
 * wallplug.c
 *
 *  Created on: Mar 8, 2017
 *      Author: erik
 */

#include "platform.h"

#include <os/utilsIntf.h>
#include <logging/logging.h>


static K_Status_e localSetupLine(PlatformGPIO_e gpio)
{
  K_Status_e rc = K_Status_General_Error;

  const IUtils_t * const utils = getUtilsIntf();

  char_t dir[255];
  char_t filePath[255];

  if(platform_openGPIO(gpio) != K_Status_OK)
  {
    FATAL("Could not export GPIO %d\n",gpio);
  }
  else if (utils->stringWrite(dir, sizeof(dir), NULL, PLATFORM_GPIO_DIR_TEMPLATE, gpio) != K_Status_OK)
  {
    FATAL("Could not setup directory for Relais\n");
  }
  else if (utils->stringWrite(filePath, sizeof filePath, NULL, "%s/%s", dir, VALUE_FILE) != K_Status_OK)
  {
    FATAL("Unable to create file name for wall plug\n");
  }
  else if (utils->fileExists(filePath, sizeof(filePath)) != K_True)
  {
    WARNING("%s, does not exist, skipping push button\n",filePath);
  }
  else if (platform_sysWrite(dir, DIRECTION_FILE, DIRECTION_OUT) != K_Status_OK)
  {
    WARNING("Could not set DIRECTION for %s to %s\n", dir, DIRECTION_OUT);
  }
  else if (platform_sysWrite(dir, VALUE_FILE, VALUE_INACTIVE) != K_Status_OK)
  {
    WARNING("Could not set initial value to OFF for %s to %s\n", dir, VALUE_INACTIVE);
  }
  else if (platform_sysWrite(dir, ACTIVE_LOW_FILE, VALUE_ACTIVE) != K_Status_OK)
  {
    WARNING("Could not configure active_low for %s to %s\n", dir, VALUE_ACTIVE);
  }
  else
  {
    rc = K_Status_OK;
  }

  return rc;
}

K_Status_e platform_setupWallPlug(void)
{
  K_Status_e rc = K_Status_OK;

  if (localSetupLine(GPIO_Relais_1) == K_Status_OK)
  {
    rc = localSetupLine(GPIO_Relais_2);
  }

  return rc;
}
