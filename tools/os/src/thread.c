/*
 * thread.c
 *
 *  Created on: Jan 5, 2017
 *      Author: erik
 */

#include "thread_impl.h"

#include <os/memIntf.h>

#include <pthread.h>
#include <errno.h>
#include <signal.h>

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

static K_Status_e localMarkLockBusy(OsAtomicLock_t lock);
static K_Status_e localMarkLockFree(OsAtomicLock_t lock);

OsMutex_t thread_createMutex(void)
{
  const IMem_t * const mem = getMemIntf();
  struct OsMutex * newMutex = (struct OsMutex *) mem->malloc(sizeof(struct OsMutex));

  if (newMutex != NULL)
  {
    mem->memset(newMutex, 0, sizeof(struct OsMutex));
    newMutex->guard = thread_createAtomicLock();

    if (newMutex->guard != NULL)
    {
      if (pthread_mutex_init(&newMutex->mutex, NULL) != 0)
      {
        thread_destroyMutex(newMutex);
        newMutex = NULL;
      }
    }
    else
    {
      thread_destroyMutex(newMutex);
      newMutex = NULL;
    }
  }

  return newMutex;
}

K_Status_e thread_destroyMutex(OsMutex_t mutex)
{
  K_Status_e rc = K_Status_Invalid_Param;

  if (mutex != NULL)
  {
    if (localMarkLockBusy(mutex->guard) == K_Status_OK)
    {
      const IMem_t * const mem = getMemIntf();
      OsAtomicLock_t guard = mutex->guard;
      mutex->guard = NULL;

      localMarkLockFree(guard);
      thread_destroyAtomicLock(guard);
      mem->memset(mutex, 0, sizeof(struct OsMutex));
      mem->free(mutex);

      rc = K_Status_OK;
    }
  }

  return rc;
}

OsAtomicLock_t thread_createAtomicLock(void)
{
  const IMem_t * const mem = getMemIntf();
  struct OsAtomicLock * newLock = (struct OsAtomicLock *) mem->malloc(sizeof(struct OsAtomicLock));

  if (newLock != NULL)
  {
    newLock->isBusy = THREAD_LOCK_AVAILABLE;
  }

  return newLock;
}

K_Status_e thread_destroyAtomicLock(OsAtomicLock_t lock)
{
  K_Status_e rc = K_Status_General_Error;

  if (lock != NULL)
  {
    if (localMarkLockBusy(lock) == K_Status_OK)
    {
      const IMem_t * const mem = getMemIntf();
      mem->free(lock);
      rc = K_Status_OK;
    }
  }

  return rc;
}

static K_Status_e localMarkLockBusy(OsAtomicLock_t lock)
{
  K_Status_e rc = K_Status_Locked;

  if (lock != NULL)
  {
    if (__sync_val_compare_and_swap(&lock->isBusy, THREAD_LOCK_AVAILABLE, THREAD_LOCK_BUSY) == THREAD_LOCK_AVAILABLE)
    {
      rc = K_Status_OK;
    }
  }
  return rc;

}

static K_Status_e localMarkLockFree(OsAtomicLock_t lock)
{
  K_Status_e rc = K_Status_Locked;

  if (lock != NULL)
  {
    if (__sync_val_compare_and_swap(&lock->isBusy, THREAD_LOCK_BUSY, THREAD_LOCK_AVAILABLE) == THREAD_LOCK_BUSY)
    {
      rc = K_Status_OK;
    }
  }
  return rc;
}
