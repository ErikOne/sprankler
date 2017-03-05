/*
 * logging.h
 *
 *  Created on: Jan 29, 2013
 *      Author: erik
 */

#ifndef LOGGING_H_
#define LOGGING_H_

#include <stdio.h>
#include <inttypes.h>
#include <common/k_types.h>
#include <logging/loglevels.h>

typedef struct _logIntf
{
  K_Status_e (* init)(const char_t * const name);
  void (* shutdown)(void);

  K_Status_e (* setLevel)(LogLevel_e level);
  K_Boolean_e (* isEnabledFor)(LogLevel_e level);
} LoggerIntf_t;

LoggerIntf_t * getLoggerIntf(void);

extern uint32_t system_logLevel;

#ifndef UNITTESTS
#include <zlog/zlog.h>

#define FATAL(args ...)    { if (system_logLevel >= LogLevelFatal) dzlog_fatal(args); }
#define ERROR(args ...)    { if (system_logLevel >= LogLevelError) dzlog_error(args); }
#define WARNING(args ...)  { if (system_logLevel >= LogLevelWarning) dzlog_warn(args); }
#define INFO(args ...)     { if (system_logLevel >= LogLevelInfo) dzlog_notice(args); }
#define TRACE(args ...)    { if (system_logLevel >= LogLevelTrace) dzlog_info(args); }
#define VERBOSE(args ...)  { if (system_logLevel >= LogLevelVerbose) dzlog_debug(args); }
#else
#include <stdio.h>

#define INFO(args ...)       { printf("INFO: %s:%d: ", __FILE__, __LINE__); printf(args); }
#define FATAL(args ...)      { printf("FATAL: %s:%d: ", __FILE__, __LINE__); printf(args); }
#define ERROR(args ...)      { printf("ERROR: %s:%d: ", __FILE__, __LINE__); printf(args); }
#define WARNING(args ...)    { printf("WARNING: %s:%d: ", __FILE__, __LINE__); printf(args); }
#define TRACE(args ...)      { printf("TRACE: %s:%d: ", __FILE__, __LINE__); printf(args); }
#define VERBOSE(args ...)    { printf("VERBOSE: %s:%d: ", __FILE__, __LINE__); printf(args); }

#endif

#endif /* LOGGING_H_ */
