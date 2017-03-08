/*
 * sprankler_intf.c
 *
 *  Created on: Mar 8, 2017
 *      Author: erik
 */

#include "sprankler.h"

static ISprankler_t localIntf =
{
#ifndef UNITTESTS
  .getEventbus = sprankler_getEventBus,
#endif
};

ISprankler_t * getSpranklerIntf()
{
  return &localIntf;
}
