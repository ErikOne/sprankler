/*
 * handle.c
 *
 *  Created on: May 15, 2013
 *      Author: erik
 */

#include <common/k_types.h>
#include <logging/logging.h>
#include <os/memIntf.h>
#include <os/threadIntf.h>
#include <queue/queueIntf.h>
#include <eventbus/eventbusIntf.h>

#include "eventbus.h"

BusHandle_t handle_createBusHandle(HandleInit init, HandleCleanup destructor)
{
  const IMem_t * const mem = getMemIntf();
  BusHandle_t newHandle = (struct _BusHandle*) mem->malloc(sizeof (struct _BusHandle));
  if (newHandle != NULL)
  {
    mem->memset(newHandle,0,sizeof(struct _BusHandle));
    newHandle->state = busState_created;
    newHandle->initializer = init;
    newHandle->destructor = destructor;
  }

  return newHandle;
}

K_Status_e handle_destroyBusHandle(BusHandle_t handle)
{
  K_Status_e rc = K_Status_General_Error;

  if (handle != NULL)
  {
    const IMem_t * const mem = getMemIntf();

    mem->free(handle);
    rc = K_Status_OK;
  }

  return rc;
}

K_Status_e handle_sendToHandle(BusHandle_t handle, void *data)
{
  K_Status_e rc = K_Status_General_Error;

  if ((handle != NULL) && (handle->bus != NULL))
  {
    EventBus_t bus = handle->bus;
    if (BUS_ACTIVE(bus) == K_True)
    {
      struct _BusEvent sendEvent;
      sendEvent.eventType = bus_sendEventRequest;
      sendEvent.userData = data;
      sendEvent.handle = handle;

      rc = getQueueIntf()->putItem(bus->queue, &sendEvent);
    }
  }

  return rc;
}

void* handle_getUserData(BusHandle_t handle)
{
  void* data = NULL;

  if (handle != NULL)
  {
    data = handle->userData;
  }

  return data;
}

