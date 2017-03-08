/*
 * eventhandlerIntf.h
 *
 *  Created on: May 15, 2013
 *      Author: erik
 */

/*!
 * \file This file contains the interface definition for the eventbus.
 *
 * The event bus implements the reactor pattern so that it handles diferrent clients
 * in 1 single thread. This reduces locking complexities significantly.
 */

#ifndef EVENTDISPATCHERINTF_H_
#define EVENTDISPATCHERINTF_H_

#include <common/k_types.h>

/*!
 * \brief Opaque type that gives you access to the event bus
 */
typedef struct _BusHandle *BusHandle_t;

/*!
 * \brief Opque type that represents an event bus.
 */
typedef struct _EventBus *EventBus_t;


/*!
 * \brief A function that gets called every time an event (some data) was send to a
 * wellknown handle.
 *
 * \param handle The handle for which the event was aimed
 * \param data associated with the event. The memory management for the event has to aggreed
 * between publisher and consumer.
 */
typedef void (*EVENTHANDLER)(BusHandle_t handle, void* event);

/*!
 * \brief Init function of the event bus
 *
 * This function will get called only once when startBus is called. Subsequent calls to
 * startBus will be ignored.
 *
 * This function gets executed in the evenbus thread, so no locks should be taken.
 */
typedef K_Status_e (*BUSINIT)(EventBus_t bus);

typedef void (*EVENT_DATADESTRUCTOR)(void * data);
typedef void (*BUS_EVENTHANDLER)(const void * const data);

/*!
 * \brief Init function for a handle.
 *
 * A handle will only start processing event after initBusHandle has been called. This
 * functions allows the user to setup a handle within the event bus thread.
 * The init function for a handle can only be called if the handle was registered to an
 * event bus.
 */

typedef K_Status_e (*HandleInit)(BusHandle_t handle);

/*!
 * \brief A cleanup function that gets called whenever the handle is removed (unregistered)
 * from the event bus.
 *
 * This function runs in the eventbus thread just before the handle gets removed from the bus.
 */
typedef K_Status_e (*HandleCleanup)(BusHandle_t handle);

typedef struct _eventBusIntf
{
  /*!
   * \brief Creates a handle that to access an eventbus.
   *
   * \param handle : The address of the handle that will be created
   * \param init : The init function for the handle. If no initialization is needed use NULL
   * \param destructor : The cleanup function that gets called when the handle is removed from the
   * eventbus. If no cleanup is needed use NULL.
   */
  BusHandle_t (*bushandleCreate)(HandleInit init, HandleCleanup destructor);

  /*!
   * \brief Releases all resource held by a handle.
   *
   * Please note that this function does not call the destructor function of the handle. That
   * function is automatically called whenever the handle gets unregistered, This means that
   * destroying a handle prior to unregistering might result in a memory leak.
   *
   * \param handle The handle that needs to be destroyed
   */
  K_Status_e (*bushandleDestroy)(BusHandle_t handle);

  /*!
   * \brief calls the init function of the handle
   *
   * There is a good reason why the init dunction is not called automatically form within
   * the registerHandle function.
   *
   * It could be the case that during its initialisation a handle wants to send an event
   * to another handle. If that handle was not yet registered this event sending would
   * fail.
   *
   * By separating the registration of a handle and the initialization, one can write an init
   * function for the EventBus that first register all the handle and than calls init on the
   * handles.
   *
   * For example of this look at the unit test in the initSuite.c
   */
  K_Status_e (*bushandleInit)(BusHandle_t handle);

  /*!
   * \brief Creates the event bus
   *
   * \param bus he address of the bus that needs to be created
   * \param initializer The function that will be called (in the context of the EventBus thread)
   * whenever startBus is called. If no special initialization is needed you can specify NULL.
   * \param queueSize The maximum number of events that can be waiting for the bus to
   * handle them.
   */

  EventBus_t (*busCreate)(BUSINIT initializer, uint16_t queueSize);

  /*!
   * \brief Releases all resources associated with an Event bus
   *
   * Please note that one should unregister the handles from the bus before calling
   * destroyBus. Not doing so will result in memory leaks.
   *
   * \param bus he bus that needs to be destroyed
   */
  K_Status_e (*busDestroy)(EventBus_t bus);

  /*!
   * \brief Executes the init function of the bus in the context of the bus.
   *
   * \param bus The bus that needs to be initialized.
   */
  K_Status_e (*busStart)(EventBus_t bus);

  /*!
   * \brief Registers a handle to a bus
   *
   * Handles can only be registered if the bus is initialzed (e.g. after startBus was called)
   *
   * A handle can only be assigned to one bus at a time.
   *
   * \param bus The bus to which we want to register the handle.
   * \param handle The handle we want to register.
   * \param handler The function that will get called (in the context of the event bus)
   * whenever an event is send to the handle (\see send).
   * NULL is not allowed
   *
   * \param userData Data that gets assigned to this handle for later usage. This data can
   * be retrieved using getHandleData.
   * It no user data is needed, use NULL.
   */
  K_Status_e (*registerHandle)(EventBus_t bus,BusHandle_t handle, EVENTHANDLER handler, void *userData);

  /*!
   * \brief Removes the handle from the event bus.
   *
   * Whenever this function gets called the destructor function of the handler is called
   * prior to the removal of the handle form the bus.
   *
   * Please note that this function does not automatically releases the resources held by
   * the userdata. This can either be done by the cleanup function or must be done manually.
   * In the latter case note that this is done outside the bus context.
   */
  K_Status_e (*unregisterHandle)(EventBus_t bus,BusHandle_t handle);

  /*!
   * \brief Sends data to a handle.
   *
   * This will trigger the handler functions associated with this handle.
   *
   * \param handle The handle for which this data is meant. This will be the first
   * parameter f the EventHandler function that gets called
   * \param data This will be the second parameter of the EventHandler function that
   * gets called.
   */

  K_Status_e (*send)(BusHandle_t handle, void *data);

  /*!
   * \brief Returns the data associated with the handle during the registerHandle call.
   *
   * NULL is returned if no data was associated with this handle.
   */
  void* (*getHandleData)(BusHandle_t handle);



  K_Status_e (*handleOnBus)(BUS_EVENTHANDLER handler, void * data, EVENT_DATADESTRUCTOR destructor, EventBus_t bus);

} EventBusIntf_t;

EventBusIntf_t *getEventBusIntf(void);


#endif /* EVENTDISPATCHERINTF_H_ */
