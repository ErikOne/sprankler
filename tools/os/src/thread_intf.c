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
  .alockCreate      = thread_createAtomicLock,
  .alockDestroy     = thread_destroyAtomicLock,
  .alockLock        = thread_lockAtomicLock,
  .alockUnlock      = thread_unlockAtomicLock,
  .alockTrylock     = thread_trylockAtomicLock,

  .mutexCreate      = thread_createMutex,
  .mutexDestroy     = thread_destroyMutex,
  .mutexLock        = thread_lockMutex,
  .mutexUnlock      = thread_unlockMutex,
  .mutexTrylock     = thread_trylockMutex,

  .conditionCreate  = thread_conditionCreate,
  .conditionDestroy = thread_conditionDestroy,
  .conditionWait    = thread_conditionWait,
  .conditionSignal  = thread_conditionSignal,

  .threadCreate     = thread_createThread,
  .threadDestroy    = thread_destroyThread,
  .threadJoin       = thread_joinThread,
  .threadYield      = thread_yieldThread,
  .itsMe            = thread_itsMe,

#endif
};

IThread_t * getThreadIntf(void)
{
  return &localIntf;
}
