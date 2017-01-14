/*
 * thread_impl.h
 *
 *  Created on: Jan 5, 2017
 *      Author: erik
 */

#ifndef OS_HEADER_THREAD_IMPL_H_
#define OS_HEADER_THREAD_IMPL_H_

#include <os/threadIntf.h>
#include <pthread.h>

#define THREAD_LOCK_AVAILABLE  0xDEADBEEF
#define THREAD_LOCK_BUSY       0xBEEFDEAD

struct OsMutex
{
  OsAtomicLock_t guard;
  pthread_mutex_t mutex;
};

struct OsAtomicLock
{
  volatile uint32_t isBusy;
};

OsMutex_t thread_createMutex(void);
K_Status_e thread_destroyMutex(OsMutex_t mutex);
K_Status_e thread_lockMutex(OsMutex_t mutex);
K_Status_e thread_unlockMutex(OsMutex_t mutex);
K_Status_e thread_trylockMutex(OsMutex_t mutex);

OsAtomicLock_t thread_createAtomicLock(void);
K_Status_e thread_destroyAtomicLock(OsAtomicLock_t lock);

K_Status_e thread_lockAtomicLock(OsAtomicLock_t lock);
K_Status_e thread_unlockAtomicLock(OsAtomicLock_t lock);
K_Status_e thread_trylockAtomicLock(OsAtomicLock_t lock);

#endif /* OS_HEADER_THREAD_IMPL_H_ */
