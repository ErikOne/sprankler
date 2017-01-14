/*
 * memtests.c
 *
 *  Created on: Jan 6, 2017
 *      Author: erik
 */

#include "memtests.h"

#include <tests/tests.h>
#include <ostests/unittestOsDefaults.h>

#include <os/memIntf.h>

START_TEST(test_osmalloc)
  {
    const IMem_t * const mem = getMemIntf();
    void * data = NULL;
    size_t size = 1000;
    size_t i;

    data = mem->malloc(size);
    ck_assert(data != NULL);

    for (i = 0; i < size; i++)
    {
      ck_assert( *(uint8_t *)(data + i) == 0);
    }

    mem->free(data);
  }
END_TEST

START_TEST(test_osmemset)
  {
    const IMem_t * const mem = getMemIntf();
    void * data = NULL;
    size_t size = 1000;
    size_t i;

    data = mem->malloc(size);
    ck_assert(data != NULL);

    ck_assert (mem->memset(data, 'e', size) == data);

    for (i = 0; i < size; i++)
    {
      ck_assert( *(uint8_t *)(data + i) == 'e');
    }

    mem->free(data);
  }
END_TEST

Suite * osMemSuite(void)
{
  Suite * s = suite_create("Os Memory tests");
  TCase * tc = frameworkCreateValgrindTestCase("Os Memory testcase");

  tcase_add_checked_fixture(tc, unittest_installDefaultMemIntf, unittest_uninstallDefaultMemIntf);

  tcase_set_timeout(tc, 3);

  tcase_add_test(tc, test_osmalloc);
  tcase_add_test(tc, test_osmemset);

  suite_add_tcase(s, tc);

  return s;
}
