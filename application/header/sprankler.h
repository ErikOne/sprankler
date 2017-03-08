/*
 * spranker.h
 *
 *  Created on: Mar 8, 2017
 *      Author: erik
 */

#ifndef EXPORT_SPRANKLER_H_
#define EXPORT_SPRANKLER_H_

#include <sprankler/spranklerIntf.h>

K_Status_e sprankler_initEventBus(void);
EventBus_t sprankler_getEventBus(void);

#endif /* EXPORT_SPRANKLER_H_ */
