/*
 * mem.c
 *
 *  Created on: Jan 4, 2017
 *      Author: erik
 */

#include "mem_impl.h"

#include <stdlib.h>
#include <string.h>

void * mem_malloc(size_t size)
{
  void * result = NULL;

  if (size > 0)
  {
    result = malloc(size);
    if (result != NULL)
    {
      memset(result, 0, size);
    }
  }

  return result;
}

void mem_free(void * heap)
{
  if (heap != NULL)
  {
    free(heap);
  }
}

void * mem_memset(void * dest, uint8_t data, size_t size)
{
  void * result = NULL;

  if (dest != NULL)
  {
    result = memset(dest, data, size);
  }

  return result;
}

void * mem_memcpy(void * dst, const void * const src, size_t size)
{
  void * result = NULL;
  if ((dst != NULL) && (src != NULL) && (size > 0))
  {
    result = memcpy(dst, src, size);
  }
  return result;
}
