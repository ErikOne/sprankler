#include "platform.h"

#include <os/utilsIntf.h>
#include <os/memIntf.h>
#include <platform/platformIntf.h>
#include <logging/logging.h>

#include <stdlib.h>
#include <fcntl.h>

static struct _pushButtonData
{
  char_t filePath[255];
  int32_t fd;
} localPushButtonData = {};

static K_Status_e localSetupPushButton(struct _pushButtonData * button);
static void localHandlePushButtonTrigger(int32_t fd, uint8_t * data, uint32_t nbrOfBytes);

K_Status_e platform_setupPumpSwitch(void)
{
  K_Status_e rc = K_Status_General_Error;

  struct _pushButtonData tmp = {};
  if (localSetupPushButton(&tmp) == K_Status_OK)
  {

    if (getPlatformIntf()->addPollHandler(tmp.fd, localHandlePushButtonTrigger) == K_Status_OK)
    {
      INFO("Successfully add push button to system poller\n");
      rc = K_Status_OK;

      localPushButtonData = tmp;
    }
  }

  return rc;
}

static K_Status_e localSetupPushButton(struct _pushButtonData * button)
{
  K_Status_e rc = K_Status_General_Error;

  if (button != NULL)
  {
    const IUtils_t * const utils = getUtilsIntf();
    const IMem_t * const mem = getMemIntf();

    char_t dir[255];

    if (utils->stringWrite(dir, sizeof(dir), NULL, PLATFORM_GPIO_DIR_TEMPLATE, GPIO(4)) != K_Status_OK)
    {
      FATAL("Could not setup directory for Push Button\n");
    }
    else if (utils->stringWrite(button->filePath, sizeof button->filePath, NULL, "%s/%s", dir, VALUE_FILE) != K_Status_OK)
    {
      FATAL("Unable to create file name for push button\n");
    }
    else if (utils->fileExists(button->filePath, sizeof(button->filePath)) != K_True)
    {
      WARNING("%s, does not exist, skipping push button\n", button->filePath);
    }
    else if (platform_sysWrite(dir, DIRECTION_FILE, DIRECTION_IN) != K_Status_OK)
    {
      WARNING("Could not configure edge trigger for %s to %s\n", dir, EDGE_RISING);
    }
    else if (platform_sysWrite(dir, EDGE_FILE, EDGE_RISING) != K_Status_OK)
    {
      WARNING("Could not configure edge trigger for %s to %s\n", dir, EDGE_RISING);
    }
    else if (platform_sysWrite(dir, ACTIVE_LOW_FILE, VALUE_ACTIVE) != K_Status_OK)
    {
      WARNING("Could not configure active_low for %s to %s\n", dir, VALUE_ACTIVE);
    }
    else
    {
      button->fd = open(button->filePath, O_RDONLY | O_NONBLOCK);
      if (button->fd < 0)
      {
        ERROR("Could not open pulse counter %s\n", button->filePath);
      }
      else
      {
        INFO("Successfully opened pulse counter %s\n", button->filePath);
        rc = K_Status_OK;
      }

    }

    if (rc != K_Status_OK)
    {
      mem->memset(button, 0, sizeof(struct _pushButtonData));
    }
  }

  return rc;
}

static void localHandlePushButtonTrigger(int32_t fd, uint8_t * data, uint32_t nbrOfBytes)
{
  if ((data != NULL) && (localPushButtonData.fd != 0))
  {
    const IUtils_t * const utils = getUtilsIntf();

    /* Strip the trailing \n */
    if (nbrOfBytes > 0)
    {
      data[nbrOfBytes - 1] = '\0';
    }

    int64_t value;
    if (utils->stringToInt((char_t *) data, 10, &value) == K_Status_OK)
    {
      if (value == 1)
      {
        static uint64_t lastTime;

        if (fd == localPushButtonData.fd)
        {
          OsTime_t now = {};

          if (utils->getTimeOfDay(&now) == K_Status_OK)
          {
            uint64_t x = now.sec * 1000000000 + now.nano_sec;

            if (x - lastTime > 500000000)
            {
              const IPlatform_t * const platform = getPlatformIntf();
              printf("Button was pressed\n");

              platform->toggleGPIO(GPIO_Relais_1);
              platform->toggleGPIO(GPIO_Relais_2);
            }
            lastTime = x;
          }
        }
      }
    }
  }

}
