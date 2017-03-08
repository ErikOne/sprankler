/*
 * dispatcher.c
 *
 *  Created on: May 15, 2013
 *      Author: erik
 */

#include <common/k_types.h>
#include <eventbus/eventbusIntf.h>
#include <os/memIntf.h>
#include <os/threadIntf.h>
#include <queue/queueIntf.h>
#include <logging/logging.h>

#include "eventbus.h"

static K_Status_e localSetupEventBus(struct _EventBus *dispacher,BUSINIT initializer, uint16_t queueSize);
static K_Status_e localGetEvent(uint16_t pos,void* const storage,void * const item);
static K_Status_e localStoreEvent(uint16_t,void* const storage,const void* const item);
static K_Status_e localStartBus(struct _EventBus * bus);
static void* localBusMainloop(void* data);
static K_Status_e localWaitForRequest(struct _EventBus *bus);

/* Event handler functions */

static K_Status_e localHandleStartEvent(struct _BusEvent *event);
static K_Status_e localHandleRegisterEvent(struct _BusEvent *event);
static K_Status_e localHandleUnRegisterEvent(struct _BusEvent *event);
static K_Status_e localHandleBusHandleInitEvent(struct _BusEvent *event);
static K_Status_e localHandleBusHandleWriteEvent(struct _BusEvent *event);

EventBus_t bus_createEventBus(BUSINIT initializer, uint16_t queueSize)
{
  EventBus_t newBus = NULL;
  if ( (queueSize >= EVENTBUS_MIN_QUEUESIZE) && (queueSize <= EVENTBUS_MAX_QUEUESIZE))
  {
    const IMem_t * const mem = getMemIntf();
    newBus = (struct _EventBus *) mem->malloc(sizeof(struct _EventBus));
    if (newBus != NULL)
    {
      mem->memset(newBus,0,sizeof(struct _EventBus));
      if (localSetupEventBus(newBus,initializer,queueSize) == K_Status_OK)
      {
        if ( localStartBus(newBus) != K_Status_OK)
        {
          bus_destroyEventBus(newBus);
          newBus = NULL;
        }
      }
      else
      {
        bus_destroyEventBus(newBus);
        newBus = NULL;
      }
    }
  }

  return newBus;
}

K_Status_e bus_destroyEventBus(EventBus_t bus)
{
  K_Status_e rc = K_Status_General_Error;

  if (bus != NULL)
  {
    const IThread_t * const th = getThreadIntf();
    const IMem_t * const mem = getMemIntf();
    const IQueue_t * const qi = getQueueIntf();

    /* I will create a shutdown event. That event will be handled and cause the mainloop to end.
     * All off this is synchronized via the threadDestroy */

    struct _BusEvent shutdown;
    shutdown.eventType = bus_ShutdownRequest;
    shutdown.handle = 0;
    shutdown.userData = NULL;

    if (qi->putItem(bus->queue,&shutdown) == K_Status_OK)
    {
      rc = K_Status_OK;
    }
    else
    {
      FATAL("Unable to send shutdown request\n");
    }

    (void) th->threadDestroy(bus->workerThread);
    (void) qi->destroyQueue(bus->queue);
    mem->free(bus->storage);

    (void) th->conditionDestroy(bus->condition);
    (void) th->mutexDestroy(bus->mutex);

    mem->free(bus);
  }

  return rc;
}

K_Status_e bus_startBus(EventBus_t bus)
{
  K_Status_e rc = K_Status_General_Error;

  if ( (bus != NULL) && (bus->state == busState_initializing) )
  {
    const IThread_t * const th = getThreadIntf();
    const IQueue_t * const qi = getQueueIntf();

    struct _BusEvent startEvent;
    startEvent.eventType = bus_startRequest;
    startEvent.handle = 0;
    startEvent.userData = NULL;
    startEvent.dispatcher = bus;
    startEvent.handler = NULL;

    if (qi->putItem(bus->queue,&startEvent) == K_Status_OK)
    {
      INFO("Wait until bus leaves the busState_created state\n");

      while ( BUS_INITCALLED(bus) == K_False)
      {
        th->threadYield();
      }

      TRACE("New state=%d\n",bus->state);
      rc = (bus->state==busState_running)?K_Status_OK:K_Status_General_Error;
    }
  }
  return rc;
}

K_Status_e bus_registerHandle(EventBus_t bus,BusHandle_t handle, EVENTHANDLER handler, void *userData)
{
  K_Status_e rc = K_Status_General_Error;

  if ((bus != NULL) && (handle != NULL))
  {
    if ( (handle->bus == NULL) && BUS_ACTIVE(bus) )
    {
      const IThread_t * const th = getThreadIntf();
      const IQueue_t * const qi = getQueueIntf();
      struct _BusEvent registerEvent;

      registerEvent.eventType = bus_RegisterHandleRequest;
      registerEvent.handle = handle;
      registerEvent.userData = userData;
      registerEvent.dispatcher = bus;
      registerEvent.handler = handler;

      /* This loop is here to make the call synchronous to external clients */
      if (th->itsMe(bus->workerThread) == K_False)
      {
        if (K_Status_OK == qi->putItem(bus->queue,&registerEvent))
        {
          while (bus != handle->bus)
          {
            th->threadYield();
          }

          bus->numberOfHandles++;
          rc = K_Status_OK;
        }
        else
        {
          ERROR("Could not post register event.\n");
        }
      }
      else
      {
        rc = localHandleRegisterEvent(&registerEvent);
        if (rc == K_Status_OK)
        {
          bus->numberOfHandles++;
        }
      }
    }
  }

  return rc;
}

K_Status_e bus_unregisterHandle(EventBus_t bus,BusHandle_t handle)
{
  K_Status_e rc = K_Status_General_Error;

  if ((bus != NULL) && (handle != NULL))
  {
    if( (handle->bus == bus) && BUS_ACTIVE(bus) )
    {
      const IThread_t * const th = getThreadIntf();
      const IQueue_t * const qi = getQueueIntf();
      struct _BusEvent unregisterEvent;

      unregisterEvent.eventType = bus_UnregisterHandleRequest;
      unregisterEvent.handle = handle;

      /* This loop is here to make the call synchronous to external clients */
      if (th->itsMe(bus->workerThread) == K_False)
      {
        if (qi->putItem(bus->queue,&unregisterEvent) == K_Status_OK)
        {
          while ( handle->bus != NULL)
          {
            th->threadYield();
          }

          if (bus->numberOfHandles > 0)
          {
            bus->numberOfHandles--;
          }
          rc = K_Status_OK;
        }
        else
        {
          FATAL("Unable post unregister event\n");
        }
      }
      else
      {
        rc = localHandleUnRegisterEvent(&unregisterEvent);
        if (rc == K_Status_OK)
        {
          if (bus->numberOfHandles > 0)
          {
            bus->numberOfHandles--;
          }
        }
      }
    }
  }

  return rc;
}

K_Status_e handle_initBusHandle(BusHandle_t handle)
{
  K_Status_e rc = K_Status_General_Error;

  if ( (handle != NULL) && (handle->state == busState_created) )
  {
    if ((handle->bus != NULL) && BUS_ACTIVE(handle->bus) )
    {
      const IThread_t * const th = getThreadIntf();
      const IQueue_t * const qi = getQueueIntf();

      struct _BusEvent initEvent;
      initEvent.eventType = bus_initHandleRequest;
      initEvent.handle = handle;
      initEvent.userData = NULL;
      initEvent.dispatcher = NULL;
      initEvent.handler = NULL;

      if (th->itsMe(handle->bus->workerThread) == K_False)
      {
        if (qi->putItem(handle->bus->queue, &initEvent) == K_Status_OK)
        {
          while (handle->state == busState_created)
          {
            th->threadYield();
          }

          rc = (handle->state == busState_ready)?K_Status_OK:K_Status_General_Error;
        }
      }
      else
      {
        rc = localHandleBusHandleInitEvent(&initEvent);
      }
    }
  }

  return rc;
}


static K_Status_e localSetupEventBus(struct _EventBus *bus,BUSINIT initializer, uint16_t queueSize)
{
  K_Status_e rc = K_Status_General_Error;

  const IThread_t * const th = getThreadIntf();
  const IMem_t * const mem = getMemIntf();
  const IQueue_t * const qi = getQueueIntf();

  if ( (bus->mutex = th->mutexCreate()) !=  NULL)
  {
    if (  (bus->condition = th->conditionCreate()) != NULL)
    {
      uint32_t size = queueSize*(sizeof(struct _BusEvent));
      bus->storage = (struct _BusEvent *)mem->malloc(size);
      if (bus->storage != NULL)
      {
        mem->memset(bus->storage, 0, size);
        if (  (bus->queue =  qi->createQueue(Q_BLOCKING, queueSize, bus->storage, localGetEvent, localStoreEvent)) != NULL)
        {
          bus->state = busState_created;
          bus->initFunction = initializer;
          rc = K_Status_OK;
        }
      }

    }
  }

  return rc;
}

static K_Status_e localGetEvent(uint16_t pos,void* const storage,void * const item)
{
  K_Status_e rc = K_Status_General_Error;

  if (storage != NULL)
  {
    struct _BusEvent* events  = (struct _BusEvent *) storage;
    struct _BusEvent* event = (struct _BusEvent *) item;
    if (event != NULL)
    {
      const IMem_t * const mem = getMemIntf();

      if ( mem->memcpy(event,&events[pos],sizeof(struct _BusEvent)) == event )
      {
        rc = K_Status_OK;
      }
    }
  }

  return rc;
}

static K_Status_e localStoreEvent(uint16_t pos,void* const storage,const void* const item)
{
  K_Status_e rc = K_Status_General_Error;

  if (storage != NULL)
  {
    struct _BusEvent* events  = (struct _BusEvent *) storage;
    struct _BusEvent* event = (struct _BusEvent *) item;
    if (event != NULL)
    {
      const IMem_t * const mem = getMemIntf();

      if ( mem->memcpy(&events[pos],event,sizeof(struct _BusEvent)) == &events[pos] )
      {
        rc = K_Status_OK;
      }
    }
  }

  return rc;
}


static K_Status_e localStartBus(struct _EventBus *bus)
{
  K_Status_e rc = K_Status_General_Error;
  const IThread_t * const th = getThreadIntf();

  if (th->mutexLock(bus->mutex) == K_Status_OK)
  {
    bus->state = busState_created;
    if ( ( bus->workerThread =  th->threadCreate(localBusMainloop,bus) ) != NULL)
    {
      while (bus->state != busState_initializing)
      {
        INFO("Waiting for the mainloop to start initialization\n");
        th->conditionWait(bus->condition, bus->mutex);
      }
      rc = K_Status_OK;
    }

    th->mutexUnlock(bus->mutex);
  }

  return rc;
}

static void* localBusMainloop(void* data)
{
  if (data != NULL)
  {
    struct _EventBus *bus = (struct _EventBus *) data;
    const IThread_t * const th = getThreadIntf();

    while (bus->state != busState_shutdown)
    {
      switch (bus->state)
      {
        case busState_created:
          if (th->mutexLock(bus->mutex) == K_Status_OK)
          {
            if (bus->state != busState_shutdown)
            {
              bus->state = busState_initializing;
              th->conditionSignal(bus->condition);
            }
            th->mutexUnlock(bus->mutex);
          }
          break;
        case busState_initializing:
        case busState_ready:
        case busState_running:
          localWaitForRequest(bus);
          break;
        default:
          ERROR("Unhandled event bus state = %d\n", bus->state);
          th->threadYield();
          break;
      }
    }
    INFO("main selector loop finished\n");
  }

  return NULL;
}

static K_Status_e localWaitForRequest(struct _EventBus *bus)
{
    K_Status_e rc = K_Status_General_Error;

    if (bus != NULL)
    {
      const IQueue_t * const qi = getQueueIntf();
      struct _BusEvent event;

      if (qi->getItem(bus->queue,&event) == K_Status_OK)
      {
        bus->handledEvents++;
        switch (event.eventType)
        {
          case bus_ShutdownRequest:
            bus->state = busState_shutdown;
            break;
          case bus_RegisterHandleRequest:
            rc = localHandleRegisterEvent(&event);
            break;
          case bus_UnregisterHandleRequest:
            rc = localHandleUnRegisterEvent(&event);
            break;
          case bus_sendEventRequest:
            rc = localHandleBusHandleWriteEvent(&event);
            break;
          case bus_startRequest:
            rc = localHandleStartEvent(&event);
            break;
          case bus_initHandleRequest:
            rc = localHandleBusHandleInitEvent(&event);
            break;
          default:
            ERROR("Received unknown event type %d\n",event.eventType);
            break;
        }
      }
      else
      {
        ERROR("Could not retrieve event from queue\n");
      }
    }
    return rc;
}

static K_Status_e localHandleStartEvent(struct _BusEvent *event)
{
  K_Status_e rc = K_Status_General_Error;

  if ( (event != NULL) && (event->dispatcher != NULL) )
  {
    struct _EventBus *bus = event->dispatcher;
    if ( bus->initFunction != NULL)
    {
      bus->state = busState_ready;
      if (bus->initFunction((EventBus_t) bus) == K_Status_OK)
      {
        bus->state = busState_running;
        rc = K_Status_OK;
      }
      else
      {
        ERROR("Failed to run start function on event bus\n");
        bus->state = busState_shutdown;
        rc = K_Status_General_Error;
      }
    }
    else
    {
      INFO("No init function specified, going active\n");
      bus->state = busState_running;
      rc = K_Status_OK;
    }
  }

  return rc;
}

static K_Status_e localHandleRegisterEvent(struct _BusEvent *event)
{
  K_Status_e rc = K_Status_General_Error;

  if (event != NULL)
  {
    if ( (event->handle != NULL) && (event->handler != NULL) )
    {
      event->handle->handler = event->handler;
      event->handle->userData = event->userData;
      event->handle->bus = event->dispatcher;

      rc = K_Status_OK;
    }
  }
  return rc;
}

static K_Status_e localHandleUnRegisterEvent(struct _BusEvent *event)
{
  K_Status_e rc = K_Status_General_Error;

  if ( (event != NULL) && (event->handle != NULL) )
  {
    /* First call the channel's destructor */
    if (event->handle->destructor != NULL)
    {
      event->handle->destructor((BusHandle_t) event->handle);
    }
    else
    {
      TRACE("No destructor specified for the handle, are you sure this is not a memory leak\n");
    }

    event->handle->handler = NULL;
    event->handle->userData = NULL;
    event->handle->bus = NULL;
    event->handle->state = busState_created;

    rc = K_Status_OK;
  }

  return rc;
}

static K_Status_e localHandleBusHandleInitEvent(struct _BusEvent *event)
{
  K_Status_e rc = K_Status_General_Error;
  if ( (event != NULL) && (event->handle != NULL) )
  {
    /* Check again for the initCalled flag */
    if (event->handle->state == busState_created)
    {
      if (event->handle->initializer != NULL)
      {
        if (event->handle->initializer((BusHandle_t)event->handle) == K_Status_OK)
        {
          event->handle->state = busState_ready;
          rc = K_Status_OK;
        }
        else
        {
          event->handle->state = busState_initializing;
        }
      }
      else
      {
        INFO("No initializer found for handler\n");
        event->handle->state = busState_ready;
        rc = K_Status_OK;
      }
    }
  }

  return rc;
}

static K_Status_e localHandleBusHandleWriteEvent(struct _BusEvent *event)
{
  K_Status_e rc = K_Status_General_Error;
  if ( (event != NULL) && (event->handle != NULL) )
  {
    if (event->handle->state == busState_ready)
    {
      if (event->handle->handler != NULL)
      {
        event->handle->handler((BusHandle_t)event->handle,event->userData);
        rc = K_Status_OK;
      }
    }
  }

  return rc;
}
