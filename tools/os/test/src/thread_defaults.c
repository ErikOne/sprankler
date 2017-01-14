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

}

void unittest_uninstallDefaultThreadIntf()
{
  IThread_t * const intf = getThreadIntf();

  intf->createALock = NULL;
  intf->destroyALock = NULL;
  intf->lockAtomic = NULL;
  intf->unlockAtomic = NULL;
  intf->trylockAtomic = NULL;

}

#endif
