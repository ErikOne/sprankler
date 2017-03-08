/*
 * tools.c
 *
 *  Created on: Mar 4, 2017
 *      Author: erik
 */

#include "platform.h"

#include <os/utilsIntf.h>
#include <logging/logging.h>

K_Status_e platform_sysWrite(const char_t * dir, const char_t * file, const char_t * data)
{
  K_Status_e rc = K_Status_General_Error;
  const IUtils_t * const utils = getUtilsIntf();
  char_t buffer[255];

  if (utils->stringWrite(buffer, sizeof buffer, NULL, "%s/%s", dir, file) == K_Status_OK)
  {
    if (utils->fileExists(buffer, sizeof(buffer)) == K_True)
    {
      OsFile_t file;
      if (utils->openFile(&file, buffer, F_WRITE_CREATE) == K_Status_OK)
      {
        rc = utils->writeToFile(file, (void *) data, 1, utils->stringLength(data), NULL);
        utils->closeFile(file);
      }
      else
      {
        WARNING("Could not open %s for writing\n", buffer);
      }
    }
    else
    {
      WARNING("%s does not exists\n", buffer);
    }
  }

  return rc;
}

K_Status_e platform_getDirForGPIO(PlatformGPIO_e gpio, char_t * buffer, size_t size)
{
  K_Status_e rc = K_Status_General_Error;

  int32_t num = -1;

  switch (gpio)
  {
    case GPIO_Relais_1:
      num = GPIO(27);
      break;
    case GPIO_Relais_2:
      num = GPIO(22);
      break;
    case GPIO_Input_Switch:
      num = GPIO(4);
      break;
    default:
      break;
  }

  if (num != -1)
  {
    const IUtils_t * const utils = getUtilsIntf();
    if (utils->stringWrite(buffer, size, NULL, PLATFORM_GPIO_DIR_TEMPLATE, num) == K_Status_OK)
    {
      rc = K_Status_OK;
    }
  }

  return rc;
}

K_Status_e platform_openGPIO(PlatformGPIO_e gpio)
{
  K_Status_e rc = K_Status_Invalid_Param;
  char_t dir[255];

  if (platform_getDirForGPIO(gpio, dir, sizeof(dir)) == K_Status_OK)
  {
    const IUtils_t * const utils = getUtilsIntf();
    char_t file[255];

    if (utils->stringWrite(file, sizeof file, NULL, "%s/value", dir) == K_Status_OK)
    {
      if (utils->fileExists(file, sizeof(file)) == K_True)
      {
        rc = K_Status_OK;
      }
      else
      {
        char_t number[16];

        if (utils->stringWrite(number, sizeof(number), NULL, "%d", gpio) == K_Status_OK)
        {
          if (platform_sysWrite(PLATFORM_GPIO_DIR, EXPORT_FILE, number) == K_Status_OK)
          {
            utils->usleep(100000);
            if (utils->fileExists(file, sizeof(file)) == K_True)
            {
              rc = K_Status_OK;
            }
          }
        }
      }
    }
  }
  return rc;
}
