/*
 * logging_intf.c
 *
 *  Created on: Sep 10, 2013
 *      Author: erik
 */

#include "logging.h"

uint32_t system_logLevel = LogLevelInfo;

static LoggerIntf_t intf =
{
#ifndef UNITTESTS
  &log_init,
  &log_shutdown,
  &log_setLevel,
  &log_isEnabledFor,
#endif
};

LoggerIntf_t * getLoggerIntf(void)
{
  return &intf;
}
