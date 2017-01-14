/*
 * mem_defaults.c
 *
 *  Created on: Jan 7, 2017
 *      Author: erik
 */

#include "mem_impl.h"

#include <ostests/unittestOsDefaults.h>

#ifdef UNITTESTS

void unittest_installDefaultMemIntf()
{
  IMem_t * mem = getMemIntf();

  mem->free = mem_free;
  mem->malloc = mem_malloc;
  mem->memset = mem_memset;
}

void unittest_uninstallDefaultMemIntf()
{
  IMem_t * mem = getMemIntf();

  mem->free = NULL;
  mem->malloc = NULL;
  mem->memset = NULL;
}

#endif
