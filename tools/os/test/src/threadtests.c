/*
 * threadtests.c
 *
 *  Created on: Jan 15, 2017
 *      Author: erik
 */

#include "threadtests.h"

#include <tests/tests.h>

#include <ostests/unittestOsDefaults.h>
#include <os/memIntf.h>

#include "thread_impl.h"

static void localSetupThreadTests(void)
{
  unittest_installDefaultMemIntf();
  unittest_installDefaultUtilsIntf();
  unittest_installDefaultThreadIntf();
}

static void localTeardownThreadTest(void)
{
  unittest_uninstallDefaultThreadIntf();
  unittest_uninstallDefaultUtilsIntf();
  unittest_uninstallDefaultMemIntf();
}

static void * localDoNothing(void * ignore)
{
  return NULL;
}

START_TEST(test_createAndDestroyThread)
  const IThread_t * const ti = getThreadIntf();
  OsThread_t thread = ti->createThread(localDoNothing, NULL);

  K_Status_e rc = ti->joinThread(thread);
  ck_assert_int_eq(rc, K_Status_OK);

  rc = ti->destroyThread(thread);
  ck_assert_int_eq(rc, K_Status_OK);

END_TEST

struct _testData1
{
  OsAtomicLock_t alock;
  volatile uint64_t value;
  uint32_t loop;

};

static void * localIncrementWithAtomic(struct _testData1 * data)
{
  uint32_t i;
  const IThread_t * const ti = getThreadIntf();

  for (i = 0; i < data->loop; i++)
  {
    while (ti->lockAtomic(data->alock) != K_Status_OK)
    {
      ti->yieldThread();
    }

    data->value++;
    ti->unlockAtomic(data->alock);
  }

  return NULL;
}

START_TEST(test_atomiclocksInMutiThread)
  {
    const IThread_t * const ti = getThreadIntf();
    const IMem_t * const mem = getMemIntf();

    struct _testData1 * data = mem->malloc(sizeof(struct _testData1));

    data->alock = ti->createALock();
    data->value = 0;
    data->loop = 10000;

    ck_assert(data->alock != NULL);
    ck_assert(ti->lockAtomic(data->alock) == K_Status_OK);

    size_t nbrOfThreads = 5;
    size_t i;

    OsThread_t threads[nbrOfThreads];

    for (i = 0; i < nbrOfThreads; i++)
    {
      threads[i] = ti->createThread((THREAD_FUNCTION)localIncrementWithAtomic, data);
      ck_assert(threads[i] != NULL);
    }

    ck_assert(ti->unlockAtomic(data->alock) == K_Status_OK);

    for (i = 0; i < nbrOfThreads; i++)
    {
      ck_assert(ti->destroyThread(threads[i]) == K_Status_OK);
    }

    ck_assert(data->value == nbrOfThreads * data->loop);
    ck_assert(ti->destroyALock(data->alock) == K_Status_OK);
    mem->free(data);

  }
END_TEST

Suite * osThreadSuite(void)
{
  Suite * s = suite_create("Os Thread Suite");
  TCase * tc = frameworkCreateValgrindTestCase("Os Thread testcase");

  tcase_add_checked_fixture(tc, localSetupThreadTests, localTeardownThreadTest);

  tcase_set_timeout(tc, 3);

  tcase_add_test(tc, test_createAndDestroyThread);
  tcase_add_test(tc, test_atomiclocksInMutiThread);

  suite_add_tcase(s, tc);

  return s;

}
