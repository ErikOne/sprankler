/*
 * utils_defaults.c
 *
 *  Created on: Jan 15, 2017
 *      Author: erik
 */

#include "utils_impl.h"

#include <ostests/unittestOsDefaults.h>

#ifdef UNITTESTS

void unittest_installDefaultUtilsIntf()
{
  IUtils_t * intf = getUtilsIntf();

  intf->sleep = utils_sleep;
  intf->usleep = utils_usleep;
}

void unittest_uninstallDefaultUtilsIntf()
{
  IUtils_t * intf = getUtilsIntf();

  intf->sleep = NULL;
  intf->usleep = NULL;

}

#endif
