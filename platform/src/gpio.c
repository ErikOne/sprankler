/*
 * gpio.c
 *
 *  Created on: Mar 5, 2017
 *      Author: erik
 */

#include "platform.h"

#include <os/utilsIntf.h>
#include <logging/logging.h>

K_Status_e platform_setGPIOActive(PlatformGPIO_e pin, K_Boolean_e active)
{
  K_Status_e rc = K_Status_General_Error;

  char_t buffer[256];
  if (platform_getDirForGPIO(pin, buffer, sizeof(buffer)) == K_Status_OK)
  {
    if (platform_sysWrite(buffer, VALUE_FILE, (active == K_True)?VALUE_ACTIVE:VALUE_INACTIVE) == K_Status_OK)
    {
      rc = K_Status_OK;
    }
  }

  return rc;
}

K_Boolean_e platform_isGPIOActive(PlatformGPIO_e gpio)
{
  K_Boolean_e active = K_False;

  const IUtils_t * const utils = getUtilsIntf();
  char_t filename[256];

  if (utils->stringWrite(filename, sizeof(filename), NULL, PLATFORM_GPIO_DIR_TEMPLATE "/value", gpio) == K_Status_OK)
  {
    if (utils->fileExists(filename, sizeof(filename)) == K_True)
    {
      OsFile_t file;
      if (utils->openFile(&file, filename, F_READONLY) == K_Status_OK)
      {
        char_t data[2] = {};
        if (utils->readFromFile(file, (void *) data, 1, 1, NULL) == K_Status_OK)
        {
          int64_t value;
          if (utils->stringToInt((char_t *) data, 10, &value) == K_Status_OK)
          {
            if (value == 1)
            {
              active = K_True;
            }
          }
        }
      }
      else
      {
        WARNING("Could not open %s for reading\n", filename);
      }
    }
  }

  return active;
}

K_Status_e platform_toggleGPIO(PlatformGPIO_e gpio)
{
  K_Status_e rc = K_Status_General_Error;

  const IPlatform_t * const platform = getPlatformIntf();
  if (platform->isGPIOActive(gpio) == K_True)
  {
    rc = platform->setGPIOActive(gpio, K_False);
  }
  else
  {
    rc = platform->setGPIOActive(gpio, K_True);
  }

  return rc;
}
