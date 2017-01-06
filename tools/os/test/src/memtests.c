/*
 * memtests.c
 *
 *  Created on: Jan 6, 2017
 *      Author: erik
 */

#include "memtests.h"

#include <tests/tests.h>
#include "mem_impl.h"


static int localSetupMemIntf(void **state)
{
  IMem_t *mem = getMemIntf();

  mem->free = mem_free;
  mem->malloc = mem_malloc;
  mem->memset = mem_memset;
  return 0;
}

static int localTeardownMemIntf(void **state)
{
  IMem_t *mem = getMemIntf();

  mem->free = NULL;
  mem->malloc = NULL;
  mem->memset = NULL;

  return 0;
}


static void test_osmalloc(void **state)
{
  const IMem_t *const mem = getMemIntf();
  void *data = NULL;
  size_t size = 1000;
  size_t i;

  data = mem->malloc(size);
  assert_non_null(data);

  for (i=0; i < size; i++)
  {
    assert_int_equal( *(uint8_t*)(data+i), 0);
  }

  mem->free(data);
}

static void test_osmalloc0size(void **state)
{
  const IMem_t *const mem = getMemIntf();
  void *data = NULL;
  size_t size = 0;

  data = mem->malloc(size);
  assert_null(data);
}

static void test_osfreeNULL(void **state)
{
  const IMem_t *const mem = getMemIntf();
  mem->free(NULL);
}

static void test_osmemset(void **state)
{
  const IMem_t *const mem = getMemIntf();

  void *data = NULL;
  size_t size = 1000;
  size_t i;

  data = mem->malloc(size);
  assert_non_null(data);
  mem->memset(data, 'A', size);


  for (i=0; i < size; i++)
  {
    assert_int_equal( *(uint8_t*)(data+i), 'A');
  }

  mem->free(data);
}

int mem_tests(void)
{
  const struct CMUnitTest tests[] = {
      cmocka_unit_test(test_osmalloc),
      cmocka_unit_test(test_osmalloc0size),
      cmocka_unit_test(test_osfreeNULL),
      cmocka_unit_test(test_osmemset),

  };
  return cmocka_run_group_tests(tests, localSetupMemIntf, localTeardownMemIntf);
}
