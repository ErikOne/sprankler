/*
 * handle_onbus.c
 *
 *  Created on: Mar 8, 2017
 *      Author: erik
 */

#include <eventbus/eventbusIntf.h>
#include <os/memIntf.h>

#include "eventbus.h"


typedef struct
{
  BUS_EVENTHANDLER handler;
  void * data;
  EVENT_DATADESTRUCTOR destructor;
} InternalEvent_t;

static void localDestroyEvent(InternalEvent_t * event)
{
  const IMem_t * const mem = getMemIntf();

  if (event->data != NULL)
  {
    if (event->destructor != NULL)
    {
      event->destructor(event->data);
    }
    else
    {
      mem->free(event->data);
    }
  }
  mem->free(event);
}

static void localEventbusHandler(BusHandle_t handle, void* event)
{
  InternalEvent_t * ievent = (InternalEvent_t *) event;
  if (ievent != NULL)
  {
    ievent->handler(ievent->data);
    localDestroyEvent(ievent);
  }
}

static struct _BusHandle s_fakehandler =
{
    NULL,
    NULL,
    localEventbusHandler,
    NULL,
    NULL,
    busState_ready
};

K_Status_e ebuswrapper_handleOnBus(BUS_EVENTHANDLER handler, void * data, EVENT_DATADESTRUCTOR destructor, EventBus_t bus)
{
  K_Status_e rc = K_Status_General_Error;

  const IMem_t * const mem = getMemIntf();
  if ((handler != NULL) && (bus != NULL) )
  {
    if (BUS_ACTIVE(bus) == K_True)
    {
      InternalEvent_t * event = (InternalEvent_t *) mem->malloc(sizeof(InternalEvent_t));
      if (event != NULL)
      {
        mem->memset(event, 0, sizeof(InternalEvent_t));
        event->handler = handler;

        if (data != NULL)
        {
          event->data = data;
          event->destructor = destructor;
        }

        const IQueue_t *const qi = getQueueIntf();
        struct _BusEvent sendEvent;
        sendEvent.eventType = bus_sendEventRequest;
        sendEvent.userData = event;
        sendEvent.handle = &s_fakehandler;

        rc = qi->putItem(bus->queue, &sendEvent);
      }

      if (rc != K_Status_OK)
      {
        localDestroyEvent(event);
      }
    }
  }
  else
  {
    if (data != NULL)
    {
      if (destructor != NULL)
      {
        destructor(data);
      }
      else
      {
        mem->free(data);
      }
    }
  }

  return rc;
}


K_Status_e eventbus_handleOnBus(BUS_EVENTHANDLER handler, void * data, EVENT_DATADESTRUCTOR destructor, EventBus_t bus)
{
  K_Status_e rc = K_Status_General_Error;

  return rc;
}





