/*
 * thread_defaults.c
 *
 *  Created on: Jan 7, 2017
 *      Author: erik
 */

#include <ostests/unittestOsDefaults.h>

#include "thread_impl.h"

#ifdef UNITTESTS

void unittest_installDefaultThreadIntf()
{
  IThread_t * const intf = getThreadIntf();

  intf->createALock = thread_createAtomicLock;
  intf->destroyALock = thread_destroyAtomicLock;
  intf->lockAtomic = thread_lockAtomicLock;
  intf->unlockAtomic = thread_unlockAtomicLock;
  intf->trylockAtomic = thread_trylockAtomicLock;
  intf->createMutex = thread_createMutex;
  intf->destroyMutex = thread_destroyMutex;
  intf->lockMutex = thread_lockMutex;
  intf->unlockMutex = thread_unlockMutex;
  intf->trylockMutex = thread_trylockMutex;
  intf->createThread = thread_createThread;
  intf->destroyThread = thread_destroyThread;
  intf->joinThread = thread_joinThread;
  intf->yieldThread = thread_yieldThread;
}

void unittest_uninstallDefaultThreadIntf()
{
  IThread_t * const intf = getThreadIntf();

  intf->createALock = NULL;
  intf->destroyALock = NULL;
  intf->lockAtomic = NULL;
  intf->unlockAtomic = NULL;
  intf->trylockAtomic = NULL;
  intf->createMutex = NULL;
  intf->destroyMutex = NULL;
  intf->lockMutex = NULL;
  intf->unlockMutex = NULL;
  intf->trylockMutex = NULL;

  intf->createThread = NULL;
  intf->destroyThread = NULL;
  intf->joinThread = NULL;
  intf->yieldThread = NULL;

}

#endif
