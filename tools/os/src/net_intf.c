/*
 * net_intf.c
 *
 *  Created on: Mar 5, 2017
 *      Author: erik
 */

#include "net_impl.h"

static INetwork_t localIntf =
{
#ifndef UNITTESTS
  .createSocket   = osnet_createSocket,
  .bind           = osnet_bind,
  .deleteSocket   = osnet_deleteSocket,
  .setNonBlocking = osnet_setNonBlocking,
  .udpSend        = osnet_udpSend,
  .getFD          = osnet_getFD,
#endif
};

INetwork_t * getNetIntf(void)
{
  return &localIntf;
}
