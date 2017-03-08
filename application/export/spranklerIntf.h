/*
 * spranklerIntf.h
 *
 *  Created on: Mar 8, 2017
 *      Author: erik
 */

#ifndef EXPORT_SPRANKLERINTF_H_
#define EXPORT_SPRANKLERINTF_H_

#include <eventbus/eventbusIntf.h>

typedef struct _sprankler_interface
{
  EventBus_t (* getEventbus)(void);

} ISprankler_t;

ISprankler_t * getSpranklerIntf(void);

#endif /* EXPORT_SPRANKLERINTF_H_ */
