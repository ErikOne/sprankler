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
  .createALock   = thread_createAtomicLock,
  .destroyALock  = thread_destroyAtomicLock,
  .lockAtomic    = thread_lockAtomicLock,
  .unlockAtomic  = thread_unlockAtomicLock,
  .trylockAtomic = thread_trylockAtomicLock,

  .createMutex   = thread_createMutex,
  .destroyMutex  = thread_destroyMutex,
  .lockMutex     = thread_lockMutex,
  .unlockMutex   = thread_unlockMutex,
  .trylockMutex  = thread_tryLockMutex,
#endif
};

IThread_t * getThreadIntf(void)
{
  return &localIntf;
}
