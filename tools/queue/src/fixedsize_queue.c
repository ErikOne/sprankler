/*
 * queue.c
 *
 *  Created on: Jan 28, 2013
 *      Author: erik
 */
#include <logging/logging.h>

#include <common/k_types.h>
#include <queue/queueIntf.h>
#include <os/memIntf.h>
#include <os/threadIntf.h>

#include "queue.h"

static K_Status_e localCheckParameters(QueuePolicy_e policy, uint16_t size, void * storage, QueueRetriever_t get, QueueStorer_t put);
static K_Status_e localDoBlockingRead(struct _Queue * q, void * item);
static K_Status_e localDoNonblockingRead(struct _Queue * q, void * item);

static K_Status_e localFixedSizeQueueDestroyQueue(struct _Queue * fsq);
static K_Status_e localFixedSizeQueuePutItem(struct _Queue * fsq, void * item);
static K_Status_e localFixedSizeQueueResetQueue(struct _Queue * fsq, uint16_t * droppedPackets);
static K_Status_e localFixedQueueUsage(struct _Queue * impl, uint32_t * usage, uint32_t * maxSize);

Queue_t qCreateFixedSizeQueue(QueuePolicy_e policy, uint16_t size, void * storage, QueueRetriever_t get, QueueStorer_t put)
{
  Queue_t newQueue = NULL;

  if (localCheckParameters(policy, size, storage, get, put) == K_Status_OK)
  {
    const IMem_t * const mem = getMemIntf();
    newQueue = (struct _Queue *) mem->malloc(sizeof(struct _Queue));
    if (newQueue != NULL)
    {
      mem->memset(newQueue, 0, sizeof(struct _Queue));
      if (qCreateCommonQueueFields(newQueue, policy) == K_Status_OK)
      {
        newQueue->readPos = 0;
        newQueue->writePos = 0;
        newQueue->usage = 0;
        newQueue->size = size;
        newQueue->storage = storage;
        newQueue->retriever = get;
        newQueue->storer = put;

        newQueue->ops.queue_destroy = &localFixedSizeQueueDestroyQueue;
        newQueue->ops.queue_get_non_blocking = &localDoNonblockingRead;
        newQueue->ops.queue_get_blocking = &localDoBlockingRead;
        newQueue->ops.queue_put = &localFixedSizeQueuePutItem;
        newQueue->ops.queue_reset = &localFixedSizeQueueResetQueue;
        newQueue->ops.queue_getUsage = &localFixedQueueUsage;
      }
      else
      {
        (void)localFixedSizeQueueDestroyQueue(newQueue);
        newQueue = NULL;
      }
    }
  }
  return newQueue;
}

static K_Status_e localFixedSizeQueueDestroyQueue(struct _Queue * fsq)
{
  K_Status_e rc = K_Status_General_Error;

  if (fsq != NULL)
  {
    const IMem_t * const mem = getMemIntf();
    qDestroyCommonQueueFields(fsq);
    /* Put everything back to 0 */
    mem->free(fsq);
    rc = K_Status_OK;
  }

  return rc;
}

static K_Status_e localFixedSizeQueuePutItem(struct _Queue * fsq, void * item)
{
  K_Status_e rc = K_Status_General_Error;

  if ((fsq != NULL) && (item != NULL))
  {
    const IThread_t * const ti = getThreadIntf();

    if (ti->mutexLock(fsq->mutex) == K_Status_OK)
    {
      uint16_t next = (fsq->writePos + 1 + fsq->size - fsq->readPos) % fsq->size;
      /* The queue is full, wait for it to get space again */
      while (next == 0)
      {
        ti->conditionWait(fsq->full, fsq->mutex);
        next = (fsq->writePos + 1 + fsq->size - fsq->readPos) % fsq->size;
      }

      rc = fsq->storer(fsq->writePos, fsq->storage, item);
      if (rc == K_Status_OK)
      {
        fsq->usage++;
        fsq->writePos = (uint16_t) (fsq->writePos + 1) % fsq->size;
      }

      (void) ti->mutexUnlock(fsq->mutex);

      /* As the reader might be blocked because of empty inform him about it */
      (void) ti->conditionSignal(fsq->empty);

    }
    else
    {
      ERROR("Can not take lock on queue\n");
    }
  }

  return rc;
}

static K_Status_e localCheckParameters(QueuePolicy_e policy, uint16_t size, void * storage, QueueRetriever_t get, QueueStorer_t put)
{
  K_Status_e rc = K_Status_General_Error;

  if (size < 2)
  {
    WARNING("Queue size must be minimum 2\n");
  }
  else if (storage == NULL)
  {
    WARNING("No storage defined\n");
  }
  else if (get == NULL)
  {
    WARNING("No function speficfied to retrieve items from the queue\n");
  }
  else if (put == NULL)
  {
    WARNING("No function speficfied to store items in the queue\n");
  }
  else
  {
    rc = K_Status_OK;
  }

  return rc;
}

static K_Status_e localDoBlockingRead(struct _Queue * q, void * item)
{
  K_Status_e rc = K_Status_General_Error;
  const IThread_t * const ti = getThreadIntf();

  if (ti->mutexLock(q->mutex) == K_Status_OK)
  {
    while (q->readPos == q->writePos)
    {
      ti->conditionWait(q->empty, q->mutex);
    }

    rc = q->retriever(q->readPos, q->storage, item);
    if (rc == K_Status_OK)
    {
      q->usage--;
      q->readPos = (q->readPos + 1) % q->size;
    }

    (void) ti->mutexUnlock(q->mutex);

    /* As the writer might be blocked because of overflow inform him about it */
    rc = ti->conditionSignal(q->full);
  }
  return rc;
}

/* In case of an empty queue I immediately return with K_Status_General_Error */

static K_Status_e localDoNonblockingRead(struct _Queue * q, void * item)
{
  K_Status_e rc = K_Status_General_Error;
  const IThread_t * const ti = getThreadIntf();

  if (ti->mutexLock(q->mutex) == K_Status_OK)
  {
    if (q->readPos != q->writePos)
    {
      rc = q->retriever(q->readPos, q->storage, item);
      if (rc == K_Status_OK)
      {
        q->readPos = (q->readPos + 1) % q->size;
        q->usage--;
      }
      (void) ti->mutexUnlock(q->mutex);
      rc = ti->conditionSignal(q->empty);
    }
    else
    {
      rc = K_Status_NoResult;
      (void) ti->mutexUnlock(q->mutex);
    }
  }

  return rc;
}

static K_Status_e localFixedSizeQueueResetQueue(struct _Queue * impl, uint16_t * droppedPackets)
{
  K_Status_e rc = K_Status_General_Error;
  const IThread_t * const ti = getThreadIntf();

  if ((impl != NULL) && (droppedPackets != NULL))
  {
    if (ti->mutexLock(impl->mutex) == K_Status_OK)
    {
      *droppedPackets = impl->usage;

      impl->readPos = 0;
      impl->writePos = 0;
      impl->usage = 0;

      (void) ti->mutexUnlock(impl->mutex);
      rc = ti->conditionSignal(impl->full);
    }
  }

  return rc;
}

static K_Status_e localFixedQueueUsage(struct _Queue * impl, uint32_t * usage, uint32_t * maxSize)
{
  K_Status_e rc = K_Status_General_Error;

  if ((impl != NULL) && (usage != NULL) && (maxSize != NULL))
  {
    *maxSize = impl->size;
    *usage = impl->usage;

    rc = K_Status_OK;
  }

  return rc;
}
