/*
 * logging.c
 *
 *  Created on: Sep 10, 2013
 *      Author: erik
 */

#include <stdio.h>
#include "logging.h"

K_Status_e log_init(const char_t * const name)
{
  K_Status_e rc = K_Status_General_Error;

  if (name != NULL)
  {
    if (dzlog_init(NULL, name) == 0)
    {
      rc = K_Status_OK;
      INFO("Successfully intitialized log framework\n");
    }
    else
    {
      fprintf(stderr, "Could not initialize log framework\n");
    }
  }
  else
  {
    fprintf(stderr, "Please specify a category name\n");
  }

  return rc;
}

void log_shutdown(void)
{
  zlog_fini();
}

K_Boolean_e log_isEnabledFor(LogLevel_e level)
{
  K_Boolean_e enabled = K_False;

  if ((system_logLevel >= level) && (system_logLevel != LogLevelNoLog))
  {
    enabled = K_True;
  }

  return enabled;
}

K_Status_e log_setLevel(LogLevel_e level)
{
  K_Status_e rc = K_Status_General_Error;

  switch (level)
  {
    case LogLevelError:
      system_logLevel = LogLevelError;
      rc = K_Status_OK;
      break;
    case LogLevelFatal:
      system_logLevel = LogLevelFatal;
      rc = K_Status_OK;
      break;
    case LogLevelInfo:
      system_logLevel = LogLevelInfo;
      rc = K_Status_OK;
      break;
    case LogLevelNoLog:
      system_logLevel = LogLevelNoLog;
      rc = K_Status_OK;
      break;
    case LogLevelTrace:
      system_logLevel = LogLevelTrace;
      rc = K_Status_OK;
      break;
    case LogLevelVerbose:
      system_logLevel = LogLevelVerbose;
      rc = K_Status_OK;
      break;
    case LogLevelWarning:
      system_logLevel = LogLevelWarning;
      rc = K_Status_OK;
      break;
    default:
      WARNING("Invalid loglevel specified");
      break;
  }

  return rc;
}
