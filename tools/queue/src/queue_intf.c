/*
 * queue_intf.c
 *
 *  Created on: Jan 27, 2013
 *      Author: erik
 */

#include <common/k_types.h>
#include <queue/queueIntf.h>

#include "queue.h"

static IQueue_t localQueueImpl =
{
#ifndef UNITTESTS
  qCreateFixedSizeQueue,
  qCreateDynamicQueue,
  qDestroyQueue,
  qPutItem,
  qGetItem,
  qReset,
  qUsage,
#endif
};

IQueue_t * getQueueIntf(void)
{
  return &localQueueImpl;
}
