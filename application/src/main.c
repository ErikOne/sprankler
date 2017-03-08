#include <os/utilsIntf.h>
#include <platform/platformIntf.h>
#include <logging/logging.h>
#include <os/threadIntf.h>

#include "sprankler.h"

int main(int argc, char ** argv)
{
  const IPlatform_t * const platform = getPlatformIntf();

  if (sprankler_initEventBus() == K_Status_OK)
  {
    if (platform->init() == K_Status_OK)
    {
      INFO("Successfully started system\n");
      const IThread_t * const ti = getThreadIntf();
      OsMutex_t godotM;
      OsCondition_t godotC;

      if ((godotM = ti->mutexCreate()) != NULL)
      {
        if ((godotC = ti->conditionCreate()) != NULL)
        {
          if (ti->mutexLock(godotM) == K_Status_OK)
          {
            (void) ti->conditionWait(godotC, godotM);

            ti->mutexUnlock(godotM);
            INFO("Godot has arrived, exiting.\n");
          }

          (void) ti->conditionDestroy(godotC);
        }
        (void) ti->mutexDestroy(godotM);
      }

    }
  }

  return 0;
}
