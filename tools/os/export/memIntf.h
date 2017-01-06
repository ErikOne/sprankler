/*
 * memIntf.h
 *
 *  Created on: Jan 4, 2017
 *      Author: erik
 */

#ifndef EXPORT_MEMINTF_H_
#define EXPORT_MEMINTF_H_

#include <common/k_types.h>

typedef struct _mem_interface
{
  /*!
   * \brief Returns heap memory of size size_t.
   *
   * The memory is always set to 0. This implies that if the memory is not available NULL is returned. Therefor the
   * caller of this function MUST always test the return value of this function
   */

  void *(*malloc)(size_t size);

  /*!
   * \brief Returns the memory previously alloced by #malloc to the system
   *
   * If NULL is passed in the cal is silently ignored
   */

  void (*free) (void * heap);

  void* (*memset) (void * dest, uint8_t value, size_t size);

} IMem_t;

IMem_t *getMemIntf(void);


#endif /* EXPORT_MEMINTF_H_ */
