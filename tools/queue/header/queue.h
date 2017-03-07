/*
 * queue.h
 *
 *  Created on: Jan 27, 2013
 *      Author: erik
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <common/k_types.h>
#include <queue/queueIntf.h>
#include <os/threadIntf.h>

struct QueueOps
{
  K_Status_e (* queue_destroy)(struct _Queue * impl);
  K_Status_e (* queue_get_blocking)(struct _Queue * impl, void * item);
  K_Status_e (* queue_get_non_blocking)(struct _Queue * impl, void * item);
  K_Status_e (* queue_put)(struct _Queue * impl, void * item);
  K_Status_e (* queue_reset)(struct _Queue * impl, uint16_t * drops);
  K_Status_e (* queue_getUsage)(struct _Queue * impl, uint32_t * usage, uint32_t * size);
};

struct _QueueNode
{
  void * data;
  struct _QueueNode * next;
};

struct _Queue
{
  /* Members explicitly for fixed size queues for */
  uint16_t size;
  QueueRetriever_t retriever;
  QueueStorer_t storer;
  void * storage;

  volatile uint16_t readPos;
  volatile uint16_t writePos;

  OsCondition_t full;

  /* Members for Dynamic Queues */
  volatile struct _QueueNode * first;
  volatile struct _QueueNode * last;
  ItemDestructor_t destroyItem;

  /* Common Members */

  volatile uint32_t usage;
  QueuePolicy_e policy;
  OsMutex_t mutex;
  OsCondition_t empty;

  struct QueueOps ops;
};

Queue_t qCreateFixedSizeQueue(QueuePolicy_e policy, uint16_t size, void * storage, QueueRetriever_t get, QueueStorer_t put);

Queue_t qCreateDynamicQueue(QueuePolicy_e policy, ItemDestructor_t destructor);
K_Status_e qCreateCommonQueueFields(struct _Queue * queue, QueuePolicy_e policy);

void qDestroyCommonQueueFields(Queue_t queue);

K_Status_e qDestroyQueue(Queue_t queue);
K_Status_e qGetItem(Queue_t queue, void * item);
K_Status_e qPutItem(Queue_t queue, void * item);
K_Status_e qReset(Queue_t queue, uint16_t * drops);
K_Status_e qUsage(Queue_t queue, uint32_t * usage, uint32_t * size);

#endif /* QUEUE_H_ */
