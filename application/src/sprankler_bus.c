/*
 * sprankler_bus.c
 *
 *  Created on: Mar 8, 2017
 *      Author: erik
 */

#include "sprankler.h"

static EventBus_t myBus;

K_Status_e sprankler_initEventBus(void)
{
  K_Status_e rc = K_Status_General_Error;

  if (myBus == NULL)
  {
    const EventBusIntf_t * const ebi = getEventBusIntf();

    if ((myBus = ebi->busCreate(NULL, 100)) != NULL)
    {
      if (ebi->busStart(myBus) == K_Status_OK)
      {
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

EventBus_t sprankler_getEventBus(void)
{
  return myBus;
}
