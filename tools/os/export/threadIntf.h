/*
 * threadIntf.h
 *
 *  Created on: Jan 5, 2017
 *      Author: erik
 */

#ifndef OS_EXPORT_THREADINTF_H_
#define OS_EXPORT_THREADINTF_H_

#include <common/k_types.h>

typedef void * (* THREAD_FUNCTION)(void * data);

typedef struct OsMutex * OsMutex_t;
typedef struct OsCondition * OsCondition_t;
typedef struct OsAtomicLock * OsAtomicLock_t;
typedef struct OsThread * OsThread_t;

typedef struct _thread_interface
{
  /*!
   * \brief Creates a new AtomicLock
   */

  OsAtomicLock_t (* createALock)(void);

  /*!
   * \brief Destroys an OsAtomicLock_t previously created with createALock
   */

  K_Status_e (* destroyALock)(OsAtomicLock_t lock);

  K_Status_e (* lockAtomic)(OsAtomicLock_t lock);

  K_Status_e (* unlockAtomic)(OsAtomicLock_t lock);

  K_Status_e (* trylockAtomic)(OsAtomicLock_t lock);

  /*!
   * \brief Creates a Mutex
   */

  OsMutex_t (* createMutex)(void);

  /*!
   * \brief Destroys a mutex previously created with #createMutex
   *
   */

  K_Status_e (* destroyMutex)(OsMutex_t mutex);

  K_Status_e (* lockMutex)(OsMutex_t mutex);

  K_Status_e (* unlockMutex)(OsMutex_t mutex);

  K_Status_e (* trylockMutex)(OsMutex_t mutex);

  OsCondition_t (* conditionCreate)(void);

  K_Status_e (* conditionDestroy)(OsCondition_t c);

  K_Status_e (* conditionWait)(OsCondition_t c, OsMutex_t m);

  K_Status_e (* conditionSignal)(OsCondition_t c);

  OsThread_t (* createThread)(THREAD_FUNCTION f, void * userData);

  K_Status_e (* destroyThread)(OsThread_t thread);

  K_Status_e (* joinThread)(OsThread_t thread);

  K_Status_e (* yieldThread)(void);

} IThread_t;

IThread_t * getThreadIntf(void);

#endif /* OS_EXPORT_THREADINTF_H_ */
