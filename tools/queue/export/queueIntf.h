/*
 * queueIntf.h
 *
 *  Created on: Jan 27, 2013
 *      Author: erik
 */

#ifndef QUEUEINTF_H_
#define QUEUEINTF_H_

#include <common/k_types.h>

/*!
 * \file
 * \brief Interface file for queues
 *
 * Queues are thread safe
 *
 * Important note. Although the interface seems uniform there is a hughe difference between dynamic
 * queues or fixed size queues.
 *
 * At creation time a fixed sized queue has all the memory allocated to store the items. The way the
 * items are store is opaque to the queue and is handled by the QueueRetriever_t and QueueStorer_t.
 * This means that a queue user can just pass in the address of a local variable to store or retrieve
 * queue items. The memory management is done by the storer and retriever of the queue.
 *
 * However dynamic queues do not have a fixed size limitations, and there is is no storer and retriever
 * function. The dynamic queue just handles serialized access. Therefor it is important to notice that
 * one should ALWAYS pass in heap memory to the store operation. This implies that for the retrieve
 * operation one MUST pass in an address of a pointer.
 *
 *
 * E.g
 *
 * The caller does something like this
 *
 * char_t *data = malloc(100);
 * queue->putItem(q, data);
 *
 *
 * The retriever will get the data as follows
 *
 * char_t *data = NULL
 * queue->getItem(q, &data);
 */

typedef enum
{
  Q_BLOCKING = 1,
  Q_NONBLOCKING = 2
}QueuePolicy_e;

typedef K_Status_e (* QueueRetriever_t)(uint16_t pos, void * const storage, void * const item);
typedef K_Status_e (* QueueStorer_t)(uint16_t, void * const storage, const void * const item);

typedef K_Status_e (* ItemDestructor_t)(void * item);

typedef struct _Queue * Queue_t;

typedef struct QueueIntf
{
  Queue_t (* createQueue)(QueuePolicy_e policy, uint16_t size, void * storage, QueueRetriever_t get, QueueStorer_t put);
  /*!
   * \ brief Creates a dynamic queue.
   *
   * As oposed to the fixed size queue a dynamic queue does not have a free allocated storage
   * for its items. Therefor we need a way to free the items that where dropped when calling
   * reset. This is done by the destroy parameter.
   *
   * It will point to a user defined function that knows how to release the resources of the
   * items in the queue.
   */

  Queue_t (* createDynamicQueue)(QueuePolicy_e policy, ItemDestructor_t destroy);

  /*!
   * \ brief Cleans all the resources allocated by a queue, apart from the queueStorage.
   *
   * As the queue storage is still owned by the caller of queue create, this function will not release
   * the storage of the queu, that is left over to the caller.
   *
   */

  K_Status_e (* destroyQueue)(Queue_t queue);

  /*!
   * \brief Puts an item on the back of the queue.
   *
   * If the queue was created with the policy Q_BLOCKING, than this call will block until an space
   * is available on the queue
   *
   * It the queue was created with the policy Q_NONBLOCKING and the queue is empty, the call will
   * return immediately and One_Failure qill be returned.
   *
   * If the item was successfully added to the queue One_Success is returned.
   */

  K_Status_e (* putItem)(Queue_t queue, void * item);

  /*!
   * \brief Returns the first item of the queue
   *
   * If the queue was created with the policy Q_BLOCKING, than this call will block until an item
   * arrives on the queue.
   *
   * It the queue was created with the policy Q_NONBLOCKING and the queue is empty, the call will
   * return immediately and No_ResultReturned wil be returned
   *
   * If something went wrong during the processing, One_Failure will be returned. If an item is
   * returned One_Success is returned.
   */

  K_Status_e (* getItem)(Queue_t queue, void * item);

  /*!
   * \brief Empties the queue and returns the number of items that where dropped
   *
   */

  K_Status_e (* reset)(Queue_t queue, uint16_t * drops);

  /*!
   * \brief Returns the number of items on the queue.
   *
   * In case of a fixed size queue the maximum queue size is also returned, for a dynamic queue size will be set to 0.
   *
   * Please note that this is just a snapshot, no locks will be taken on the queue to determine the exact numbers.
   *
   */

  K_Status_e (* getUsage)(Queue_t queue, uint32_t * usage, uint32_t * size);
} IQueue_t;

IQueue_t * getQueueIntf(void);

#endif /* QUEUEINTF_H_ */
