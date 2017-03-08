/*
 * dynamic_queue.c
 *
 *  Created on: Jan 9, 2014
 *      Author: erik
 */

#include <common/k_types.h>
#include <queue/queueIntf.h>
#include <os/memIntf.h>
#include <logging/logging.h>

#include "queue.h"

static K_Status_e localDynamicQueueDestroyQueue(struct _Queue * dynq);
static K_Status_e localDynamicQueueBlockingGet(struct _Queue * dynq, void * item);
static K_Status_e localDynamicQueueNonBlockingGet(struct _Queue * dynq, void * item);
static K_Status_e localDynamicQueuePutItem(struct _Queue * dynq, void * item);
static K_Status_e localDynamicQueueResetQueue(struct _Queue * dynq, uint16_t * droppedPackets);
static K_Status_e localDynamicQueueUsage(struct _Queue * dynq, uint32_t * usage, uint32_t * maxSize);

Queue_t qCreateDynamicQueue(QueuePolicy_e policy, ItemDestructor_t destructor)
{
  Queue_t newQueue = NULL;

  const IMem_t * const mem = getMemIntf();
  newQueue = (struct _Queue *) mem->malloc(sizeof(struct _Queue));
  if (newQueue != NULL)
  {
    mem->memset(newQueue, 0, sizeof(struct _Queue));
    if (qCreateCommonQueueFields(newQueue, policy) == K_Status_OK)
    {
      newQueue->ops.queue_destroy = &localDynamicQueueDestroyQueue;
      newQueue->ops.queue_get_blocking = &localDynamicQueueBlockingGet;
      newQueue->ops.queue_get_non_blocking = &localDynamicQueueNonBlockingGet;
      newQueue->ops.queue_put = &localDynamicQueuePutItem;
      newQueue->ops.queue_reset = &localDynamicQueueResetQueue;
      newQueue->ops.queue_getUsage = localDynamicQueueUsage;

      newQueue->destroyItem = destructor;
    }
    else
    {
      localDynamicQueueDestroyQueue(newQueue);
      newQueue = NULL;
    }
  }

  return newQueue;
}

static K_Status_e localDynamicQueueDestroyQueue(struct _Queue * dynq)
{
  K_Status_e rc = K_Status_General_Error;

  if (dynq != NULL)
  {
    const IMem_t * const mem = getMemIntf();
    uint16_t droppedItems;

    if (localDynamicQueueResetQueue(dynq, &droppedItems) == K_Status_OK)
    {
      TRACE("Successfully cleared queue. %u items dropped\n", droppedItems);
    }
    qDestroyCommonQueueFields(dynq);
    mem->memset(dynq, 0, sizeof(struct _Queue));
    mem->free(dynq);
    rc = K_Status_OK;
  }

  return rc;
}

static K_Status_e localDynamicQueueBlockingGet(struct _Queue * dynq, void * item)
{
  K_Status_e rc = K_Status_General_Error;

  if ((dynq != NULL) && (item != NULL))
  {
    const IThread_t * const ti = getThreadIntf();
    const IMem_t * const mem = getMemIntf();

    /* In a dynamic queue we store pointers to stuff. However in order to not break the interface with
     * a fixedsize queue the caller I used just a void * but the caller should always pass in the
     * address of a pointer
     */

    void ** data = (void **) item;

    if (ti->mutexLock(dynq->mutex) == K_Status_OK)
    {
      while (dynq->usage == 0)
      {
        ti->conditionWait(dynq->empty, dynq->mutex);
      }

      struct _QueueNode * first = (struct _QueueNode *) dynq->first;

      if (first != NULL)
      {
        *data = first->data;
        dynq->first = first->next;

        if (dynq->first == NULL)
        {
          dynq->last = NULL;
        }

        dynq->usage--;

        mem->free(first);
        rc = K_Status_OK;
      }

      (void) ti->mutexUnlock(dynq->mutex);
    }
  }

  return rc;
}

static K_Status_e localDynamicQueueNonBlockingGet(struct _Queue * dynq, void * item)
{
  K_Status_e rc = K_Status_General_Error;

  if ((dynq != NULL) && (item != NULL))
  {
    const IThread_t * const ti = getThreadIntf();
    const IMem_t * const mem = getMemIntf();

    /* In a dynamic queue we store pointers to stuff. However in order to not break the interface with
     * a fixedsize queue the caller I used just a void * but the caller should always pass in the
     * address of a pointer
     */

    void ** data = (void **) item;

    if (ti->mutexLock(dynq->mutex) == K_Status_OK)
    {
      if (dynq->first != NULL)
      {
        struct _QueueNode * first = (struct _QueueNode *) dynq->first;

        if (first != NULL)
        {
          *data = first->data;
          dynq->first = first->next;

          if (dynq->first == NULL)
          {
            dynq->last = NULL;
          }

          dynq->usage--;

          mem->free(first);

          rc = K_Status_OK;
        }
      }
      else
      {
        rc = K_Status_NoResult;
      }

      (void) ti->mutexUnlock(dynq->mutex);
    }
  }

  return rc;
}

static K_Status_e localDynamicQueuePutItem(struct _Queue * dynq, void * item)
{
  K_Status_e rc = K_Status_General_Error;

  if ((dynq != NULL) && (item != NULL))
  {
    const IThread_t * const ti = getThreadIntf();
    const IMem_t * const mem = getMemIntf();

    struct _QueueNode * newNode = (struct _QueueNode *) mem->malloc(sizeof(struct _QueueNode));
    if (newNode != NULL)
    {
      newNode->data = item;
      newNode->next = NULL;

      if (ti->mutexLock(dynq->mutex) == K_Status_OK)
      {
        dynq->usage++;
        if ((dynq->last == NULL) && (dynq->first == NULL))
        {
          dynq->first = newNode;
          dynq->last = newNode;

          rc = K_Status_OK;
        }
        else if ((dynq->last != NULL) && (dynq->first != NULL))
        {
          dynq->last->next = newNode;
          dynq->last = newNode;

          rc = K_Status_OK;
        }
        else
        {
          FATAL("Corrupt QUEUE\n");
          abort();
        }

        (void) ti->mutexUnlock(dynq->mutex);

        /* As the reader might be blocked because of empty inform him about it */
        (void) ti->conditionSignal(dynq->empty);
      }
      else
      {
        ERROR("Can not take lock on queue\n");
      }
    }
  }

  return rc;
}

static K_Status_e localDynamicQueueResetQueue(struct _Queue * dynq, uint16_t * droppedPackets)
{
  K_Status_e rc = K_Status_General_Error;

  if ((dynq != NULL) && (droppedPackets != NULL))
  {
    const IThread_t * const ti = getThreadIntf();
    const IMem_t * const mem = getMemIntf();

    if (ti->mutexLock(dynq->mutex) == K_Status_OK)
    {
      *droppedPackets = dynq->usage;

      dynq->usage = 0;

      struct _QueueNode * node = (struct _QueueNode *) dynq->first;
      while (node != NULL)
      {
        dynq->first = node->next;
        if (dynq->destroyItem != NULL)
        {
          dynq->destroyItem(node->data);
        }
        mem->free(node);
        node = (struct _QueueNode *) dynq->first;
      }

      dynq->last = NULL;

      rc = K_Status_OK;
      (void) ti->mutexUnlock(dynq->mutex);
    }
  }

  return rc;
}

static K_Status_e localDynamicQueueUsage(struct _Queue * dynq, uint32_t * usage, uint32_t * maxSize)
{
  K_Status_e rc = K_Status_General_Error;

  if ((dynq != NULL) && (usage != NULL) && (maxSize != NULL))
  {
    *usage = dynq->usage;
    *maxSize = dynq->size;

    rc = K_Status_OK;
  }

  return rc;
}
