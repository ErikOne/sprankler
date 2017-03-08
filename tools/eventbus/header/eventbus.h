/*
 * dispatcher.h
 *
 *  Created on: May 15, 2013
 *      Author: erik
 */

#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <eventbus/eventbusIntf.h>
#include <os/threadIntf.h>
#include <queue/queueIntf.h>

#define EVENTBUS_MIN_QUEUESIZE  5
#define EVENTBUS_MAX_QUEUESIZE  1024

enum BusState_e
{
  busState_created = 10,
  busState_initializing = 20,
  busState_ready = 30,
  busState_running = 40,
  busState_shutdown = 50
};

enum BusEventType_e
{
  bus_ShutdownRequest,
  bus_RegisterHandleRequest,
  bus_UnregisterHandleRequest,
  bus_sendEventRequest,
  bus_startRequest,
  bus_initHandleRequest,
};

struct _EventBus
{
  OsMutex_t mutex;
  OsCondition_t condition;
  OsThread_t workerThread;
  Queue_t queue;
  BUSINIT initFunction;

  volatile enum BusState_e state;
  struct _BusEvent * storage;
  uint64_t handledEvents;
  uint16_t numberOfHandles;
};

struct _BusHandle
{
  struct _EventBus * bus;
  void * userData;
  EVENTHANDLER handler;
  HandleInit initializer;
  HandleCleanup destructor;

  volatile enum BusState_e state;
};

struct _BusEvent
{
  enum BusEventType_e eventType;
  struct _EventBus * dispatcher;
  struct _BusHandle * handle;
  EVENTHANDLER handler;
  void * userData;
};

#define BUS_ACTIVE(b) (((b->state == busState_ready) || (b->state == busState_running))?K_True:K_False)
#define BUS_INITCALLED(b) (((b->state == busState_shutdown) || (b->state == busState_running))?K_True:K_False)

EventBus_t bus_createEventBus(BUSINIT initializer, uint16_t queueSize);
K_Status_e bus_destroyEventBus(EventBus_t bus);
K_Status_e bus_startBus(EventBus_t bus);
K_Status_e bus_unregisterHandle(EventBus_t bus, BusHandle_t handle);
K_Status_e bus_registerHandle(EventBus_t bus, BusHandle_t handle, EVENTHANDLER handler, void * userData);

BusHandle_t handle_createBusHandle(HandleInit init, HandleCleanup destructor);
K_Status_e handle_destroyBusHandle(BusHandle_t handle);
K_Status_e handle_initBusHandle(BusHandle_t handle);
K_Status_e handle_sendToHandle(BusHandle_t handle, void * data);
void * handle_getUserData(BusHandle_t handle);
K_Status_e ebuswrapper_handleOnBus(BUS_EVENTHANDLER handler, void * data, EVENT_DATADESTRUCTOR destructor, EventBus_t bus);

#endif /* DISPATCHER_H_ */
