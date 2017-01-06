/*
 * thread_intf.c
 *
 *  Created on: Jan 5, 2017
 *      Author: erik
 */

#include "thread_impl.h"

static IThread_t localIntf =
{
#ifndef UNITTESTS
    .createMutex  = thread_createMutex,
    .destroyMutex = thread_destroyMutex,
#if 0
    .lock         = thread_lock,
    .unlock       = thread_unlock,
    .tryLock      = thread_tryLock,
#endif
#endif
};

IThread_t *getThreadIntf(void)
{
  return &localIntf;
}
