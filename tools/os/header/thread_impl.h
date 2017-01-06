/*
 * thread_impl.h
 *
 *  Created on: Jan 5, 2017
 *      Author: erik
 */

#ifndef OS_HEADER_THREAD_IMPL_H_
#define OS_HEADER_THREAD_IMPL_H_

#include <os/threadIntf.h>

OsMutex_t thread_createMutex(void);
K_Status_e thread_destroyMutex(OsMutex_t mutex);
K_Status_e thread_lockMutex(OsMutex_t mutex);
K_Status_e thread_unlockMutex(OsMutex_t mutex);
K_Status_e thread_tryLockMutex(OsMutex_t mutex);

OsAtomicLock_t thread_createAtomicLock(void);
K_Status_e thread_destroyAtomicLock(OsAtomicLock_t lock);

K_Status_e thread_atomiclockLock(OsAtomicLock_t lock);
K_Status_e thread_atomiclockUnlock(OsAtomicLock_t lock);
K_Status_e thread_atomiclockTryLock(OsAtomicLock_t lock);


#endif /* OS_HEADER_THREAD_IMPL_H_ */
