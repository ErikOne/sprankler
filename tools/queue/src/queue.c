/*
 * queue.c
 *
 *  Created on: Jan 9, 2014
 *      Author: erik
 */

#include <common/k_types.h>
#include <queue/queueIntf.h>
#include <logging/logging.h>

#include "queue.h"

K_Status_e qDestroyQueue(Queue_t queue)
{
  K_Status_e rc = K_Status_General_Error;

  if ((queue != NULL) && (queue->ops.queue_destroy != NULL))
  {
    rc = queue->ops.queue_destroy(queue);
  }

  return rc;
}

K_Status_e qGetItem(Queue_t queue, void * item)
{
  K_Status_e rc = K_Status_General_Error;

  if (queue != NULL)
  {
    switch (queue->policy)
    {
      case Q_NONBLOCKING:
        if (queue->ops.queue_get_non_blocking != NULL)
        {
          rc = queue->ops.queue_get_non_blocking(queue, item);
        }
        break;
      case Q_BLOCKING:
        if (queue->ops.queue_get_blocking != NULL)
        {
          rc = queue->ops.queue_get_blocking(queue, item);
        }
        break;
      default:
        ERROR("Unsuported queue type\n");
        break;
    }

  }

  return rc;
}

K_Status_e qPutItem(Queue_t queue, void * item)
{
  K_Status_e rc = K_Status_General_Error;

  if ((queue != NULL) && (queue->ops.queue_put != NULL))
  {
    rc = queue->ops.queue_put(queue, item);
  }

  return rc;
}

K_Status_e qReset(Queue_t queue, uint16_t * drops)
{
  K_Status_e rc = K_Status_General_Error;

  if ((queue != NULL) && (queue->ops.queue_reset != NULL))
  {
    rc = queue->ops.queue_reset(queue, drops);
  }

  return rc;
}

K_Status_e qCreateCommonQueueFields(struct _Queue * queue, QueuePolicy_e policy)
{
  K_Status_e rc = K_Status_General_Error;

  if (queue != NULL)
  {
    const IThread_t * const ti = getThreadIntf();
    queue->policy = policy;
    if ((queue->mutex = ti->createMutex()) != NULL)
    {
      if ((queue->full = ti->conditionCreate()) != NULL)
      {
        if ((queue->empty = ti->conditionCreate()) != NULL)
        {
          rc = K_Status_OK;
        }
      }
    }
  }

  return rc;
}

K_Status_e qUsage(Queue_t queue, uint32_t * usage, uint32_t * size)
{
  K_Status_e rc = K_Status_General_Error;

  if ((usage != NULL) && (size != NULL))
  {
    if ((queue != NULL) && (queue->ops.queue_getUsage != NULL))
    {
      rc = queue->ops.queue_getUsage(queue, usage, size);
    }
  }

  return rc;
}

void qDestroyCommonQueueFields(struct _Queue * queue)
{
  const IThread_t * const ti = getThreadIntf();

  if (ti->destroyMutex(queue->mutex) != K_Status_OK)
  {
    ERROR("Can not destroy queue mutex");
  }
  if (ti->conditionDestroy(queue->full) != K_Status_OK)
  {
    ERROR("Can not destroy overflow condition");
  }
  if (ti->conditionDestroy(queue->empty) != K_Status_OK)
  {
    ERROR("Can not destroy underflow condition");
  }
}
