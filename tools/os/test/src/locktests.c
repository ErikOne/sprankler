/*
 * locktests.c
 *
 *  Created on: Jan 7, 2017
 *      Author: erik
 */

#include "locktests.h"

#include <tests/tests.h>

#include <ostests/unittestOsDefaults.h>

#include "thread_impl.h"

static void localSetupLockTests(void)
{
  unittest_installDefaultMemIntf();
  unittest_installDefaultThreadIntf();
}

static void localTeardownLockTest(void)
{
  unittest_uninstallDefaultThreadIntf();
  unittest_uninstallDefaultMemIntf();
}

START_TEST(test_createAtomicLock)
  const IThread_t * const ti = getThreadIntf();
  K_Status_e result;

  OsAtomicLock_t lock = ti->alockCreate();
  ck_assert(lock != NULL);

  ck_assert_int_eq(lock->isBusy, THREAD_LOCK_AVAILABLE);
  result = ti->alockDestroy(lock);
  ck_assert_int_eq(result, K_Status_OK);
END_TEST

START_TEST(test_lockAndUnlockAtomicLock)
  const IThread_t * const ti = getThreadIntf();

  OsAtomicLock_t lock = ti->alockCreate();
  ck_assert(lock != NULL);

  ck_assert_int_eq(lock->isBusy, THREAD_LOCK_AVAILABLE);
  K_Status_e result = ti->alockLock(lock);

  ck_assert_int_eq(result, K_Status_OK);
  ck_assert_int_eq(lock->isBusy, THREAD_LOCK_BUSY);
  result = ti->alockTrylock(lock);
  ck_assert_int_eq(result, K_Status_Locked);
  result = ti->alockUnlock(lock);
  ck_assert_int_eq(result, K_Status_OK);
  ck_assert_int_eq(lock->isBusy, THREAD_LOCK_AVAILABLE);

  result = ti->alockDestroy(lock);
  ck_assert_int_eq(result, K_Status_OK);
END_TEST

START_TEST(test_trylockAtomicLock)
  const IThread_t * const ti = getThreadIntf();

  OsAtomicLock_t lock = ti->alockCreate();
  ck_assert(lock != NULL);

  ck_assert_int_eq(lock->isBusy, THREAD_LOCK_AVAILABLE);
  K_Status_e result = ti->alockTrylock(lock);

  ck_assert_int_eq(result, K_Status_OK);
  ck_assert_int_eq(lock->isBusy, THREAD_LOCK_BUSY);
  result = ti->alockTrylock(lock);
  ck_assert_int_eq(result, K_Status_Locked);
  result = ti->alockUnlock(lock);
  ck_assert_int_eq(result, K_Status_OK);
  ck_assert_int_eq(lock->isBusy, THREAD_LOCK_AVAILABLE);

  result = ti->alockDestroy(lock);
  ck_assert_int_eq(result, K_Status_OK);
END_TEST

START_TEST(test_atomicCallsWithNULL)
  const IThread_t * const ti = getThreadIntf();

  K_Status_e result = ti->alockLock(NULL);
  ck_assert_int_eq(result, K_Status_Invalid_Param);
  result = ti->alockUnlock(NULL);
  ck_assert_int_eq(result, K_Status_Invalid_Param);
  result = ti->alockTrylock(NULL);
  ck_assert_int_eq(result, K_Status_Invalid_Param);
  result = ti->alockDestroy(NULL);
  ck_assert_int_eq(result, K_Status_Invalid_Param);
END_TEST

START_TEST(test_createMutex)
  const IThread_t * const ti = getThreadIntf();
  K_Status_e result;

  OsMutex_t mutex = ti->mutexCreate();
  ck_assert(mutex != NULL);

  struct OsMutex * impl = mutex;
  ck_assert(impl != NULL);

  ck_assert(impl->guard != NULL);

  result = ti->mutexDestroy(mutex);
  ck_assert_int_eq(result, K_Status_OK);
END_TEST

START_TEST(test_mutexLock)
  const IThread_t * const ti = getThreadIntf();
  K_Status_e result;

  OsMutex_t mutex = ti->mutexCreate();
  ck_assert(mutex != NULL);

  struct OsMutex * impl = mutex;
  ck_assert(impl != NULL);

  ck_assert(impl->guard != NULL);
  result = ti->mutexLock(mutex);
  ck_assert_int_eq(result, K_Status_OK);
  result = ti->mutexTrylock(mutex);
  ck_assert_int_eq(result, K_Status_Unexpected_State);
  result = ti->mutexUnlock(mutex);
  ck_assert_int_eq(result, K_Status_OK);
  result = ti->mutexTrylock(mutex);
  ck_assert_int_eq(result, K_Status_OK);
  result = ti->mutexUnlock(mutex);
  ck_assert_int_eq(result, K_Status_OK);

  result = ti->mutexDestroy(mutex);
  ck_assert_int_eq(result, K_Status_OK);
END_TEST

START_TEST(test_destroyMutexWithGuardLocked)
  const IThread_t * const ti = getThreadIntf();
  K_Status_e result;

  OsMutex_t mutex = ti->mutexCreate();
  ck_assert(mutex != NULL);

  struct OsMutex * impl = mutex;
  ck_assert(impl != NULL);

  ck_assert(impl->guard != NULL);
  /* Now lock the guard */
  result = ti->alockLock(impl->guard);
  ck_assert_int_eq(result, K_Status_OK);

  result = ti->mutexDestroy(mutex);
  ck_assert_int_eq(result, K_Status_Locked);

  result = ti->alockUnlock(impl->guard);
  ck_assert_int_eq(result, K_Status_OK);

  result = ti->mutexDestroy(mutex);
  ck_assert_int_eq(result, K_Status_OK);
END_TEST

Suite * osLockSuite(void)
{
  Suite * s = suite_create("Os Lock Suite");
  TCase * tc = frameworkCreateValgrindTestCase("Os Locks testcase");

  tcase_add_checked_fixture(tc, localSetupLockTests, localTeardownLockTest);

  tcase_set_timeout(tc, 3);

  tcase_add_test(tc, test_createAtomicLock);
  tcase_add_test(tc, test_lockAndUnlockAtomicLock);
  tcase_add_test(tc, test_trylockAtomicLock);
  tcase_add_test(tc, test_atomicCallsWithNULL);

  tcase_add_test(tc, test_createMutex);
  tcase_add_test(tc, test_mutexLock);
  tcase_add_test(tc, test_destroyMutexWithGuardLocked);

  suite_add_tcase(s, tc);

  return s;

}
