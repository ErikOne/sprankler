/*
 * thread.c
 *
 *  Created on: Jan 5, 2017
 *      Author: erik
 */

#include "thread_impl.h"

#include <os/memIntf.h>
#include <logging/logging.h>

#include <errno.h>
#include <signal.h>

static K_Status_e localMarkLockBusy(OsAtomicLock_t lock);
static K_Status_e localMarkLockAvailable(OsAtomicLock_t lock);
static void localFreeGuard(struct OsAtomicLock * alock);
static K_Status_e localJoinThread(struct OsThread * impl);
static void * localStartThreadWithBlockedSignals(void * data);

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
    rc = localMarkLockBusy(mutex->guard);
    if (rc == K_Status_OK)
    {
      const IMem_t * const mem = getMemIntf();
      OsAtomicLock_t guard = mutex->guard;
      mutex->guard = NULL;

      localFreeGuard(guard);
      mem->memset(mutex, 0, sizeof(struct OsMutex));
      mem->free(mutex);

      rc = K_Status_OK;
    }
  }

  return rc;
}

K_Status_e thread_lockMutex(OsMutex_t mutex)
{
  K_Status_e rc = K_Status_Invalid_Param;

  if (mutex != NULL)
  {
    struct OsMutex * impl = mutex;
    rc = (pthread_mutex_lock(&impl->mutex) == 0)?K_Status_OK:K_Status_General_Error;
  }
  return rc;
}

K_Status_e thread_unlockMutex(OsMutex_t mutex)
{
  K_Status_e rc = K_Status_Invalid_Param;

  if (mutex != NULL)
  {
    struct OsMutex * impl = mutex;
    rc = (pthread_mutex_unlock(&impl->mutex) == 0)?K_Status_OK:K_Status_General_Error;
  }
  return rc;
}

K_Status_e thread_trylockMutex(OsMutex_t mutex)
{
  K_Status_e rc = K_Status_Invalid_Param;

  if (mutex != NULL)
  {
    struct OsMutex * impl = mutex;
    rc = (pthread_mutex_trylock(&impl->mutex) == 0)?K_Status_OK:K_Status_Unexpected_State;
  }

  return rc;
}

K_Status_e thread_lockAtomicLock(OsAtomicLock_t lock)
{
  K_Status_e rc = K_Status_Invalid_Param;

  if (lock != NULL)
  {
    while (rc != K_Status_OK)
    {
      rc = thread_trylockAtomicLock(lock);
    }
  }

  return rc;
}

K_Status_e thread_unlockAtomicLock(OsAtomicLock_t lock)
{
  K_Status_e rc = K_Status_Invalid_Param;

  if (lock != NULL)
  {
    rc = localMarkLockAvailable(lock);
  }

  return rc;
}

K_Status_e thread_trylockAtomicLock(OsAtomicLock_t lock)
{
  K_Status_e rc = K_Status_Invalid_Param;

  if (lock != NULL)
  {
    rc = localMarkLockBusy(lock);
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
  K_Status_e rc = K_Status_Invalid_Param;

  if (lock != NULL)
  {
    if (localMarkLockBusy(lock) == K_Status_OK)
    {
      localFreeGuard(lock);
      rc = K_Status_OK;
    }
  }

  return rc;
}

OsCondition_t thread_conditionCreate(void)
{
  OsCondition_t newCondition = NULL;
  const IMem_t * const mem = getMemIntf();

  newCondition = (OsCondition_t) mem->malloc(sizeof(struct OsCondition));
  if (newCondition != NULL)
  {
    mem->memset(newCondition, 0, sizeof(struct OsCondition));
    if (pthread_cond_init(&newCondition->condition, NULL) != 0)
    {
      mem->free(newCondition);
      newCondition = NULL;
    }
  }

  return newCondition;
}

K_Status_e thread_conditionDestroy(OsCondition_t c)
{
  K_Status_e rc = K_Status_General_Error;

  if (c != NULL)
  {
    const IMem_t * const mem = getMemIntf();
    pthread_cond_destroy(&c->condition);

    mem->free(c);

    rc = K_Status_OK;
  }

  return rc;
}

K_Status_e thread_conditionWait(OsCondition_t c, OsMutex_t m)
{
  K_Status_e rc = K_Status_General_Error;

  if ((c != NULL) && (m != NULL))
  {
    if (pthread_cond_wait(&c->condition, &m->mutex) == 0)
    {
      rc = K_Status_OK;
    }
  }
  return rc;
}

K_Status_e thread_conditionSignal(OsCondition_t c)
{
  K_Status_e rc = K_Status_General_Error;

  if (c != NULL)
  {
    if (pthread_cond_signal(&c->condition) == 0)
    {
      rc = K_Status_OK;
    }
  }

  return rc;
}

OsThread_t thread_createThread(THREAD_FUNCTION f, void * data)
{
  struct OsThread * newThread = NULL;

  if (f != NULL)
  {
    const IMem_t * const mem = getMemIntf();

    newThread = (struct OsThread *) mem->malloc(sizeof(struct OsThread));
    if (newThread != NULL)
    {
      mem->memset(newThread, 0, sizeof(struct OsThread));
      newThread->guard = thread_createAtomicLock();

      if (newThread->guard != NULL)
      {
        newThread->function = f;
        newThread->userData = data;

        if (pthread_create(&newThread->thread, NULL, localStartThreadWithBlockedSignals, newThread) != 0)
        {
          thread_destroyThread(newThread);
          newThread = NULL;
        }
      }
      else
      {
        mem->free(newThread);
        newThread = NULL;
      }
    }
  }

  return newThread;
}

K_Status_e thread_destroyThread(OsThread_t t)
{
  K_Status_e rc = K_Status_Invalid_Param;

  struct OsThread * impl = (struct OsThread *) t;
  if (impl != NULL)
  {
    rc = localMarkLockBusy(impl->guard);
    if (rc == K_Status_OK)
    {
      const IMem_t * const mem = getMemIntf();
      if (localJoinThread(impl) == K_Status_OK)
      {
        rc = K_Status_OK;
      }

      OsAtomicLock_t guard = impl->guard;
      impl->guard = NULL;

      localFreeGuard(guard);
      mem->free(impl);
    }
  }
  return rc;
}

K_Status_e thread_joinThread(OsThread_t thread)
{
  K_Status_e rc = K_Status_Invalid_Param;

  struct OsThread * impl = thread;
  if (impl != NULL)
  {
    rc = localMarkLockBusy(impl->guard);

    if (rc == K_Status_OK)
    {
      if (pthread_join(impl->thread, NULL) == 0)
      {
        rc = K_Status_OK;
      }
      localMarkLockAvailable(impl->guard);
    }
  }

  return rc;
}

K_Status_e thread_yieldThread(void)
{
  return (sched_yield() == 0)?K_Status_OK:K_Status_General_Error;
}

static K_Status_e localMarkLockBusy(OsAtomicLock_t lock)
{
  K_Status_e rc = K_Status_Invalid_Param;

  if (lock != NULL)
  {
    int value = __sync_val_compare_and_swap(&lock->isBusy, THREAD_LOCK_AVAILABLE, THREAD_LOCK_BUSY);
    switch (value)
    {
      case THREAD_LOCK_AVAILABLE:
        rc = K_Status_OK;
        break;
      case THREAD_LOCK_BUSY:
        rc = K_Status_Locked;
        break;
      default:
        rc = K_Status_General_Error;
        break;
    }
  }
  return rc;
}

K_Boolean_e thread_itsMe(OsThread_t thread)
{
  K_Boolean_e itsMe = K_False;
  if (thread != NULL)
  {
    if (pthread_self() == thread->thread)
    {
      itsMe = K_True;
    }
  }

  return itsMe;
}

static K_Status_e localMarkLockAvailable(OsAtomicLock_t lock)
{
  K_Status_e rc = K_Status_Invalid_Param;

  if (lock != NULL)
  {
    int value = __sync_val_compare_and_swap(&lock->isBusy, THREAD_LOCK_BUSY, THREAD_LOCK_AVAILABLE);
    switch (value)
    {
      case THREAD_LOCK_BUSY:
        rc = K_Status_OK;
        break;
      case THREAD_LOCK_AVAILABLE:
        rc = K_Status_Unexpected_State;
        break;
      default:
        rc = K_Status_General_Error;
        break;
    }
  }
  return rc;
}

static K_Status_e localJoinThread(struct OsThread * impl)
{
  return (pthread_join(impl->thread, NULL) == 0)?K_Status_OK:K_Status_General_Error;
}

static void localFreeGuard(struct OsAtomicLock * alock)
{
  const IMem_t * const mem = getMemIntf();
  mem->free(alock);
}

static void * localStartThreadWithBlockedSignals(void * data)
{
  struct OsThread * impl = (struct OsThread *) data;
  void * result = NULL;

  if ((impl != NULL) && (impl->function != NULL))
  {
    sigset_t set;
    if (sigfillset(&set) == 0)
    {
      if (pthread_sigmask(SIG_SETMASK, &set, NULL) == 0)
      {
        result = impl->function(impl->userData);
      }
    }
  }

  return result;
}
