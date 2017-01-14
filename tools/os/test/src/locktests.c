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
  {
    const IThread_t * const ti = getThreadIntf();
    K_Status_e result;

    OsAtomicLock_t lock = ti->createALock();
    ck_assert(lock != NULL);

    ck_assert_int_eq(lock->isBusy, THREAD_LOCK_AVAILABLE);
    result = ti->destroyALock(lock);
    ck_assert_int_eq(result, K_Status_OK);
  }
END_TEST

START_TEST(test_lockAndUnlockAtomicLock)
  {
    const IThread_t * const ti = getThreadIntf();

    OsAtomicLock_t lock = ti->createALock();
    ck_assert(lock != NULL);

    ck_assert_int_eq(lock->isBusy, THREAD_LOCK_AVAILABLE);
    K_Status_e result = ti->lockAtomic(lock);

    ck_assert_int_eq(result, K_Status_OK);
    ck_assert_int_eq(lock->isBusy, THREAD_LOCK_BUSY);
    result = ti->trylockAtomic(lock);
    ck_assert_int_eq(result, K_Status_Locked);
    result = ti->unlockAtomic(lock);
    ck_assert_int_eq(result, K_Status_OK);
    ck_assert_int_eq(lock->isBusy, THREAD_LOCK_AVAILABLE);

    result = ti->destroyALock(lock);
    ck_assert_int_eq(result, K_Status_OK);
  }
END_TEST

START_TEST(test_atomicCallsWithNULL)
  {
    const IThread_t * const ti = getThreadIntf();

    K_Status_e result = ti->lockAtomic(NULL);
    ck_assert_int_eq(result, K_Status_Invalid_Param);
    result = ti->unlockAtomic(NULL);
    ck_assert_int_eq(result, K_Status_Invalid_Param);
    result = ti->trylockAtomic(NULL);
    ck_assert_int_eq(result, K_Status_Invalid_Param);
    result = ti->destroyALock(NULL);
    ck_assert_int_eq(result, K_Status_Invalid_Param);
  }
END_TEST

Suite * osLockSuite(void)
{
  Suite * s = suite_create("Os Lock Suite");
  TCase * tc = frameworkCreateValgrindTestCase("Os Locks testcase");

  tcase_add_checked_fixture(tc, localSetupLockTests, localTeardownLockTest);

  tcase_set_timeout(tc, 3);

  tcase_add_test(tc, test_createAtomicLock);
  tcase_add_test(tc, test_lockAndUnlockAtomicLock);
  tcase_add_test(tc, test_atomicCallsWithNULL);

  suite_add_tcase(s, tc);

  return s;

}
