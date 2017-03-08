/*
 * dispatcher_intf.c
 *
 *  Created on: May 15, 2013
 *      Author: erik
 */

#include <eventbus/eventbusIntf.h>

#include "eventbus.h"

static EventBusIntf_t localIntf = {
#ifndef UNITTESTS
    .bushandleCreate = handle_createBusHandle,
    .bushandleDestroy = handle_destroyBusHandle,
    .bushandleInit = handle_initBusHandle,

    .busCreate = bus_createEventBus,
    .busDestroy = bus_destroyEventBus,
    .busStart = bus_startBus,

    .registerHandle = bus_registerHandle,
    .unregisterHandle = bus_unregisterHandle,

    .send =  handle_sendToHandle,
    .getHandleData = handle_getUserData,
    .handleOnBus = ebuswrapper_handleOnBus,

#endif
};


EventBusIntf_t *getEventBusIntf(void)
{
  return &localIntf;
}


