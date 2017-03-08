/*
 * mem_intf.c
 *
 *  Created on: Jan 4, 2017
 *      Author: erik
 */

#include "mem_impl.h"

static IMem_t localIntf =
{
#ifndef UNITTESTS
  .malloc = mem_malloc,
  .free   = mem_free,
  .memset = mem_memset,
  .memcpy = mem_memcpy,
#endif
};

IMem_t * getMemIntf(void)
{
  return &localIntf;
}
