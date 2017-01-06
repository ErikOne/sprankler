/*
 * mem_impl.h
 *
 *  Created on: Jan 4, 2017
 *      Author: erik
 */

#ifndef OS_HEADER_MEM_IMPL_H_
#define OS_HEADER_MEM_IMPL_H_

#include <os/memIntf.h>

void *mem_malloc(size_t size);
void mem_free(void *heap);
void *mem_memset(void *dest, uint8_t value, size_t size);


#endif /* OS_HEADER_MEM_IMPL_H_ */
