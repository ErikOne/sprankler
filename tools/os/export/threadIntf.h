/*
 * threadIntf.h
 *
 *  Created on: Jan 5, 2017
 *      Author: erik
 */

#ifndef OS_EXPORT_THREADINTF_H_
#define OS_EXPORT_THREADINTF_H_

#include <common/k_types.h>

typedef struct OsMutex *OsMutex_t;
typedef struct OsAtomicLock *OsAtomicLock_t;

typedef struct _thread_interface
{
  /*!
   * \brief Verifies atomically if the given memory address holds the expectedValue.
   *
   * \return this function will return Status_OK
   *
   */

  K_Status_e (*atomicCheck)(K_Boolean_e *ptr, K_Boolean_e expectedValue);





  /*!
   * \brief Creates a Mutex
   */

  OsMutex_t (*createMutex)(void);

  /*!
   * \brief Destroys a mutex previously created with #createMutex
   *
   */

  K_Status_e (*destroyMutex)(OsMutex_t mutex);

  K_Status_e (*lock)(OsMutex_t mutex);

  K_Status_e (*unlock)(OsMutex_t mutex);

  K_Status_e (*tryLock)(OsMutex_t mutex);


} IThread_t;

IThread_t *getThreadIntf(void);

#endif /* OS_EXPORT_THREADINTF_H_ */
