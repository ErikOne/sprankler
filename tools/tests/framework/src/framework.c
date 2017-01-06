/*
 * framework.c
 *
 *  Created on: Jan 28, 2013
 *      Author: erik
 */

/* This is the only file that uses malloc and free directly. The reason for this
 * is that I don't want any tests depending on osabstraction. That's what
 * dependency injection is all about.
 */

#include <check/check.h>
#include <stdlib.h>
#include <stdio.h>

#include <common/one_types.h>
#include <testframework/tests.h>
#include <valgrind/memcheck.h>

struct TestSuite
{
  Suite* suite;
  struct TestSuite *next;
};

static struct TestSuite *localSuites;
static uint64_t localLeakedUntilNow;
static void localValgrindSetup(void);
static void localValgrindTeardown(void);
static void localResetMemleaks(void);
static uint64_t localGetMemleaks(void);

static void localFreeSuites(void);

void frameworkInit(void)
{
  localSuites = NULL;
}

OneStatus_t frameworkAddSuite(Suite* const suite)
{
  OneStatus_t rc = One_Failure;
  struct TestSuite *newSuite = (struct TestSuite *) malloc(sizeof(struct TestSuite));
  if (newSuite != NULL)
  {
    memset(newSuite, 0, sizeof(struct TestSuite));
    newSuite->suite = suite;

    if (localSuites == NULL)
    {
      localSuites = newSuite;
    }
    else
    {
      struct TestSuite * tmp = localSuites;
      while (tmp->next != NULL)
      {
        tmp = tmp->next;
      }

      tmp->next = newSuite;
    }

    rc = One_Success;
  }

  return rc;
}

void frameworkRun(const char* const suiteName)
{
  if (localSuites != NULL)
  {
    SRunner *sr = srunner_create(NULL);
    struct TestSuite *tmp = localSuites;
    char_t filename[1024];

    /* Jenkins sets this environment variable when it runs, I deliberatly don't use the osutils interface,
     * to avoid all the tests to become dependent on osutils, which is against unit tests priciples.*/

    const char_t* resultsdir = getenv(ENV_RESULTS_DIR);

    if (resultsdir != NULL)
    {
      snprintf(filename,1024,"%s/%s_testresults.xml",resultsdir,suiteName);
    }
    else
    {
      snprintf(filename,1024,"/tmp/%s_testresults.xml",suiteName);
    }

    srunner_set_fork_status(sr, CK_FORK_GETENV);
    srunner_set_xml(sr,filename);

    while (tmp != NULL)
    {
      srunner_add_suite(sr, tmp->suite);
      tmp = tmp->next;
    }

    srunner_run_all(sr, CK_NORMAL);
    localFreeSuites();
    srunner_free(sr);
  }
}

static void localFreeSuites(void)
{

  while (localSuites != NULL)
  {
    struct TestSuite *tmp = localSuites;
    localSuites = tmp->next;

    /* The actual suites pointed to by tmp->suite is freed by srunner_free, this */

    memset(tmp, 0, sizeof(struct TestSuite));
    free(tmp);
  }
}

TCase *frameworkCreateTestCase(const char *name)
{
  return tcase_create(name);
}

TCase *frameworkCreateValgrindTestCase(const char * const name)
{
    TCase *tc = tcase_create(name);
    tcase_set_timeout(tc, 0);
    tcase_add_checked_fixture(tc, localValgrindSetup, localValgrindTeardown);

    return tc;
}

static void localResetMemleaks(void)
{
    localLeakedUntilNow = localGetMemleaks();
}

static uint64_t localGetMemleaks(void)
{
    uint64_t leaked;
    uint64_t dubious;
    uint64_t reachable;
    uint64_t suppressed;

    VALGRIND_DO_LEAK_CHECK;
    VALGRIND_COUNT_LEAKS(leaked, dubious, reachable, suppressed);

    if (leaked < localLeakedUntilNow)
    {
        printf("VALGRIND STRANGE SITUATION, reset everything\n");
        localLeakedUntilNow = leaked;
    }

    return leaked;
}

static void localValgrindSetup()
{
    if (RUNNING_ON_VALGRIND)
    {
        printf("RUNNING VALGRIND SETUP\n");
        localResetMemleaks();
    }
}

static void localValgrindTeardown()
{
    if (RUNNING_ON_VALGRIND)
    {
        printf("RUNNING VALGRIND TEARDOWN.\n");
        int64_t leaked = localGetMemleaks() - localLeakedUntilNow;
        fail_unless(leaked == 0,"There are %llu bytes leaked in this test\n",leaked);
        printf("(%lu bytes definitely lost)\n", leaked);
    }
}
