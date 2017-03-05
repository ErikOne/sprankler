/*
 * loglevels.h
 *
 *  Created on: Sep 18, 2013
 *      Author: erik
 */

#ifndef LOGLEVELS_H_
#define LOGLEVELS_H_

#include <zlog/zlog.h>

typedef enum
{
  LogLevelNoLog = 0,
  LogLevelFatal = 1000,
  LogLevelError = 2000,
  LogLevelWarning = 3000,
  LogLevelInfo = 4000,
  LogLevelTrace = 5000,
  LogLevelVerbose = 6000
} LogLevel_e;

#endif /* LOGLEVELS_H_ */
