#include <os/utilsIntf.h>
#include <platform/platformIntf.h>
#include <eventbus/eventbusIntf.h>

#include  <logging/logging.h>

#include <sprankler/sprankler.h>

static EventBus_t myBus;

static K_Status_e localCreateEventBus(void)
{
  K_Status_e rc = K_Status_General_Error;

  if (myBus == NULL)
  {
    const EventBusIntf_t * const ebi = getEventBusIntf();

    if ((myBus = ebi->busCreate(NULL, 100)) != NULL)
    {
      if (ebi->busStart(myBus) == K_Status_OK)
      {
        printf("Successfully started bus\n");
        rc = K_Status_OK;
      }
      else
      {
        ebi->busDestroy(myBus);
        myBus = NULL;
      }
    }
  }

  return rc;
}

EventBus_t getSpranklerBus(void)
{
  return myBus;
}

int main(int argc, char ** argv)
{
  const IPlatform_t * const platform = getPlatformIntf();

  if (localCreateEventBus() == K_Status_OK)
  {
    if (platform->init() == K_Status_OK)
    {
      INFO("Successfully started system\n");
      const IUtils_t * const utils = getUtilsIntf();

      while (K_True)
      {
        utils->sleep(1);
      }
    }
  }

  return 0;
}
