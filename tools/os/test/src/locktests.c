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

static int localSetupLockTests(void ** state)
{
  unittest_installDefaultMemIntf();
  unittest_installDefaultThreadIntf();
  return 0;
}

static int localTeardownLockTest(void ** state)
{
  unittest_uninstallDefaultThreadIntf();
  unittest_uninstallDefaultMemIntf();
  return 0;
}

static void test_createAtomicLock(void ** state)
{
  const IThread_t * const ti = getThreadIntf();
  K_Status_e result;

  OsAtomicLock_t lock = ti->createALock();
  assert_non_null(lock);

  assert_int_equal(lock->isBusy, THREAD_LOCK_AVAILABLE);
  result = ti->destroyALock(lock);
  assert_int_equal(result, K_Status_OK);
}

static void test_lockAndUnlocktomicLock(void ** state)
{
  const IThread_t * const ti = getThreadIntf();

  OsAtomicLock_t lock = ti->createALock();
  assert_non_null(lock);

  assert_int_equal(lock->isBusy, THREAD_LOCK_AVAILABLE);
  K_Status_e result = ti->lockAtomic(lock);

  assert_int_equal(result, K_Status_OK);
  assert_int_equal(lock->isBusy, THREAD_LOCK_BUSY);
  result = ti->trylockAtomic(lock);
  assert_int_equal(result, K_Status_Locked);
  result = ti->unlockAtomic(lock);
  assert_int_equal(result, K_Status_OK);
  assert_int_equal(lock->isBusy, THREAD_LOCK_AVAILABLE);

  result = ti->destroyALock(lock);
  assert_int_equal(result, K_Status_OK);
}

static void test_atomicCallsWithNULL(void ** state)
{
  const IThread_t * const ti = getThreadIntf();

  K_Status_e result = ti->lockAtomic(NULL);
  assert_int_equal(result, K_Status_Invalid_Param);
  result = ti->unlockAtomic(NULL);
  assert_int_equal(result, K_Status_Invalid_Param);
  result = ti->trylockAtomic(NULL);
  assert_int_equal(result, K_Status_Invalid_Param);
  result = ti->destroyALock(NULL);
  assert_int_equal(result, K_Status_Invalid_Param);
}

int lock_tests(void)
{
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(test_createAtomicLock),
    cmocka_unit_test(test_lockAndUnlocktomicLock),
    cmocka_unit_test(test_atomicCallsWithNULL),
  };

  return cmocka_run_group_tests(tests, localSetupLockTests, localTeardownLockTest);
}
