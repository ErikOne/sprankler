/*
 * testhelper.c
 *
 *  Created on: Jul 15, 2014
 *      Author: erik
 */

#include <check/check.h>
#include <testframework/tests.h>

#include <ostests/defaultintf.h>
#include <logging/logging.h>

#include "logging.h"
#include "testhelper.h"

void setupLoggerTests(void)
{
  setDefaultMemIntfForTests();
  setDefaultOsUtilsIntfForTests();

  LoggerIntf_t * intf = getLoggerIntf();
  intf->setLevel = log_setLevel;
  intf->isEnabledFor = log_isEnabledFor;
}

void teardownLoggerTests(void)
{
  LoggerIntf_t * intf = getLoggerIntf();
  intf->setLevel = NULL;
  intf->isEnabledFor = NULL;

  resetOsUtilsIntfForTests();
  resetMemIntfForTests();
}

